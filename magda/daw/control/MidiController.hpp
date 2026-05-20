#pragma once

#include "ControlSurface.hpp"

#include <juce_audio_devices/juce_audio_devices.h>

#include <map>
#include <memory>
#include <mutex>
#include <optional>

namespace magda {

/**
 * @brief A single MIDI CC mapping entry.
 */
struct MidiCCMapping {
    int ccNumber = -1;       ///< MIDI CC number (0-127)
    int midiChannel = 0;     ///< MIDI channel (0-15, 0 = omni)
    int targetChannel = 0;   ///< DAW channel/parameter to control
    float minValue = 0.0f;   ///< Minimum output value
    float maxValue = 1.0f;   ///< Maximum output value
    bool isRelative = false; ///< true = relative encoder, false = absolute
};

/**
 * @brief Preset mapping configurations for common MIDI controllers.
 */
struct MidiControllerPreset {
    juce::String name;
    juce::String manufacturer;
    std::vector<MidiCCMapping> mappings;
};

/**
 * @brief Generic MIDI CC control surface implementation.
 *
 * Maps MIDI CC messages to DAW parameters with support for:
 * - Absolute and relative (encoder) modes
 * - MIDI learn mode for easy assignment
 * - Preset mappings for common controllers
 * - Note-on for button presses
 */
class MidiController : public ControlSurface,
                       private juce::MidiInputCallback {
  public:
    MidiController();
    ~MidiController() override;

    // --- ControlSurface interface ---

    juce::String getName() const override { return name_; }
    juce::String getProtocol() const override { return "MIDI"; }
    bool isConnected() const override { return connected_; }

    bool open() override;
    void close() override;

    void onFaderMove(int channel, float value) override;
    void onButtonPress(int id) override;
    void onEncoderTurn(int id, int delta) override;

    void sendFeedback(int channel, float value) override;
    void sendLED(int id, bool state) override;

    // --- MIDI device selection ---

    /** Set the MIDI input device to use. */
    void setInputDevice(const juce::String& deviceIdentifier);

    /** Set the MIDI output device for feedback. */
    void setOutputDevice(const juce::String& deviceIdentifier);

    /** Get available MIDI input devices. */
    static juce::StringArray getAvailableInputDevices();

    /** Get available MIDI output devices. */
    static juce::StringArray getAvailableOutputDevices();

    // --- Mapping ---

    /** Add a CC mapping. */
    void addMapping(const MidiCCMapping& mapping);

    /** Remove a mapping by CC number and channel. */
    void removeMapping(int ccNumber, int midiChannel = 0);

    /** Clear all mappings. */
    void clearMappings();

    /** Get all current mappings. */
    std::vector<MidiCCMapping> getMappings() const;

    // --- Learn mode ---

    /**
     * @brief Enter learn mode: the next CC received will be mapped.
     * @param targetChannel The DAW channel to assign the learned CC to.
     * @param callback Called when a CC is learned (cc, channel).
     */
    void listenForNextCC(int targetChannel,
                         std::function<void(int cc, int midiChannel)> callback = nullptr);

    /** Cancel learn mode. */
    void cancelLearn();

    /** Check if currently in learn mode. */
    bool isLearning() const { return learning_; }

    // --- Presets ---

    /** Load a preset mapping configuration. */
    void loadPreset(const MidiControllerPreset& preset);

    /** Get built-in presets for common controllers. */
    static std::vector<MidiControllerPreset> getBuiltInPresets();

    /** Set the controller display name. */
    void setName(const juce::String& name) { name_ = name; }

  private:
    // juce::MidiInputCallback
    void handleIncomingMidiMessage(juce::MidiInput* source,
                                   const juce::MidiMessage& message) override;

    void handleCC(int cc, int value, int midiChannel);
    void handleNoteOn(int note, int velocity, int midiChannel);

    float ccToFloat(int ccValue, const MidiCCMapping& mapping) const;

    juce::String name_ = "MIDI Controller";
    bool connected_ = false;

    juce::String inputDeviceId_;
    juce::String outputDeviceId_;
    std::unique_ptr<juce::MidiInput> midiInput_;
    std::unique_ptr<juce::MidiOutput> midiOutput_;

    // Mappings: key = (ccNumber << 4) | midiChannel
    std::map<int, MidiCCMapping> mappings_;
    mutable std::mutex mutex_;

    // Learn mode
    bool learning_ = false;
    int learnTargetChannel_ = 0;
    std::function<void(int, int)> learnCallback_;
};

}  // namespace magda
