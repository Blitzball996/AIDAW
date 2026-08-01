namespace llm {

juce::String OpenAIChatClient::buildRequestBody(const Request& request) const {
    auto messagesArray = juce::Array<juce::var>();

    auto* sysMsg = new juce::DynamicObject();
    sysMsg->setProperty("role", "system");
    sysMsg->setProperty("content", request.systemPrompt);
    messagesArray.add(juce::var(sysMsg));

    auto* userMsg = new juce::DynamicObject();
    userMsg->setProperty("role", "user");
    userMsg->setProperty("content", request.userMessage);
    messagesArray.add(juce::var(userMsg));

    auto* payload = new juce::DynamicObject();
    payload->setProperty("model", config_.model);
    payload->setProperty("messages", messagesArray);
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

            // A relay that injects an agent system prompt (e.g. a Claude Code
            // Max channel with system_prompt_override enabled) can push the
            // model into tool-calling mode. It then answers with tool_calls and
            // an empty content string, which is a well-formed response — so the
            // generic "failed to parse" message below is actively misleading.
            // Name the tool and the likely cause instead.
            if (!response.success) {
                if (auto* toolCalls = choice["message"]["tool_calls"].getArray()) {
                    if (toolCalls->size() > 0) {
                        auto toolName = (*toolCalls)[0]["function"]["name"].toString();
                        response.error =
                            "The model replied with a tool call"
                            + (toolName.isNotEmpty() ? " (" + toolName + ")" : juce::String())
                            + " instead of text, so there is nothing to use. This usually means "
                              "the endpoint is prepending an agent/coding system prompt to the "
                              "request. Disable that channel's system prompt override, or pick a "
                              "model that is not routed through it.";
                        return response;
                    }
                }
            }
        }
    }

    if (!response.success)
        response.error = "Failed to parse response: " + jsonString.substring(0, 200);

    return response;
}

}  // namespace llm
