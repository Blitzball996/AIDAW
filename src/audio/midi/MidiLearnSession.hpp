#pragma once

#include <juce_core/juce_core.h>

namespace aidaw {

using ControllerId = int;

/**
 * @brief Binding message type for MIDI learn
 */
enum class BindingMsgType {
    NoteOn,
    NoteOff,
    CC,
    PitchBend,
    ProgramChange
};

/**
 * @brief Snapshot of the MIDI event that triggered a learn session capture.
 *
 * Produced on the MIDI thread, delivered to the caller on the message thread.
 */
struct LearnCapture {
    juce::String portId;
    juce::String portName;
    ControllerId controllerId = 0;
    BindingMsgType msgType = BindingMsgType::CC;
    int channel = 0;   // 1..16
    int number = 0;    // CC number, note number, or 0 for pitch-bend
    int rawValue = 0;
};

/**
 * @brief Configuration for a MIDI learn capture session.
 */
struct LearnSessionConfig {
    /** Minimum gap in ms between Note-on and Note-off needed to suppress the
     *  Note-off from triggering a second capture. */
    int captureDebounceMs = 50;

    /** When true, the captured channel is used verbatim in the binding source.
     *  When false, channel is stored as 0 (any channel). */
    bool lockChannel = false;
};

}  // namespace aidaw
