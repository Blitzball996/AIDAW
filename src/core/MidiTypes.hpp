#pragma once

#include <juce_core/juce_core.h>

namespace aidaw {

/**
 * @brief Real-time MIDI note event (note on/off)
 */
struct MidiNoteEvent {
    int noteNumber = 0;
    int velocity = 0;
    bool isNoteOn = false;
    double timestamp = 0.0;

    MidiNoteEvent() = default;
    MidiNoteEvent(int note, int vel, bool on, double time = 0.0)
        : noteNumber(note), velocity(vel), isNoteOn(on), timestamp(time) {}
};

/**
 * @brief MIDI Control Change (CC) event
 */
struct MidiCCEvent {
    int controller = 0;
    int value = 0;
    double timestamp = 0.0;

    MidiCCEvent() = default;
    MidiCCEvent(int cc, int val, double time = 0.0)
        : controller(cc), value(val), timestamp(time) {}
};

/**
 * @brief MIDI device information
 */
struct MidiDeviceInfo {
    juce::String id;
    juce::String name;
    bool isEnabled = false;
    bool isAvailable = true;

    MidiDeviceInfo() = default;
    MidiDeviceInfo(const juce::String& deviceId, const juce::String& deviceName,
                   bool enabled = false, bool available = true)
        : id(deviceId), name(deviceName), isEnabled(enabled), isAvailable(available) {}
};

}  // namespace aidaw
