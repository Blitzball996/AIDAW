#pragma once

#include <juce_core/juce_core.h>

#include <atomic>
#include <functional>
#include <memory>
#include <string>

namespace aidaw {

/**
 * @brief Abstract base for any per-device AI agent driven by the AI side panel.
 *
 * Two flavours sit underneath this:
 *  - SoundDesignAgent — picks parameter values for an existing device
 *  - CoderAgent — generates code for devices that host code (e.g. Faust)
 *
 * Both share the same call-site signature so the UI panel can hold a single
 * pointer regardless of flavour.
 */
class DeviceAIAgent {
public:
    virtual ~DeviceAIAgent() = default;

    /// Per-token streaming callback. Return false to cancel.
    using TokenCallback = std::function<bool(const juce::String& token)>;

    /// Run the agent on `prompt` and apply its output to the device.
    /// Returns a one-line status string.
    virtual juce::String generateAndApply(const juce::String& prompt,
                                          TokenCallback onToken = {}) = 0;

    /// Best-effort cancel; safe to call from any thread.
    virtual void requestCancel() { shouldStop_ = true; }

protected:
    std::atomic<bool> shouldStop_{false};
};

/**
 * @brief Per-device "design me a preset" agent interface.
 *
 * The chat console (/design) and the per-device AI side panel both go
 * through this. A device type (e.g. synth) ships an implementation that
 * knows how to talk to its LLM, parse the result into the device's
 * preset shape, and write it. New devices add support by writing their
 * own subclass.
 */
class SoundDesignAgent : public DeviceAIAgent {
public:
    /**
     * Optional category override the agent should bias toward
     * (e.g. "Bass", "Lead", "Pad"). Empty = let the model pick.
     */
    virtual void setCategoryOverride(const juce::String& /*category*/) {}
};

/**
 * Returns the SoundDesignAgent implementation for `pluginId`, or
 * nullptr if no specialised agent exists. The returned agent is owned
 * by the caller; create one per design session so cancel state doesn't leak.
 */
std::unique_ptr<SoundDesignAgent> createSoundDesignAgentFor(const juce::String& pluginId);

/**
 * Quick check — returns true iff createSoundDesignAgentFor(pluginId)
 * would return a non-null agent.
 */
bool isSoundDesignSupported(const juce::String& pluginId);

}  // namespace aidaw
