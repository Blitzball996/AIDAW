#pragma once

#include <atomic>
#include <string>
#include <vector>

#include "AutomationExecutor.hpp"
#include "AutomationParser.hpp"
#include "llm/LlmClient.hpp"

namespace aidaw {

/**
 * @brief Automation agent — emits automation curves on a selected lane.
 *
 * Generates AUTO instructions (shape-based: sin, tri, saw, square, exp, log,
 * line, plus freeform). Time domain is beats. Values are normalized [0, 1].
 *
 * processMessage() flow:
 *   1. Call LLM with system prompt describing AUTO grammar + current
 *      selection context
 *   2. Parse response into AutoInstruction IR
 *   3. Execute against the automation surface
 */
class AutomationAgent {
public:
    explicit AutomationAgent(LlmClient* client);

    struct GenerateResult {
        std::string rawOutput;
        std::vector<AutoInstruction> instructions;
        std::string error;
        bool hasError = false;
    };

    /** Call LLM to produce AUTO instructions (background-thread safe). */
    GenerateResult generate(const std::string& message);

    /** Streaming variant — token callback fires for each streamed chunk. */
    GenerateResult generateStreaming(const std::string& message, TokenCallback onToken);

    /** Execute previously-parsed IR. */
    std::string execute(const GenerateResult& result);

    void requestCancel() { shouldStop_ = true; }
    void resetCancel() { shouldStop_ = false; }

    static const char* getSystemPrompt();

private:
    LlmClient* llmClient_;
    AutomationParser parser_;
    AutomationExecutor executor_;
    std::atomic<bool> shouldStop_{false};

    static std::string cleanOutput(const std::string& raw);
};

}  // namespace aidaw
