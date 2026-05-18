#include "RelayLlmClient.hpp"
#include <juce_core/juce_core.h>

namespace aidaw {

RelayLlmClient::RelayLlmClient(const std::string& url)
    : relayUrl(url) {}

bool RelayLlmClient::isAvailable() const {
    return !relayUrl.empty();
}

LlmResponse RelayLlmClient::send(const LlmRequest& request) {
    LlmResponse response;

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

    auto postData = juce::JSON::toString(juce::var(root.release()));
    auto url = juce::URL(juce::String(relayUrl)).withPOSTData(postData);

    juce::StringPairArray headers;
    headers.set("Content-Type", "application/json");

    auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
        .withExtraHeaders(headers.joinIntoString("\r\n"));

    if (auto stream = url.createInputStream(options)) {
        auto body = stream->readEntireStreamAsString();
        auto parsed = juce::JSON::parse(body);
        if (auto* obj = parsed.getDynamicObject()) {
            if (obj->hasProperty("content")) {
                response.content = obj->getProperty("content").toString().toStdString();
                response.success = true;
            } else if (obj->hasProperty("error")) {
                response.error = obj->getProperty("error").toString().toStdString();
            }
        }
    } else {
        response.error = "Failed to connect to relay server";
    }

    return response;
}

LlmResponse RelayLlmClient::sendStreaming(const LlmRequest& request, TokenCallback onToken) {
    auto response = send(request);
    if (response.success && onToken) {
        onToken(response.content);
    }
    return response;
}

}  // namespace aidaw
