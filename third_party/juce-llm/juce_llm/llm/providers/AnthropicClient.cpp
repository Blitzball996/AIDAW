namespace llm {

juce::String AnthropicClient::buildRequestBody(const Request& request) const {
    auto messagesArray = juce::Array<juce::var>();

    if (request.messages.empty()) {
        auto* userMsg = new juce::DynamicObject();
        userMsg->setProperty("role", "user");
        userMsg->setProperty("content", request.userMessage);
        messagesArray.add(juce::var(userMsg));
    } else {
        // Anthropic has no "tool" role: a tool result is a user turn whose
        // content is one or more tool_result blocks. Consecutive results are
        // merged into a single user turn, which the API requires.
        juce::Array<juce::var> pendingToolResults;

        auto flushToolResults = [&]() {
            if (pendingToolResults.isEmpty())
                return;
            auto* msg = new juce::DynamicObject();
            msg->setProperty("role", "user");
            msg->setProperty("content", pendingToolResults);
            messagesArray.add(juce::var(msg));
            pendingToolResults.clear();
        };

        for (const auto& m : request.messages) {
            if (m.role == Message::Role::Tool) {
                auto* block = new juce::DynamicObject();
                block->setProperty("type", "tool_result");
                block->setProperty("tool_use_id", m.toolCallId);
                block->setProperty("content", m.content);
                pendingToolResults.add(juce::var(block));
                continue;
            }

            flushToolResults();

            auto* msg = new juce::DynamicObject();
            if (m.role == Message::Role::User) {
                msg->setProperty("role", "user");
                msg->setProperty("content", m.content);
            } else {
                msg->setProperty("role", "assistant");
                if (m.toolCalls.empty()) {
                    msg->setProperty("content", m.content);
                } else {
                    juce::Array<juce::var> blocks;
                    if (m.content.isNotEmpty()) {
                        auto* textBlock = new juce::DynamicObject();
                        textBlock->setProperty("type", "text");
                        textBlock->setProperty("text", m.content);
                        blocks.add(juce::var(textBlock));
                    }
                    for (const auto& tc : m.toolCalls) {
                        auto* useBlock = new juce::DynamicObject();
                        useBlock->setProperty("type", "tool_use");
                        useBlock->setProperty("id", tc.id);
                        useBlock->setProperty("name", tc.name);
                        // input must be an object, not the raw JSON string.
                        auto parsed = juce::JSON::parse(tc.arguments);
                        useBlock->setProperty(
                            "input", parsed.isObject() ? parsed
                                                       : juce::var(new juce::DynamicObject()));
                        blocks.add(juce::var(useBlock));
                    }
                    msg->setProperty("content", blocks);
                }
            }
            messagesArray.add(juce::var(msg));
        }

        flushToolResults();
    }

    auto* payload = new juce::DynamicObject();
    payload->setProperty("model", config_.model);
    // 4096 was too tight for long structured output: a full song arrangement
    // runs past it and comes back truncated mid-line, which the DSL parser then
    // silently drops. Every model this client targets supports at least 8192.
    int maxTok = request.maxTokens > 0 ? request.maxTokens
                                       : (config_.maxTokens > 0 ? config_.maxTokens : 8192);
    payload->setProperty("max_tokens", maxTok);
    payload->setProperty("temperature", (double)request.temperature);
    payload->setProperty("messages", messagesArray);

    if (!request.tools.empty()) {
        juce::Array<juce::var> toolsArray;
        for (const auto& t : request.tools) {
            auto* tool = new juce::DynamicObject();
            tool->setProperty("name", t.name);
            tool->setProperty("description", t.description);
            tool->setProperty("input_schema", t.parameters);
            toolsArray.add(juce::var(tool));
        }
        payload->setProperty("tools", toolsArray);
    }

    // System prompt with prompt caching — cache the system prompt block
    // since it's identical across calls for each agent
    if (request.systemPrompt.isNotEmpty()) {
        auto* sysBlock = new juce::DynamicObject();
        sysBlock->setProperty("type", "text");
        sysBlock->setProperty("text", request.systemPrompt);

        auto* cacheControl = new juce::DynamicObject();
        cacheControl->setProperty("type", "ephemeral");
        sysBlock->setProperty("cache_control", juce::var(cacheControl));

        juce::Array<juce::var> systemArray;
        systemArray.add(juce::var(sysBlock));
        payload->setProperty("system", systemArray);
    }

    // NOTE: Anthropic's Messages API does not accept an `effort` / `output_config`
    // field — that is an OpenAI-ism. `reasoningEffort` is deliberately ignored here.
    // (Extended thinking uses a separate `thinking` block on models that support it.)

    // App identification for abuse tracking
    if (config_.userAgent.isNotEmpty()) {
        auto* metadata = new juce::DynamicObject();
        metadata->setProperty("user_id", config_.userAgent);
        payload->setProperty("metadata", juce::var(metadata));
    }

    return juce::JSON::toString(juce::var(payload), true);
}

juce::String AnthropicClient::getEndpointUrl() const {
    return config_.baseUrl + "/messages";
}

juce::StringPairArray AnthropicClient::getHeaders() const {
    juce::StringPairArray headers;
    headers.set("x-api-key", config_.apiKey);
    headers.set("anthropic-version", "2023-06-01");
    headers.set("Content-Type", "application/json");
    return headers;
}

Response AnthropicClient::parseResponseBody(const juce::String& jsonString) const {
    Response response;
    auto json = juce::JSON::parse(jsonString);

    if (auto* content = json["content"].getArray()) {
        // Concatenate every text block rather than reading only the first one:
        // a response may lead with a thinking or tool_use block and carry the
        // actual answer in a later block.
        juce::String text;
        for (auto& block : *content) {
            auto type = block["type"].toString();
            if (type == "tool_use") {
                ToolCall call;
                call.id = block["id"].toString();
                call.name = block["name"].toString();
                // Anthropic gives the arguments as an object; keep them as a
                // JSON string so ToolCall looks the same on every provider.
                call.arguments = juce::JSON::toString(block["input"], true);
                if (call.name.isNotEmpty())
                    response.toolCalls.push_back(std::move(call));
                continue;
            }
            auto blockText = block["text"].toString();
            if (blockText.isNotEmpty())
                text += blockText;
        }
        response.text = text.trim();
        // A tool call with no text is a well-formed reply. Whether it is
        // usable depends on whether the caller offered tools, which this
        // function cannot see — LLMClient::sendRequest makes that call.
        response.success = response.text.isNotEmpty() || !response.toolCalls.empty();
    }

    response.truncated = json["stop_reason"].toString() == "max_tokens";

    if (!response.success)
        response.error = "Failed to parse response: " + jsonString.substring(0, 200);

    return response;
}

// Anthropic SSE format:
//   data: {"type":"content_block_delta","index":0,"delta":{"type":"text_delta","text":"token"}}
juce::String AnthropicClient::parseStreamChunk(const juce::String& dataLine) const {
    auto json = juce::JSON::parse(dataLine);
    auto type = json["type"].toString();
    if (type == "content_block_delta")
        return json["delta"]["text"].toString();
    return {};
}

}  // namespace llm
