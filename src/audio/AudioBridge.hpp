#pragma once

#include <functional>
#include <map>
#include <memory>
#include <atomic>

#include "TrackController.hpp"
#include "TransportStateManager.hpp"
#include "DeviceMeteringManager.hpp"
#include "WarpMarkerManager.hpp"

#ifdef AIDAW_HAS_TRACKTION
#include <tracktion_engine/tracktion_engine.h>
#endif

namespace aidaw {

#ifdef AIDAW_HAS_TRACKTION
namespace te = tracktion;
#endif

// Forward declarations
class Engine;

using DeviceId = int;
using ClipId = int;
constexpr DeviceId INVALID_DEVICE_ID = -1;

/**
 * @brief Warp marker information for UI display
 */
struct WarpMarkerInfo {
    double sourceTime;
    double warpTime;
};

/**
 * @brief Metering data for a single track
 */
struct MeterData {
    float peakL = 0.0f;
    float peakR = 0.0f;
    float rmsL = 0.0f;
    float rmsR = 0.0f;
    bool clipped = false;
};

/**
 * @brief Bridges UI models to the audio engine (Tracktion Engine)
 *
 * Responsibilities:
 * - Maps TrackId to audio engine track instances
 * - Maps DeviceId to plugin instances
 * - Manages metering and parameter communication
 * - Handles clip synchronization
 *
 * Thread Safety:
 * - UI thread: Receives notifications, updates mappings
 * - Audio thread: Reads mappings, processes parameter changes, pushes metering
 */
class AudioBridge : public juce::Timer {
  public:
#ifdef AIDAW_HAS_TRACKTION
    AudioBridge(te::Engine& engine, te::Edit& edit);
#else
    AudioBridge();
#endif
    ~AudioBridge() override;

    // =========================================================================
    // Clip Synchronization
    // =========================================================================

    void syncClipToEngine(ClipId clipId);
    void removeClipFromEngine(ClipId clipId);

    // =========================================================================
    // Warp Markers
    // =========================================================================

    std::vector<WarpMarkerInfo> getWarpMarkers(ClipId clipId);
    int addWarpMarker(ClipId clipId, double sourceTime, double warpTime);
    double moveWarpMarker(ClipId clipId, int index, double newWarpTime);
    void removeWarpMarker(ClipId clipId, int index);

    // =========================================================================
    // Track Mapping
    // =========================================================================

#ifdef AIDAW_HAS_TRACKTION
    te::AudioTrack* getAudioTrack(TrackId trackId) const;
    te::AudioTrack* createAudioTrack(TrackId trackId, const juce::String& name);
    void removeAudioTrack(TrackId trackId);
#endif

    // =========================================================================
    // Mixer Controls
    // =========================================================================

    void setTrackVolume(TrackId trackId, float volume);
    float getTrackVolume(TrackId trackId) const;
    void setTrackPan(TrackId trackId, float pan);
    float getTrackPan(TrackId trackId) const;
    void setMasterVolume(float volume);
    float getMasterVolume() const;
    void setMasterPan(float pan);
    float getMasterPan() const;

    // =========================================================================
    // Metering
    // =========================================================================

    DeviceMeteringManager& getDeviceMetering() { return deviceMetering_; }
    const DeviceMeteringManager& getDeviceMetering() const { return deviceMetering_; }

    float getMasterPeakL() const {
        return masterPeakL_.load(std::memory_order_relaxed);
    }
    float getMasterPeakR() const {
        return masterPeakR_.load(std::memory_order_relaxed);
    }

    // =========================================================================
    // Transport State
    // =========================================================================

    void updateTransportState(bool isPlaying, bool justStarted, bool justLooped);

    bool isTransportPlaying() const {
        return transportState_.isPlaying();
    }
    bool didJustStart() const {
        return transportState_.didJustStart();
    }
    bool didJustLoop() const {
        return transportState_.didJustLoop();
    }

    // =========================================================================
    // MIDI Activity
    // =========================================================================

    void triggerMidiActivity(TrackId trackId);
    uint32_t getMidiActivityCounter(TrackId trackId) const;

    // =========================================================================
    // Audio Routing
    // =========================================================================

    void setTrackAudioOutput(TrackId trackId, const juce::String& destination);
    void setTrackAudioInput(TrackId trackId, const juce::String& deviceId);
    juce::String getTrackAudioOutput(TrackId trackId) const;
    juce::String getTrackAudioInput(TrackId trackId) const;

    // =========================================================================
    // MIDI Routing
    // =========================================================================

    void setTrackMidiInput(TrackId trackId, const juce::String& midiDeviceId);
    juce::String getTrackMidiInput(TrackId trackId) const;
    void enableAllMidiInputDevices();

    // =========================================================================
    // Synchronization
    // =========================================================================

    void syncAll();
    void syncTrackPlugins(TrackId trackId);

    // =========================================================================
    // Audio Callback Support
    // =========================================================================

    void processParameterChanges();
    void updateMetering();

    // =========================================================================
    // Engine reference
    // =========================================================================

    void setEngine(Engine* engine) { engine_ = engine; }

  private:
    void timerCallback() override;

#ifdef AIDAW_HAS_TRACKTION
    te::Engine& teEngine_;
    te::Edit& edit_;
#endif

    Engine* engine_ = nullptr;

    // Sub-managers
    TransportStateManager transportState_;
    TrackController trackController_;
    WarpMarkerManager warpMarkerManager_;
    DeviceMeteringManager deviceMetering_;

    // Master channel metering
    std::atomic<float> masterPeakL_{0.0f};
    std::atomic<float> masterPeakR_{0.0f};

    // MIDI activity counters per track
    std::map<TrackId, std::atomic<uint32_t>> midiActivityCounters_;
    mutable juce::CriticalSection midiActivityLock_;

    // Shutdown flag
    std::atomic<bool> isShuttingDown_{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioBridge)
};

}  // namespace aidaw
