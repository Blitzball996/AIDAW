#pragma once

#include <juce_core/juce_core.h>

#include <atomic>
#include <string>
#include <vector>

#include "compact_parser.hpp"

namespace magda {

/**
 * @brief Music agent — generates pure musical content.
 *
 * Generates: CHORD, NOTE, ARP.
 * Uses compact format for local models, DSL format for frontier models.
 *
 * The agent holds no DAW reference of its own. Callers that want it to revise
 * an existing arrangement rather than write from scratch pass a project
 * snapshot (MusicMemory::buildDAWSnapshot) into generate(); without one the
 * agent behaves as before and composes from nothing.
 */
class MusicAgent {
  public:
    struct GenerateResult {
        std::string rawOutput;
        std::string description;  // populated by DSL format (frontier models)
        std::vector<Instruction> instructions;
        std::string error;
        bool hasError = false;
        bool truncated = false;  // provider hit its output cap mid-answer
    };

    /** Generate music instructions from user message (background thread safe).
        @param projectContext  optional snapshot of the current arrangement. */
    GenerateResult generate(const std::string& message,
                            const std::string& projectContext = {});

    /** Streaming variant — calls onToken for each received token. */
    GenerateResult generateStreaming(const std::string& message, TokenCallback onToken,
                                     const std::string& projectContext = {});

    void requestCancel() {
        shouldStop_ = true;
    }
    void resetCancel() {
        shouldStop_ = false;
    }

  private:
    static const char* getCompactSystemPrompt();
    static const char* getDSLSystemPrompt();

    /** Parse DSL note operations into IR instructions. */
    std::vector<Instruction> parseDSL(const juce::String& text, std::string& outDescription);

    CompactParser parser_;
    std::atomic<bool> shouldStop_{false};
};

}  // namespace magda
