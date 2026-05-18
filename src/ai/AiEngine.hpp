#pragma once

#include "MusicAgent.hpp"
#include "RouterAgent.hpp"
#include "VoiceInput.hpp"
#include "PromptParser.hpp"
#include "InstructionExecutor.hpp"
#include "llm/LlmClient.hpp"
#include "llm/LlmClientFactory.hpp"
#include <memory>
#include <string>

namespace aidaw {

class AiEngine {
public:
    AiEngine();

    void initialize(const std::string& modelPath,
                    const std::string& cloudUrl,
                    const std::string& cloudKey,
                    const std::string& relayUrl);

    void setBackend(LlmBackend backend);
    LlmBackend getCurrentBackend() const { return currentBackend; }

    MusicAgent::GenerateResult processText(const std::string& text);
    void startVoiceInput();
    void stopVoiceInput();

    MusicAgent* getMusicAgent() { return musicAgent.get(); }
    RouterAgent* getRouterAgent() { return routerAgent.get(); }
    InstructionExecutor* getExecutor() { return &executor; }

private:
    LlmBackend currentBackend = LlmBackend::Cloud;
    std::unique_ptr<LlmClient> localClient;
    std::unique_ptr<LlmClient> cloudClient;
    std::unique_ptr<LlmClient> relayClient;
    std::unique_ptr<MusicAgent> musicAgent;
    std::unique_ptr<RouterAgent> routerAgent;
    VoiceInput voiceInput;
    PromptParser promptParser;
    InstructionExecutor executor;

    LlmClient* getActiveClient();
};

}  // namespace aidaw
