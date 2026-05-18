#include "RouterAgent.hpp"
#include <chrono>
#include <algorithm>
#include <cctype>

namespace aidaw {

RouterAgent::RouterAgent(LlmClient* client)
    : llmClient(client) {}

const char* RouterAgent::getSystemPrompt() {
    return R"(You are a message classifier. Classify the user's message into exactly one category.

Respond with a single word only:
- COMMAND — if the message is a DAW command (play, stop, undo, save, set volume, mute track, etc.)
- MUSIC — if the message asks to compose, generate, or describe music
- BOTH — if the message contains both a command and a music request

Respond with only one word: COMMAND, MUSIC, or BOTH.)";
}

RouterAgent::ClassifyResult RouterAgent::classify(const std::string& message) {
    ClassifyResult result;

    auto startTime = std::chrono::steady_clock::now();

    if (!llmClient || !llmClient->isAvailable()) {
        result.hasError = true;
        result.error = "LLM client not available";
        return result;
    }

    LlmRequest request;
    request.systemPrompt = getSystemPrompt();
    request.messages.push_back({"user", message});
    request.temperature = 0.0f;
    request.maxTokens = 16;

    auto response = llmClient->send(request);

    auto endTime = std::chrono::steady_clock::now();
    result.wallSeconds = std::chrono::duration<double>(endTime - startTime).count();

    if (!response.success) {
        result.hasError = true;
        result.error = response.error;
        return result;
    }

    result.intent = parseIntent(response.content);
    switch (result.intent) {
        case Intent::Command: result.intentString = "COMMAND"; break;
        case Intent::Music:   result.intentString = "MUSIC";   break;
        case Intent::Both:    result.intentString = "BOTH";    break;
    }

    return result;
}

RouterAgent::Intent RouterAgent::parseIntent(const std::string& response) {
    // Normalize: uppercase and trim
    std::string normalized;
    for (char c : response) {
        if (std::isalpha(static_cast<unsigned char>(c)))
            normalized += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }

    if (normalized.find("BOTH") != std::string::npos)
        return Intent::Both;
    if (normalized.find("COMMAND") != std::string::npos)
        return Intent::Command;

    // Default to Music if unclear
    return Intent::Music;
}

}  // namespace aidaw
