namespace llm {

namespace {

// Fallback HTTP POST via curl (works with TUN/VPN that JUCE WinINet misses)
Response curlFallback(const juce::String& endpoint, const juce::String& body,
                      const juce::StringPairArray& headers, double startTime) {
    Response response;

    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
    auto tempBody = tempDir.getChildFile("magda_llm_req.json");
    auto tempOut = tempDir.getChildFile("magda_llm_resp.txt");
    tempBody.replaceWithText(body);
    tempOut.deleteFile();

    // Build curl command with full path
#if JUCE_WINDOWS
    juce::String curlExe = "C:\\Windows\\System32\\curl.exe";
#else
    juce::String curlExe = "/usr/bin/curl";
#endif

    juce::StringArray args;
    args.add(curlExe);
    args.add("-s");
    args.add("-X");
    args.add("POST");
    args.add(endpoint);
    args.add("--connect-timeout");
    args.add("30");
    args.add("--max-time");
    args.add("300");
    args.add("-d");
    args.add("@" + tempBody.getFullPathName());

    for (auto& key : headers.getAllKeys()) {
        args.add("-H");
        args.add(key + ": " + headers[key]);
    }

    // Write response body to file, status code to a separate file
    auto tempStatus = tempDir.getChildFile("magda_llm_status.txt");
    args.add("-o");
    args.add(tempOut.getFullPathName());
    args.add("-w");
    args.add("%{http_code}");
    args.add("--output-dir");  // not needed, -o is absolute

    // Remove --output-dir, just use -o and write status to file
    // Actually simpler: write everything to one file with status appended
    args.clear();
    args.add(curlExe);
    args.add("-s");
    args.add("-X");
    args.add("POST");
    args.add(endpoint);
    args.add("--connect-timeout");
    args.add("30");
    args.add("--max-time");
    args.add("300");
    args.add("-d");
    args.add("@" + tempBody.getFullPathName());

    for (auto& key : headers.getAllKeys()) {
        args.add("-H");
        args.add(key + ": " + headers[key]);
    }

    // Output body to file, write http_code to status file
    args.add("-o");
    args.add(tempOut.getFullPathName());
    args.add("-w");
    args.add("%{http_code}");

    juce::ChildProcess proc;
    if (!proc.start(args)) {
        response.error = "Failed to launch curl at " + curlExe;
        response.wallSeconds = (juce::Time::getMillisecondCounterHiRes() - startTime) / 1000.0;
        tempBody.deleteFile();
        return response;
    }

    // Read stdout (contains http_code from -w) BEFORE waitForProcessToFinish
    juce::String stdoutText;
    while (proc.isRunning()) {
        stdoutText += proc.readAllProcessOutput();
        juce::Thread::sleep(50);
    }
    stdoutText += proc.readAllProcessOutput();
    stdoutText = stdoutText.trim();

    tempBody.deleteFile();

    auto responseBody = tempOut.loadFileAsString();
    tempOut.deleteFile();

    int statusCode = stdoutText.getIntValue();
    response.wallSeconds = (juce::Time::getMillisecondCounterHiRes() - startTime) / 1000.0;

    if (statusCode == 0) {
        response.error = "curl failed (no response) for " + endpoint;
        return response;
    }

    if (statusCode < 200 || statusCode >= 300) {
        response.error = "HTTP " + juce::String(statusCode) + ": " + responseBody.substring(0, 500);
        return response;
    }

    response.text = responseBody;
    response.success = true;
    return response;
}

}  // namespace

/** A tool call the caller never asked for is not a usable answer.

    When Request::tools is empty and the model still returns tool_calls with no
    text, something between us and the model added a tool harness of its own —
    in practice a relay channel that prepends a coding-agent system prompt. The
    text is empty, so without this the caller sees a bare "failed to parse" and
    has no way to tell that from a malformed response. */
void LLMClient::applyUnsolicitedToolCallDiagnostic(const Request& request, Response& response) {
    if (response.toolCalls.empty() || !request.tools.empty())
        return;
    if (response.text.isNotEmpty())
        return;  // there is still usable text; leave it alone

    response.success = false;
    response.error =
        "The model replied with a tool call (" + response.toolCalls.front().name
        + ") instead of text, so there is nothing to use. This usually means the endpoint is "
          "prepending an agent/coding system prompt to the request. Disable that channel's "
          "system prompt override, or pick a model that is not routed through it.";
}

Response LLMClient::sendRequest(const Request& request) const {
    Response response;
    auto startTime = juce::Time::getMillisecondCounterHiRes();

    auto body = buildRequestBody(request);
    auto url = juce::URL(getEndpointUrl()).withPOSTData(body);
    auto headers = getHeaders();

    // Inject User-Agent if configured
    if (config_.userAgent.isNotEmpty() && !headers.containsKey("User-Agent"))
        headers.set("User-Agent", config_.userAgent);

    // Build header string for JUCE URL API
    juce::String headerString;
    for (auto& key : headers.getAllKeys())
        headerString += key + ": " + headers[key] + "\r\n";

    int statusCode = 0;
    auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                       .withExtraHeaders(headerString)
                       .withStatusCode(&statusCode)
                       .withConnectionTimeoutMs(
                           config_.connectionTimeoutMs > 0 ? config_.connectionTimeoutMs : 30000)
                       .withNumRedirectsToFollow(5);

    auto stream = url.createInputStream(options);

    response.wallSeconds = (juce::Time::getMillisecondCounterHiRes() - startTime) / 1000.0;

    if (stream == nullptr) {
        // JUCE WinINet failed — fallback to curl (handles TUN/VPN proxies)
        auto fallback = curlFallback(getEndpointUrl(), body, headers, startTime);
        if (fallback.success) {
            auto parsed = parseResponseBody(fallback.text);
            parsed.wallSeconds = fallback.wallSeconds;
            LLMClient::applyUnsolicitedToolCallDiagnostic(request, parsed);
            return parsed;
        }
        if (!fallback.error.isEmpty()) {
            response.error = fallback.error;
            response.wallSeconds = fallback.wallSeconds;
            return response;
        }
        response.error = "Failed to connect to " + getEndpointUrl()
                         + " (status=" + juce::String(statusCode) + ")";
        return response;
    }

    auto responseText = stream->readEntireStreamAsString();

    if (statusCode < 200 || statusCode >= 300) {
        response.error = "HTTP " + juce::String(statusCode) + ": " + responseText.substring(0, 500);
        return response;
    }

    response = parseResponseBody(responseText);
    response.wallSeconds = (juce::Time::getMillisecondCounterHiRes() - startTime) / 1000.0;
    LLMClient::applyUnsolicitedToolCallDiagnostic(request, response);
    return response;
}

// Default streaming endpoint — same as non-streaming
juce::String LLMClient::getStreamingEndpointUrl() const {
    return getEndpointUrl();
}

// Default streaming body: inject "stream":true into the normal request body JSON
juce::String LLMClient::buildStreamingRequestBody(const Request& request) const {
    auto body = buildRequestBody(request);
    // Insert "stream":true before the closing brace
    auto lastBrace = body.lastIndexOfChar('}');
    if (lastBrace >= 0)
        return body.substring(0, lastBrace) + ",\n\"stream\": true\n}";
    return body;
}

// Default SSE chunk parser for OpenAI-compatible format:
//   data: {"choices":[{"delta":{"content":"token"}}]}
juce::String LLMClient::parseStreamChunk(const juce::String& dataLine) const {
    auto json = juce::JSON::parse(dataLine);
    if (auto* choices = json["choices"].getArray()) {
        if (choices->size() > 0)
            return (*choices)[0]["delta"]["content"].toString();
    }
    return {};
}

Response LLMClient::sendStreamingRequest(const Request& request, StreamCallback onToken) const {
    Response response;
    auto startTime = juce::Time::getMillisecondCounterHiRes();

    auto body = buildStreamingRequestBody(request);
    auto url = juce::URL(getStreamingEndpointUrl()).withPOSTData(body);
    auto headers = getHeaders();

    // Inject User-Agent if configured
    if (config_.userAgent.isNotEmpty() && !headers.containsKey("User-Agent"))
        headers.set("User-Agent", config_.userAgent);

    juce::String headerString;
    for (auto& key : headers.getAllKeys())
        headerString += key + ": " + headers[key] + "\r\n";

    int statusCode = 0;
    auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                       .withExtraHeaders(headerString)
                       .withStatusCode(&statusCode)
                       .withConnectionTimeoutMs(
                           config_.connectionTimeoutMs > 0 ? config_.connectionTimeoutMs : 0);

    auto stream = url.createInputStream(options);

    if (stream == nullptr) {
        response.error = "Failed to connect to " + getEndpointUrl();
        response.wallSeconds = (juce::Time::getMillisecondCounterHiRes() - startTime) / 1000.0;
        return response;
    }

    if (statusCode < 200 || statusCode >= 300) {
        auto errText = stream->readEntireStreamAsString();
        response.error = "HTTP " + juce::String(statusCode) + ": " + errText.substring(0, 500);
        response.wallSeconds = (juce::Time::getMillisecondCounterHiRes() - startTime) / 1000.0;
        return response;
    }

    // Read SSE stream line by line
    juce::String accumulated;
    bool cancelled = false;

    // Read into a byte buffer, convert complete lines as UTF-8
    std::vector<char> rawLineBuffer;

    while (!stream->isExhausted() && !cancelled) {
        char c;
        if (stream->read(&c, 1) != 1)
            break;

        if (c == '\n') {
            auto line =
                juce::String::fromUTF8(rawLineBuffer.data(), static_cast<int>(rawLineBuffer.size()))
                    .trim();
            rawLineBuffer.clear();

            if (line.startsWith("data: ")) {
                auto data = line.substring(6).trim();
                if (data == "[DONE]")
                    break;

                auto token = parseStreamChunk(data);
                if (token.isNotEmpty()) {
                    accumulated += token;
                    if (onToken && !onToken(token))
                        cancelled = true;
                }
            }
        } else {
            rawLineBuffer.push_back(c);
        }
    }

    response.text = accumulated.trim();
    response.success = response.text.isNotEmpty();
    response.wallSeconds = (juce::Time::getMillisecondCounterHiRes() - startTime) / 1000.0;

    if (cancelled) {
        response.error = "Cancelled";
        response.success = false;
    }

    return response;
}

}  // namespace llm
