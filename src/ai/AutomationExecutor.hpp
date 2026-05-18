#pragma once

#include <juce_core/juce_core.h>

#include <vector>

#include "AutomationParser.hpp"

namespace aidaw {

/**
 * @brief Executes AutomationAgent IR against the DAW automation surface.
 *
 * Resolves the "selected" target via the host's selection state, generates
 * shape points in beats, and writes them through the automation API.
 *
 * NOTE: In the current AIDAW port, execution is stubbed — the actual DAW
 * automation API integration will be wired when the automation subsystem
 * is available. The shape-point generation math is fully ported.
 */
class AutomationExecutor {
public:
    AutomationExecutor() = default;

    /** Run all instructions. Returns true on success. */
    bool execute(const std::vector<AutoInstruction>& instructions);

    juce::String getError() const { return error_; }
    juce::String getResults() const { return results_; }

private:
    juce::String error_;
    juce::String results_;
};

}  // namespace aidaw
