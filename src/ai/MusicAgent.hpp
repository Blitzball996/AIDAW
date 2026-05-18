#pragma once

#include "MusicInstruction.hpp"
#include "llm/LlmClient.hpp"
#include <string>
#include <vector>
#include <atomic>

namespace aidaw {

class MusicAgent {
public:
    struct GenerateResult {
        std::string rawOutput;
        std::vector<MusicInstruction> instructions;
        std::string error;
        bool hasError = false;
    };

    explicit MusicAgent(LlmClient* client);

    GenerateResult generate(const std::string& userMessage);
    GenerateResult generateStreaming(const std::string& userMessage, TokenCallback onToken);

    void requestCancel() { shouldStop = true; }
    void resetCancel() { shouldStop = false; }

private:
    LlmClient* llmClient;
    std::atomic<bool> shouldStop{false};

    static const char* getSystemPrompt();
    std::vector<MusicInstruction> parseOutput(const std::string& output);
};

}  // namespace aidaw
