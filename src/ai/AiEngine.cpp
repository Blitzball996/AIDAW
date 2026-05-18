#include "AiEngine.hpp"

namespace aidaw {

AiEngine::AiEngine() = default;

void AiEngine::initialize(const std::string& modelPath,
                           const std::string& cloudUrl,
                           const std::string& cloudKey,
                           const std::string& relayUrl) {
    if (!modelPath.empty()) {
        localClient = LlmClientFactory::create(LlmBackend::Local, modelPath);
    }
    if (!cloudUrl.empty()) {
        cloudClient = LlmClientFactory::create(LlmBackend::Cloud, cloudUrl, cloudKey);
    }
    if (!relayUrl.empty()) {
        relayClient = LlmClientFactory::create(LlmBackend::Relay, relayUrl);
    }

    musicAgent = std::make_unique<MusicAgent>(getActiveClient());
    routerAgent = std::make_unique<RouterAgent>(getActiveClient());
}

void AiEngine::setBackend(LlmBackend backend) {
    currentBackend = backend;
    musicAgent = std::make_unique<MusicAgent>(getActiveClient());
    routerAgent = std::make_unique<RouterAgent>(getActiveClient());
}

MusicAgent::GenerateResult AiEngine::processText(const std::string& text) {
    // First try direct command parsing (no LLM needed)
    auto directCommands = promptParser.parseDirectCommands(text);
    if (!directCommands.empty()) {
        executor.execute(directCommands);
        return {text, directCommands, "", false};
    }

    // Classify intent via router
    auto classification = routerAgent->classify(text);

    if (!classification.hasError) {
        switch (classification.intent) {
            case RouterAgent::Intent::Command: {
                // For commands that weren't caught by direct parsing,
                // return a result indicating it's a command
                MusicAgent::GenerateResult result;
                result.rawOutput = "Command recognized: " + text;
                return result;
            }
            case RouterAgent::Intent::Music: {
                auto result = musicAgent->generate(text);
                if (!result.hasError) {
                    executor.execute(result.instructions);
                }
                return result;
            }
            case RouterAgent::Intent::Both: {
                // Handle as music generation (commands embedded in music requests)
                auto result = musicAgent->generate(text);
                if (!result.hasError) {
                    executor.execute(result.instructions);
                }
                return result;
            }
        }
    }

    // Fallback: treat as music if router fails
    auto result = musicAgent->generate(text);
    if (!result.hasError) {
        executor.execute(result.instructions);
    }
    return result;
}

void AiEngine::startVoiceInput() {
    voiceInput.startListening([this](const std::string& text) {
        processText(text);
    });
}

void AiEngine::stopVoiceInput() {
    voiceInput.stopListening();
}

LlmClient* AiEngine::getActiveClient() {
    switch (currentBackend) {
        case LlmBackend::Local:  return localClient.get();
        case LlmBackend::Cloud:  return cloudClient.get();
        case LlmBackend::Relay:  return relayClient.get();
    }
    return cloudClient.get();
}

}  // namespace aidaw
