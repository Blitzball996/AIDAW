#include "WarpMarkerManager.hpp"

namespace aidaw {

WarpMarkerManager::~WarpMarkerManager() {
#ifdef AIDAW_HAS_TRACKTION
    coalescingTimer_.stopTimer();
#endif
}

#ifdef AIDAW_HAS_TRACKTION

bool WarpMarkerManager::getTransientTimes(te::Edit& edit,
                                           const std::map<ClipId, std::string>& clipIdToEngineId,
                                           ClipId clipId) {
    auto it = clipIdToEngineId.find(clipId);
    if (it == clipIdToEngineId.end())
        return false;

    // Check if detection already completed (cached)
    if (detectionStarted_.count(clipId) > 0 && detectionInFlight_.count(clipId) == 0)
        return true;

    // Kick off detection if not started
    if (detectionStarted_.count(clipId) == 0) {
        detectionStarted_.insert(clipId);
        detectionInFlight_.insert(clipId);
        // TODO: Start async transient detection via TE WarpTimeManager
    }

    return false;
}

void WarpMarkerManager::setTransientSensitivity(
    te::Edit& edit, const std::map<ClipId, std::string>& clipIdToEngineId, ClipId clipId,
    float sensitivity) {
    auto it = clipIdToEngineId.find(clipId);
    if (it == clipIdToEngineId.end())
        return;

    PendingDetection pending;
    pending.sensitivity = sensitivity;
    pending.engineId = it->second;
    pending.edit = &edit;
    pendingByClip_[clipId] = pending;

    // Coalesce rapid changes
    coalescingTimer_.callback = [this]() { applyPendingSensitivities(); };
    coalescingTimer_.startTimer(kCoalesceMs);
}

void WarpMarkerManager::applyPendingSensitivities() {
    auto pending = std::move(pendingByClip_);
    pendingByClip_.clear();

    for (auto& [clipId, detection] : pending) {
        if (detectionInFlight_.count(clipId) > 0) {
            dirtyAfterCompletion_[clipId] = detection;
        } else if (detection.edit) {
            applySensitivityNow(*detection.edit, detection.engineId, clipId, detection.sensitivity);
        }
    }
}

void WarpMarkerManager::applySensitivityNow(te::Edit& /*edit*/, const std::string& /*engineId*/,
                                             ClipId clipId, float /*sensitivity*/) {
    detectionStarted_.erase(clipId);
    detectionInFlight_.insert(clipId);
    // TODO: Re-run transient detection with new sensitivity
}

void WarpMarkerManager::enableWarp(te::Edit& /*edit*/,
                                    const std::map<ClipId, std::string>& /*clipIdToEngineId*/,
                                    ClipId /*clipId*/) {
    // TODO: Populate WarpTimeManager with markers at detected transients
}

void WarpMarkerManager::disableWarp(te::Edit& /*edit*/,
                                     const std::map<ClipId, std::string>& /*clipIdToEngineId*/,
                                     ClipId /*clipId*/) {
    // TODO: Remove all warp markers from WarpTimeManager
}

std::vector<WarpMarkerInfo> WarpMarkerManager::getWarpMarkers(
    te::Edit& /*edit*/, const std::map<ClipId, std::string>& /*clipIdToEngineId*/,
    ClipId /*clipId*/) {
    // TODO: Read warp markers from TE WarpTimeManager
    return {};
}

int WarpMarkerManager::addWarpMarker(te::Edit& /*edit*/,
                                      const std::map<ClipId, std::string>& /*clipIdToEngineId*/,
                                      ClipId /*clipId*/, double /*sourceTime*/,
                                      double /*warpTime*/) {
    // TODO: Add warp marker via TE WarpTimeManager
    return -1;
}

double WarpMarkerManager::moveWarpMarker(te::Edit& /*edit*/,
                                          const std::map<ClipId, std::string>& /*clipIdToEngineId*/,
                                          ClipId /*clipId*/, int /*index*/,
                                          double newWarpTime) {
    // TODO: Move warp marker via TE WarpTimeManager
    return newWarpTime;
}

void WarpMarkerManager::removeWarpMarker(
    te::Edit& /*edit*/, const std::map<ClipId, std::string>& /*clipIdToEngineId*/,
    ClipId /*clipId*/, int /*index*/) {
    // TODO: Remove warp marker via TE WarpTimeManager
}

#endif  // AIDAW_HAS_TRACKTION

}  // namespace aidaw
