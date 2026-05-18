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
}

void AiEngine::setBackend(LlmBackend backend) {
    currentBackend = backend;
    musicAgent = std::make_unique<MusicAgent>(getActiveClient());
}

MusicAgent::GenerateResult AiEngine::processText(const std::string& text) {
    auto directCommands = promptParser.parseDirectCommands(text);
    if (!directCommands.empty()) {
        executor.execute(directCommands);
        return {text, directCommands, "", false};
    }

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
