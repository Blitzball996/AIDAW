#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#ifdef AIDAW_HAS_TRACKTION
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion;
#endif

namespace aidaw {

using ClipId = int;

/**
 * @brief Warp marker information for UI display
 */
struct WarpMarkerInfo {
    double sourceTime;
    double warpTime;
};

/**
 * @brief Manages warp markers and transient detection for audio clips
 *
 * Responsibilities:
 * - Transient detection (async via Tracktion Engine's WarpTimeManager)
 * - Warp marker enable/disable
 * - Warp marker CRUD operations (add, move, remove, get)
 * - Caching of transient times
 *
 * Thread Safety:
 * - All operations run on message thread (UI thread)
 */
class WarpMarkerManager {
  public:
    WarpMarkerManager() = default;
    ~WarpMarkerManager();

#ifdef AIDAW_HAS_TRACKTION
    bool getTransientTimes(te::Edit& edit, const std::map<ClipId, std::string>& clipIdToEngineId,
                           ClipId clipId);

    void setTransientSensitivity(te::Edit& edit,
                                 const std::map<ClipId, std::string>& clipIdToEngineId,
                                 ClipId clipId, float sensitivity);

    void enableWarp(te::Edit& edit, const std::map<ClipId, std::string>& clipIdToEngineId,
                    ClipId clipId);

    void disableWarp(te::Edit& edit, const std::map<ClipId, std::string>& clipIdToEngineId,
                     ClipId clipId);

    std::vector<WarpMarkerInfo> getWarpMarkers(
        te::Edit& edit, const std::map<ClipId, std::string>& clipIdToEngineId, ClipId clipId);

    int addWarpMarker(te::Edit& edit, const std::map<ClipId, std::string>& clipIdToEngineId,
                      ClipId clipId, double sourceTime, double warpTime);

    double moveWarpMarker(te::Edit& edit, const std::map<ClipId, std::string>& clipIdToEngineId,
                          ClipId clipId, int index, double newWarpTime);

    void removeWarpMarker(te::Edit& edit, const std::map<ClipId, std::string>& clipIdToEngineId,
                          ClipId clipId, int index);
#endif

  private:
    std::set<ClipId> detectionStarted_;

#ifdef AIDAW_HAS_TRACKTION
    struct PendingDetection {
        float sensitivity = 0.0f;
        std::string engineId;
        te::Edit* edit = nullptr;
    };

    static constexpr int kCoalesceMs = 75;

    class CoalescingTimer : public juce::Timer {
      public:
        std::function<void()> callback;
        void timerCallback() override {
            stopTimer();
            if (callback)
                callback();
        }
    };

    void applyPendingSensitivities();
    void applySensitivityNow(te::Edit& edit, const std::string& engineId, ClipId clipId,
                             float sensitivity);

    std::map<ClipId, PendingDetection> pendingByClip_;
    std::set<ClipId> detectionInFlight_;
    std::map<ClipId, PendingDetection> dirtyAfterCompletion_;
    CoalescingTimer coalescingTimer_;
#endif
};

}  // namespace aidaw
