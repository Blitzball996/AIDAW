#pragma once

#include <juce_core/juce_core.h>

#include <memory>
#include <vector>

#ifdef AIDAW_HAS_TRACKTION
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion;
#endif

namespace aidaw {

using TrackId = int;

class TrackController;

/**
 * @brief Routes MIDI input devices to tracks via Tracktion Engine
 *
 * Manages the mapping between MIDI input devices and track destinations,
 * including the virtual QWERTY keyboard device.
 */
class MidiInputRouter {
  public:
#ifdef AIDAW_HAS_TRACKTION
    MidiInputRouter(te::Engine& engine, te::Edit& edit, TrackController& trackController);
#else
    MidiInputRouter();
#endif

#ifdef AIDAW_HAS_TRACKTION
    te::VirtualMidiInputDevice* getQwertyMidiDevice();
#endif

    void enableAllMidiInputDevices();
    void setTrackMidiInput(TrackId trackId, const juce::String& midiDeviceId);
    juce::String getTrackMidiInput(TrackId trackId) const;

    void setSurfaceOnlyMidiInputPort(const juce::String& midiDeviceIdOrName);
    void clearSurfaceOnlyMidiInputPorts();

    void updateForSelection();
    void resyncAllInputMonitors();
    void onMidiDevicesAvailable();
    void applyPendingRoutes();

  private:
#ifdef AIDAW_HAS_TRACKTION
    bool isSurfaceOnlyMidiInput(const juce::String& liveIdentifier,
                                const juce::String& liveName) const;
    void removeSurfaceOnlyMidiInputTargets();

    te::Engine& engine_;
    te::Edit& edit_;
    TrackController& trackController_;

    std::shared_ptr<te::MidiInputDevice> qwertyMidiDevice_;
    bool qwertyNeedsContextRefresh_ = false;

    juce::StringArray surfaceOnlyMidiInputPorts_;
    mutable juce::CriticalSection surfaceOnlyMidiInputLock_;

    std::vector<std::pair<TrackId, juce::String>> pendingMidiRoutes_;
    te::EditPlaybackContext* lastPlaybackContext_ = nullptr;
#endif
};

}  // namespace aidaw
