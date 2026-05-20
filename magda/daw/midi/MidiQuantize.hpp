#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include <vector>

#include "GrooveTemplate.hpp"

namespace magda {

/**
 * @brief Grid size for quantization.
 */
enum class QuantizeGrid {
    Quarter,     ///< 1/4 note
    Eighth,      ///< 1/8 note
    Sixteenth,   ///< 1/16 note
    ThirtySecond ///< 1/32 note
};

/**
 * @brief Settings for MIDI quantization.
 */
struct QuantizeSettings {
    QuantizeGrid gridSize = QuantizeGrid::Sixteenth;
    float strength = 100.0f;    ///< 0-100%, how much to pull toward grid
    float swing = 0.0f;         ///< 0-100%, swing amount on off-beats
    float humanize = 0.0f;      ///< 0-100%, random timing variation

    /** Get grid size in beats (quarter notes). */
    double getGridInBeats() const {
        switch (gridSize) {
            case QuantizeGrid::Quarter:
                return 1.0;
            case QuantizeGrid::Eighth:
                return 0.5;
            case QuantizeGrid::Sixteenth:
                return 0.25;
            case QuantizeGrid::ThirtySecond:
                return 0.125;
        }
        return 0.25;
    }
};

/**
 * @brief A MIDI note event with timing and velocity for quantization.
 */
struct QuantizableNote {
    double startBeat = 0.0;   ///< Start position in beats
    double lengthBeats = 0.0; ///< Duration in beats
    int noteNumber = 60;      ///< MIDI note number (0-127)
    float velocity = 0.8f;    ///< Velocity (0.0-1.0)
};

/**
 * @brief Advanced MIDI quantization engine.
 *
 * Supports standard grid quantization with adjustable strength,
 * swing, humanize, and groove template application.
 */
class MidiQuantize {
  public:
    MidiQuantize() = default;

    /**
     * @brief Quantize a set of notes to a grid.
     * @param notes     The notes to quantize (modified in place).
     * @param settings  Quantization parameters.
     */
    void quantize(std::vector<QuantizableNote>& notes,
                  const QuantizeSettings& settings) const;

    /**
     * @brief Apply a groove template to notes.
     * @param notes     The notes to groove (modified in place).
     * @param groove    The groove template to apply.
     * @param strength  How strongly to apply the groove (0.0-1.0).
     */
    void applyGroove(std::vector<QuantizableNote>& notes,
                     const GrooveTemplate& groove, float strength) const;

    /**
     * @brief Extract a groove template from a MIDI clip.
     *
     * Analyzes the timing deviations of notes from a strict grid
     * and captures them as a reusable groove template.
     *
     * @param notes    The notes to analyze.
     * @param grid     The reference grid size.
     * @return A groove template capturing the timing feel.
     */
    GrooveTemplate extractGroove(const std::vector<QuantizableNote>& notes,
                                 QuantizeGrid grid) const;

  private:
    double quantizePosition(double position, double gridSize, float strength,
                            float swing, int gridIndex) const;
    double addHumanize(double position, float humanize, double gridSize) const;
};

}  // namespace magda
