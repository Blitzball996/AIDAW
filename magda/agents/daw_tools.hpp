#pragma once

#include <juce_core/juce_core.h>

#include <functional>
#include <string>
#include <vector>

#include <juce_llm/juce_llm.h>

namespace magda {

class MagdaApi;

/**
 * @brief Read-only tools the DAW agent may call while working out what to do.
 *
 * The agent's other path — the compact instruction language — is how it
 * *changes* the project, and that path runs through the executor and the
 * harmony approval gate. These tools deliberately only read. Letting a model
 * mutate the arrangement mid-loop would bypass that gate and make a failed
 * turn leave half-applied edits behind, so every write stays in the
 * instruction pipeline where the user can see and approve it.
 *
 * Why tools at all, rather than pushing the whole project into the prompt:
 * a snapshot of every note in a large arrangement is expensive on every turn
 * and still cannot answer "what parameters does this plugin have". Tools let
 * the model fetch the specific thing it needs.
 */
class DawToolRegistry {
  public:
    struct Tool {
        juce::String name;
        juce::String description;
        juce::var parameters;  // JSON Schema for the arguments
        // Returns a JSON string handed straight back to the model.
        std::function<juce::String(MagdaApi&, const juce::var& args)> run;
    };

    /** The tool set, built once. */
    static const std::vector<Tool>& tools();

    /** Tool definitions in the shape llm::Request expects. */
    static std::vector<llm::ToolDef> toolDefs();

    /**
     * @brief Run one call by name.
     *
     * Always returns a JSON string, including for unknown names and bad
     * arguments — a tool result must be something the model can read and
     * recover from, not an exception that kills the turn.
     */
    static juce::String dispatch(MagdaApi& api, const juce::String& name,
                                 const juce::String& argumentsJson);
};

}  // namespace magda
