#include "CloudLlmClient.hpp"
#include <juce_core/juce_core.h>

namespace aidaw {

CloudLlmClient::CloudLlmClient(const std::string& url, const std::string& key)
    : apiUrl(url), apiKey(key) {}

bool CloudLlmClient::isAvailable() const {
    return !apiUrl.empty() && !apiKey.empty();
}

LlmResponse CloudLlmClient::send(const LlmRequest& request) {
    LlmResponse response;

    auto url = juce::URL(juce::String(apiUrl))
        .withPOSTData(juce::String(buildRequestJson(request)));

    juce::String extraHeaders = "Content-Type: application/json\r\nAuthorization: Bearer " + juce::String(apiKey);

    auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
        .withExtraHeaders(extraHeaders);

    if (auto stream = url.createInputStream(options)) {
        auto body = stream->readEntireStreamAsString().toStdString();
        response = parseResponseJson(body);
    } else {
        response.error = "Failed to connect to API";
    }

    return response;
}

LlmResponse CloudLlmClient::sendStreaming(const LlmRequest& request, TokenCallback onToken) {
    // TODO: SSE streaming implementation
    auto response = send(request);
    if (response.success && onToken) {
        onToken(response.content);
    }
    return response;
}

std::string CloudLlmClient::buildRequestJson(const LlmRequest& request) const {
    auto messagesArray = juce::Array<juce::var>();
    for (const auto& msg : request.messages) {
        auto msgObj = std::make_unique<juce::DynamicObject>();
        msgObj->setProperty("role", juce::String(msg.role));
        msgObj->setProperty("content", juce::String(msg.content));
        messagesArray.add(juce::var(msgObj.release()));
    }

    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("messages", messagesArray);
    root->setProperty("max_tokens", request.maxTokens);
    root->setProperty("temperature", request.temperature);
    if (!request.systemPrompt.empty()) {
        root->setProperty("system", juce::String(request.systemPrompt));
    }

    return juce::JSON::toString(juce::var(root.release())).toStdString();
}

LlmResponse CloudLlmClient::parseResponseJson(const std::string& json) const {
    LlmResponse response;
    auto parsed = juce::JSON::parse(juce::String(json));

    if (auto* obj = parsed.getDynamicObject()) {
        if (obj->hasProperty("error")) {
            response.error = obj->getProperty("error").toString().toStdString();
            return response;
        }
        // OpenAI-compatible format
        if (auto* choices = obj->getProperty("choices").getArray()) {
            if (choices->size() > 0) {
                if (auto* msg = (*choices)[0].getDynamicObject()) {
                    if (auto* message = msg->getProperty("message").getDynamicObject()) {
                        response.content = message->getProperty("content").toString().toStdString();
                        response.success = true;
                    }
                }
            }
        }
        // Anthropic format
        if (auto* content = obj->getProperty("content").getArray()) {
            if (content->size() > 0) {
                if (auto* block = (*content)[0].getDynamicObject()) {
                    response.content = block->getProperty("text").toString().toStdString();
                    response.success = true;
                }
            }
        }
    }

    return response;
}

}  // namespace aidaw
