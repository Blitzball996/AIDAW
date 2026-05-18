#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>

#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef AIDAW_HAS_TRACKTION
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion;
#endif

namespace aidaw {

// Forward declarations
class AudioBridge;
using TrackId = int;

/**
 * @brief MIDI device information
 */
struct MidiDeviceInfo {
    juce::String id;
    juce::String name;
    bool isEnabled = false;
    bool isAvailable = true;
};

/**
 * @brief MIDI note event data
 */
struct MidiNoteEvent {
    int noteNumber = 0;
    int velocity = 0;
    bool isNoteOn = false;
    int channel = 1;
};

/**
 * @brief MIDI CC event data
 */
struct MidiCCEvent {
    int ccNumber = 0;
    int value = 0;
    int channel = 1;
};

/**
 * @brief Receives raw MIDI messages from every open input device.
 *
 * Called from the MIDI callback thread -- implementations must be lock-free.
 */
struct RawMidiListener {
    virtual ~RawMidiListener() = default;
    virtual void onRawMidi(const juce::String& deviceId, const juce::String& deviceName,
                           const juce::MidiMessage& msg) = 0;
};

/**
 * @brief Bridges MIDI model to the audio engine's MIDI system
 *
 * Responsibilities:
 * - Enumerate and manage MIDI input devices
 * - Route MIDI inputs to tracks
 * - Monitor MIDI activity for visualization
 * - Thread-safe communication between UI and audio threads
 */
class MidiBridge : public juce::MidiInputCallback {
  public:
#ifdef AIDAW_HAS_TRACKTION
    explicit MidiBridge(te::Engine& engine);
#else
    MidiBridge();
#endif
    ~MidiBridge() override;

    MidiBridge(MidiBridge&&) = delete;
    MidiBridge& operator=(MidiBridge&&) = delete;

    void setAudioBridge(AudioBridge* audioBridge);
    void clearAudioBridge() { audioBridge_ = nullptr; }

    void setMidiToPluginsEnabled(bool enabled) { forwardMidiToPlugins_ = enabled; }

    // =========================================================================
    // MIDI Device Enumeration
    // =========================================================================

    std::vector<MidiDeviceInfo> getAvailableMidiInputs() const;
    std::vector<MidiDeviceInfo> getAvailableMidiOutputs() const;

    struct Listener {
        virtual ~Listener() = default;
        virtual void midiDeviceListChanged() = 0;
    };

    void addMidiDeviceListListener(Listener* l) { midiDeviceListListeners_.add(l); }
    void removeMidiDeviceListListener(Listener* l) { midiDeviceListListeners_.remove(l); }
    void notifyMidiDeviceListChanged() {
        midiDeviceListListeners_.call([](Listener& l) { l.midiDeviceListChanged(); });
    }

    // =========================================================================
    // MIDI Output
    // =========================================================================

    bool sendMidi(const juce::String& deviceNameOrId, const juce::MidiMessage& msg);
    bool sendSysEx(const juce::String& deviceNameOrId, const juce::uint8* data, size_t numBytes);

    // =========================================================================
    // MIDI Device Enable/Disable
    // =========================================================================

    void enableMidiInput(const juce::String& deviceId);
    void disableMidiInput(const juce::String& deviceId);
    void stopAllInputs();
    bool isMidiInputEnabled(const juce::String& deviceId) const;

    // =========================================================================
    // Track MIDI Routing
    // =========================================================================

    void setTrackMidiInput(TrackId trackId, const juce::String& midiDeviceId);
    juce::String getTrackMidiInput(TrackId trackId) const;
    void clearTrackMidiInput(TrackId trackId);

    // =========================================================================
    // MIDI Monitoring
    // =========================================================================

    std::function<void(TrackId, const MidiNoteEvent&)> onNoteEvent;
    std::function<void(TrackId, const MidiCCEvent&)> onCCEvent;

    void startMonitoring(TrackId trackId);
    void stopMonitoring(TrackId trackId);
    bool isMonitoring(TrackId trackId) const;

    // =========================================================================
    // Raw MIDI listener
    // =========================================================================

    void addRawMidiListener(RawMidiListener* listener);
    void removeRawMidiListener(RawMidiListener* listener);

    void broadcastSynthesizedNote(const juce::String& sourceDeviceId, int noteNumber,
                                  int velocity, bool isNoteOn);

  private:
    void handleIncomingMidiMessage(juce::MidiInput* source,
                                   const juce::MidiMessage& message) override;

#ifdef AIDAW_HAS_TRACKTION
    te::Engine& engine_;
#endif

    AudioBridge* audioBridge_ = nullptr;

    std::unordered_map<TrackId, juce::String> trackMidiInputs_;
    std::unordered_set<TrackId> monitoredTracks_;
    std::unordered_map<juce::String, std::unique_ptr<juce::MidiInput>> activeMidiInputs_;
    std::unordered_map<juce::String, std::unique_ptr<juce::MidiOutput>> activeMidiOutputs_;

    mutable juce::CriticalSection routingLock_;
    bool forwardMidiToPlugins_ = true;

    std::atomic<bool> isShuttingDown_{false};
    std::atomic<int> activeCallbacks_{0};

    juce::ListenerList<Listener> midiDeviceListListeners_;

    juce::Array<RawMidiListener*> rawMidiListeners_;
    juce::CriticalSection rawMidiListenersLock_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiBridge)
};

}  // namespace aidaw
