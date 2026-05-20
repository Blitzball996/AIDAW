#pragma once

#include <juce_core/juce_core.h>

#include <string>
#include <vector>

namespace magda {

/**
 * @brief A groove template storing timing and velocity offsets.
 *
 * Groove templates capture the rhythmic feel of a performance and can be
 * applied to quantized MIDI to add human-like timing variations.
 */
struct GrooveTemplate {
    juce::String name;
    std::vector<double> timingOffsets;    ///< Per-step timing offset in ticks
    std::vector<double> velocityOffsets;  ///< Per-step velocity offset (-1.0 to 1.0)

    /** Number of steps in this groove pattern. */
    int getStepCount() const {
        return static_cast<int>(timingOffsets.size());
    }

    /** Get timing offset for a given step (wraps around). */
    double getTimingOffset(int step) const {
        if (timingOffsets.empty()) return 0.0;
        return timingOffsets[static_cast<size_t>(step % getStepCount())];
    }

    /** Get velocity offset for a given step (wraps around). */
    double getVelocityOffset(int step) const {
        if (velocityOffsets.empty()) return 0.0;
        return velocityOffsets[static_cast<size_t>(step % getStepCount())];
    }

    bool isValid() const { return !timingOffsets.empty(); }
};

/**
 * @brief Factory for built-in groove templates.
 */
class GrooveTemplateFactory {
  public:
    /** Get all available built-in groove templates. */
    static std::vector<GrooveTemplate> getBuiltInGrooves();

    /** Get a specific built-in groove by name. */
    static GrooveTemplate getGrooveByName(const juce::String& name);

    /** Save a groove template to a JSON file. */
    static bool saveToFile(const GrooveTemplate& groove, const juce::File& file);

    /** Load a groove template from a JSON file. */
    static GrooveTemplate loadFromFile(const juce::File& file);
};

}  // namespace magda
