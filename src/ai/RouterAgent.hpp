#pragma once

#include "llm/LlmClient.hpp"
#include <string>
#include <atomic>

namespace aidaw {

/**
 * Lightweight router agent that classifies user intent.
 * Returns one of: COMMAND, MUSIC, or BOTH.
 */
class RouterAgent {
public:
    enum class Intent {
        Command,
        Music,
        Both
    };

    struct ClassifyResult {
        Intent intent = Intent::Music;
        std::string intentString;
        double wallSeconds = 0.0;
        std::string error;
        bool hasError = false;
    };

    explicit RouterAgent(LlmClient* client);

    ClassifyResult classify(const std::string& message);

    void requestCancel() { shouldStop = true; }
    void resetCancel() { shouldStop = false; }

private:
    LlmClient* llmClient;
    std::atomic<bool> shouldStop{false};

    static const char* getSystemPrompt();
    static Intent parseIntent(const std::string& response);
};

}  // namespace aidaw
