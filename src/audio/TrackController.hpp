#pragma once

#include <juce_core/juce_core.h>

#include <functional>
#include <map>
#include <vector>

#ifdef AIDAW_HAS_TRACKTION
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion;
#endif

namespace aidaw {

// Type aliases (matching project conventions)
using TrackId = int;

/**
 * @brief Manages track lifecycle, mapping, mixer controls, and audio routing
 *
 * Responsibilities:
 * - Core track lifecycle (create, remove, lookup, ensure mapping)
 * - Track mapping (TrackId -> TE AudioTrack*)
 * - Mixer controls (volume and pan for tracks)
 * - Audio routing (input/output device assignment)
 * - Metering coordination
 * - Thread-safe iteration over track mapping
 *
 * Thread Safety:
 * - All operations protected by internal trackLock_
 * - createAudioTrack() uses single lock pattern for thread safety
 * - withTrackMapping() provides lock-protected callback iteration
 */
class TrackController {
  public:
#ifdef AIDAW_HAS_TRACKTION
    TrackController(te::Engine& engine, te::Edit& edit);
#else
    TrackController();
#endif

    // =========================================================================
    // Core Track Lifecycle
    // =========================================================================

#ifdef AIDAW_HAS_TRACKTION
    te::AudioTrack* getAudioTrack(TrackId trackId) const;
    te::AudioTrack* createAudioTrack(TrackId trackId, const juce::String& name);
    void removeAudioTrack(TrackId trackId);
    te::AudioTrack* ensureTrackMapping(TrackId trackId, const juce::String& name);
#endif

    // =========================================================================
    // Mixer Controls
    // =========================================================================

    void setTrackVolume(TrackId trackId, float volume);
    float getTrackVolume(TrackId trackId) const;
    void setTrackPan(TrackId trackId, float pan);
    float getTrackPan(TrackId trackId) const;

    // =========================================================================
    // Audio Routing
    // =========================================================================

    void setTrackAudioOutput(TrackId trackId, const juce::String& destination);
    juce::String getTrackAudioOutput(TrackId trackId) const;
    void setTrackAudioInput(TrackId trackId, const juce::String& deviceId);
    juce::String getTrackAudioInput(TrackId trackId) const;

    // =========================================================================
    // Utilities
    // =========================================================================

    std::vector<TrackId> getAllTrackIds() const;
    void clearAllMappings();

#ifdef AIDAW_HAS_TRACKTION
    void withTrackMapping(
        std::function<void(const std::map<TrackId, te::AudioTrack*>&)> callback) const;

    // =========================================================================
    // Metering Coordination
    // =========================================================================

    struct MeterClientEntry {
        te::LevelMeasurer::Client client;
        te::LevelMeasurer* measurer = nullptr;
    };

    void addMeterClient(TrackId trackId, te::LevelMeterPlugin* levelMeter);
    void removeMeterClient(TrackId trackId);
    void withMeterClients(std::function<void(std::map<TrackId, MeterClientEntry>&)> callback);
#endif

  private:
#ifdef AIDAW_HAS_TRACKTION
    te::Engine& engine_;
    te::Edit& edit_;
    std::map<TrackId, te::AudioTrack*> trackMapping_;
    std::map<TrackId, MeterClientEntry> meterClients_;
#endif
    mutable juce::CriticalSection trackLock_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackController)
};

}  // namespace aidaw
