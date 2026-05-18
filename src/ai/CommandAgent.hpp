#pragma once

#include <atomic>
#include <string>

#include "llm/LlmClient.hpp"

namespace aidaw {

/**
 * @brief Command agent — handles DAW operations via DSL generation.
 *
 * Generates DSL code (track/clip/fx/notes operations).
 * Uses a cheap/fast model for quick turnaround.
 * Receives DAW state snapshot for context.
 */
class CommandAgent {
public:
    explicit CommandAgent(LlmClient* client);

    struct GenerateResult {
        std::string dslOutput;  // raw DSL text from the LLM
        std::string error;
        bool hasError = false;
    };

    /** Generate DSL from user message (background thread safe). */
    GenerateResult generate(const std::string& message);

    /** Streaming variant — calls onToken for each received token. */
    GenerateResult generateStreaming(const std::string& message, TokenCallback onToken);

    /** Set DAW state context for the next generation call. */
    void setStateContext(const std::string& stateJson);

    void requestCancel() { shouldStop_ = true; }
    void resetCancel() { shouldStop_ = false; }

    static const char* getSystemPrompt();

private:
    LlmClient* llmClient_;
    std::atomic<bool> shouldStop_{false};
    std::string stateContext_;

    static std::string extractDSL(const std::string& raw);
};

}  // namespace aidaw
