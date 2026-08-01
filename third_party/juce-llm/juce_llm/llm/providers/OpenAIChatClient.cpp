namespace llm {

juce::String OpenAIChatClient::buildRequestBody(const Request& request) const {
    auto messagesArray = juce::Array<juce::var>();

    auto* sysMsg = new juce::DynamicObject();
    sysMsg->setProperty("role", "system");
    sysMsg->setProperty("content", request.systemPrompt);
    messagesArray.add(juce::var(sysMsg));

    if (request.messages.empty()) {
        auto* userMsg = new juce::DynamicObject();
        userMsg->setProperty("role", "user");
        userMsg->setProperty("content", request.userMessage);
        messagesArray.add(juce::var(userMsg));
    } else {
        for (const auto& m : request.messages) {
            auto* msg = new juce::DynamicObject();
            switch (m.role) {
                case Message::Role::User:
                    msg->setProperty("role", "user");
                    msg->setProperty("content", m.content);
                    break;

                case Message::Role::Assistant: {
                    msg->setProperty("role", "assistant");
                    // An assistant turn that called tools may legitimately have
                    // no text; the API still requires the key to be present.
                    msg->setProperty("content", m.content);
                    if (!m.toolCalls.empty()) {
                        auto calls = juce::Array<juce::var>();
                        for (const auto& tc : m.toolCalls) {
                            auto* fn = new juce::DynamicObject();
                            fn->setProperty("name", tc.name);
                            fn->setProperty("arguments", tc.arguments);

                            auto* call = new juce::DynamicObject();
                            call->setProperty("id", tc.id);
                            call->setProperty("type", "function");
                            call->setProperty("function", juce::var(fn));
                            calls.add(juce::var(call));
                        }
                        msg->setProperty("tool_calls", calls);
                    }
                    break;
                }

                case Message::Role::Tool:
                    msg->setProperty("role", "tool");
                    msg->setProperty("tool_call_id", m.toolCallId);
                    msg->setProperty("content", m.content);
                    break;
            }
            messagesArray.add(juce::var(msg));
        }
    }

    auto* payload = new juce::DynamicObject();
    payload->setProperty("model", config_.model);
    payload->setProperty("messages", messagesArray);

    if (!request.tools.empty()) {
        auto toolsArray = juce::Array<juce::var>();
        for (const auto& t : request.tools) {
            auto* fn = new juce::DynamicObject();
            fn->setProperty("name", t.name);
            fn->setProperty("description", t.description);
            fn->setProperty("parameters", t.parameters);

            auto* tool = new juce::DynamicObject();
            tool->setProperty("type", "function");
            tool->setProperty("function", juce::var(fn));
            toolsArray.add(juce::var(tool));
        }
        payload->setProperty("tools", toolsArray);
    }

    if (!config_.noTemperature)
        payload->setProperty("temperature", (double)request.temperature);

    // Reasoning effort for GPT-5 / o-series (top-level field in Chat Completions)
    if (config_.reasoningEffort.isNotEmpty())
        payload->setProperty("reasoning_effort", config_.reasoningEffort);

    // Prompt caching — bucket by app+agent, retain for 24h
    if (config_.userAgent.isNotEmpty()) {
        payload->setProperty("prompt_cache_key", config_.userAgent);
        payload->setProperty("prompt_cache_retention", "24h");
    }

    // GBNF grammar for llama-server (per-config or per-request)
    auto grammar = config_.grammar.isNotEmpty() ? config_.grammar : request.grammar;
    if (grammar.isNotEmpty())
        payload->setProperty("grammar", grammar);

    // Max output tokens — per-request override or provider config
    // Use max_completion_tokens (required by newer OpenAI models like GPT-4o, o-series)
    int maxTok = request.maxTokens > 0 ? request.maxTokens : config_.maxTokens;
    if (maxTok > 0)
        payload->setProperty("max_completion_tokens", maxTok);

    // Structured output via JSON schema
    if (!request.schema.isVoid()) {
        auto* schemaWrapper = new juce::DynamicObject();
        schemaWrapper->setProperty("name", "response");
        schemaWrapper->setProperty("strict", true);
        schemaWrapper->setProperty("schema", request.schema);

        auto* responseFormat = new juce::DynamicObject();
        responseFormat->setProperty("type", "json_schema");
        responseFormat->setProperty("json_schema", juce::var(schemaWrapper));

        payload->setProperty("response_format", juce::var(responseFormat));
    }

    return juce::JSON::toString(juce::var(payload), true);
}

juce::String OpenAIChatClient::getEndpointUrl() const {
    return config_.baseUrl + "/chat/completions";
}

juce::StringPairArray OpenAIChatClient::getHeaders() const {
    juce::StringPairArray headers;
    headers.set("Authorization", "Bearer " + config_.apiKey);
    headers.set("Content-Type", "application/json");

    // OpenRouter-specific headers for app identification
    if (config_.baseUrl.contains("openrouter.ai")) {
        if (config_.userAgent.isNotEmpty())
            headers.set("X-Title", config_.userAgent);
        if (config_.appUrl.isNotEmpty())
            headers.set("HTTP-Referer", config_.appUrl);
    }

    return headers;
}

Response OpenAIChatClient::parseResponseBody(const juce::String& jsonString) const {
    Response response;
    auto json = juce::JSON::parse(jsonString);

    if (auto* choices = json["choices"].getArray()) {
        if (choices->size() > 0) {
            const auto& choice = (*choices)[0];
            response.text = choice["message"]["content"].toString().trim();
            response.success = response.text.isNotEmpty();
            response.truncated = choice["finish_reason"].toString() == "length";

            if (auto* toolCalls = choice["message"]["tool_calls"].getArray()) {
                for (const auto& tc : *toolCalls) {
                    ToolCall call;
                    call.id = tc["id"].toString();
                    call.name = tc["function"]["name"].toString();
                    call.arguments = tc["function"]["arguments"].toString();
                    if (call.name.isNotEmpty())
                        response.toolCalls.push_back(std::move(call));
                }
            }

            // A tool call with no text is a well-formed reply, not a parse
            // failure. Whether it is *usable* depends on if the caller offered
            // tools, which this function cannot see — LLMClient::sendRequest
            // makes that call.
            if (!response.toolCalls.empty())
                response.success = true;
        }
    }

    if (!response.success)
        response.error = "Failed to parse response: " + jsonString.substring(0, 200);

    return response;
}

}  // namespace llm
