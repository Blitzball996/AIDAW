#include "AutomationManager.hpp"

#include <algorithm>
#include <cmath>

namespace aidaw {

AutomationManager& AutomationManager::getInstance() {
    static AutomationManager instance;
    return instance;
}

AutomationManager::AutomationManager() = default;

// Lane Management

AutomationLaneId AutomationManager::createLane(const AutomationTarget& target,
                                               AutomationLaneType type) {
    if (auto existing = getLaneForTarget(target); existing != INVALID_AUTOMATION_LANE_ID)
        return existing;

    AutomationLaneInfo lane;
    lane.id = nextLaneId_++;
    lane.target = target;
    lane.type = type;

    if (type == AutomationLaneType::Absolute) {
        AutomationPoint point;
        point.id = nextPointId_++;
        point.beatPosition = 0.0;
        point.value = 0.5;
        point.curveType = AutomationCurveType::Linear;
        lane.absolutePoints.push_back(point);
    }

    lanes_.push_back(lane);
    notifyLanesChanged();
    return lane.id;
}

AutomationLaneId AutomationManager::getOrCreateLane(const AutomationTarget& target,
                                                    AutomationLaneType type) {
    AutomationLaneId existingId = getLaneForTarget(target);
    if (existingId != INVALID_AUTOMATION_LANE_ID)
        return existingId;
    return createLane(target, type);
}

void AutomationManager::deleteLane(AutomationLaneId laneId) {
    auto* lane = getLane(laneId);
    if (!lane) return;

    if (lane->isClipBased()) {
        for (auto clipId : lane->clipIds) {
            clips_.erase(
                std::remove_if(clips_.begin(), clips_.end(),
                               [clipId](const AutomationClipInfo& c) { return c.id == clipId; }),
                clips_.end());
        }
    }

    lanes_.erase(std::remove_if(lanes_.begin(), lanes_.end(),
                                [laneId](const AutomationLaneInfo& l) { return l.id == laneId; }),
                 lanes_.end());
    notifyLanesChanged();
}

AutomationLaneInfo* AutomationManager::getLane(AutomationLaneId laneId) {
    for (auto& lane : lanes_)
        if (lane.id == laneId) return &lane;
    return nullptr;
}

const AutomationLaneInfo* AutomationManager::getLane(AutomationLaneId laneId) const {
    for (const auto& lane : lanes_)
        if (lane.id == laneId) return &lane;
    return nullptr;
}

std::vector<AutomationLaneId> AutomationManager::getLanesForTrack(TrackId trackId) const {
    std::vector<AutomationLaneId> result;
    for (const auto& lane : lanes_)
        if (lane.target.trackId == trackId)
            result.push_back(lane.id);
    return result;
}

AutomationLaneId AutomationManager::getLaneForTarget(const AutomationTarget& target) const {
    for (const auto& lane : lanes_)
        if (lane.target == target)
            return lane.id;
    return INVALID_AUTOMATION_LANE_ID;
}

// Lane Properties

void AutomationManager::setLaneName(AutomationLaneId laneId, const juce::String& name) {
    if (auto* lane = getLane(laneId)) {
        lane->name = name;
        notifyLanePropertyChanged(laneId);
    }
}

void AutomationManager::setLaneVisible(AutomationLaneId laneId, bool visible) {
    if (auto* lane = getLane(laneId)) {
        lane->visible = visible;
        notifyLanePropertyChanged(laneId);
    }
}

void AutomationManager::setGlobalLaneVisibility(bool enabled) {
    if (globalLaneVisibilityEnabled_ == enabled) return;
    globalLaneVisibilityEnabled_ = enabled;
    notifyLanesChanged();
}

void AutomationManager::setLaneExpanded(AutomationLaneId laneId, bool expanded) {
    if (auto* lane = getLane(laneId)) {
        lane->expanded = expanded;
        notifyLanePropertyChanged(laneId);
    }
}

void AutomationManager::setLaneBypass(AutomationLaneId laneId, bool bypass) {
    if (auto* lane = getLane(laneId)) {
        if (lane->bypass == bypass) return;
        lane->bypass = bypass;
        notifyLanePropertyChanged(laneId);
    }
}

void AutomationManager::setLaneHeight(AutomationLaneId laneId, int height) {
    if (auto* lane = getLane(laneId)) {
        lane->height = juce::jmax(30, height);
        notifyLanePropertyChanged(laneId);
    }
}

AutomationVisualState AutomationManager::getVisualState(const AutomationTarget& target) const {
    AutomationLaneId laneId = getLaneForTarget(target);
    if (laneId == INVALID_AUTOMATION_LANE_ID)
        return AutomationVisualState::None;
    const auto* lane = const_cast<AutomationManager*>(this)->getLane(laneId);
    if (!lane) return AutomationVisualState::None;
    if (lane->bypass || lane->touchSuppressed)
        return AutomationVisualState::Overridden;
    return AutomationVisualState::Active;
}

// Automation Clips

AutomationClipId AutomationManager::createClip(AutomationLaneId laneId, double startBeats,
                                               double lengthBeats) {
    auto* lane = getLane(laneId);
    if (!lane || !lane->isClipBased()) return INVALID_AUTOMATION_CLIP_ID;

    AutomationClipInfo clip;
    clip.id = nextClipId_++;
    clip.laneId = laneId;
    clip.startBeats = startBeats;
    clip.lengthBeats = lengthBeats;
    clip.colour = AutomationClipInfo::getDefaultColor(static_cast<int>(clips_.size()));
    clip.name = "Automation " + juce::String(clip.id);

    clips_.push_back(clip);
    lane->clipIds.push_back(clip.id);
    notifyClipsChanged(laneId);
    return clip.id;
}

void AutomationManager::deleteClip(AutomationClipId clipId) {
    auto* clip = getClip(clipId);
    if (!clip) return;

    AutomationLaneId laneId = clip->laneId;
    if (auto* lane = getLane(laneId)) {
        lane->clipIds.erase(std::remove(lane->clipIds.begin(), lane->clipIds.end(), clipId),
                            lane->clipIds.end());
    }

    clips_.erase(std::remove_if(clips_.begin(), clips_.end(),
                                [clipId](const AutomationClipInfo& c) { return c.id == clipId; }),
                 clips_.end());
    notifyClipsChanged(laneId);
}

AutomationClipInfo* AutomationManager::getClip(AutomationClipId clipId) {
    for (auto& clip : clips_)
        if (clip.id == clipId) return &clip;
    return nullptr;
}

const AutomationClipInfo* AutomationManager::getClip(AutomationClipId clipId) const {
    for (const auto& clip : clips_)
        if (clip.id == clipId) return &clip;
    return nullptr;
}

void AutomationManager::moveClip(AutomationClipId clipId, double newStartTime) {
    if (auto* clip = getClip(clipId)) {
        clip->startBeats = juce::jmax(0.0, newStartTime);
        notifyClipsChanged(clip->laneId);
    }
}

void AutomationManager::resizeClip(AutomationClipId clipId, double newLength, bool fromStart) {
    if (auto* clip = getClip(clipId)) {
        double minLength = 0.1;
        newLength = juce::jmax(minLength, newLength);
        if (fromStart) {
            double endBeats = clip->getEndBeats();
            clip->startBeats = endBeats - newLength;
            if (clip->startBeats < 0.0) {
                clip->startBeats = 0.0;
                newLength = endBeats;
            }
        }
        clip->lengthBeats = newLength;
        notifyClipsChanged(clip->laneId);
    }
}

AutomationClipId AutomationManager::duplicateClip(AutomationClipId clipId) {
    auto* sourceClip = getClip(clipId);
    if (!sourceClip) return INVALID_AUTOMATION_CLIP_ID;

    AutomationClipInfo newClip = *sourceClip;
    newClip.id = nextClipId_++;
    newClip.startBeats = sourceClip->getEndBeats();
    newClip.name = sourceClip->name + " copy";

    for (auto& point : newClip.points)
        point.id = nextPointId_++;

    clips_.push_back(newClip);
    if (auto* lane = getLane(newClip.laneId))
        lane->clipIds.push_back(newClip.id);

    notifyClipsChanged(newClip.laneId);
    return newClip.id;
}

// Point Management

AutomationPointId AutomationManager::addPoint(AutomationLaneId laneId, double beatPosition,
                                              double value, AutomationCurveType curveType) {
    auto* lane = getLane(laneId);
    if (!lane || !lane->isAbsolute()) return INVALID_AUTOMATION_POINT_ID;

    AutomationPoint point;
    point.id = nextPointId_++;
    point.beatPosition = juce::jmax(0.0, beatPosition);
    point.value = juce::jlimit(0.0, 1.0, value);
    point.curveType = curveType;

    lane->absolutePoints.push_back(point);
    sortPoints(lane->absolutePoints);
    notifyPointsChanged(laneId);
    return point.id;
}

AutomationPointId AutomationManager::addPointToClip(AutomationClipId clipId,
                                                    double localBeatPosition, double value,
                                                    AutomationCurveType curveType) {
    auto* clip = getClip(clipId);
    if (!clip) return INVALID_AUTOMATION_POINT_ID;

    AutomationPoint point;
    point.id = nextPointId_++;
    point.beatPosition = juce::jlimit(0.0, clip->lengthBeats, localBeatPosition);
    point.value = juce::jlimit(0.0, 1.0, value);
    point.curveType = curveType;

    clip->points.push_back(point);
    sortPoints(clip->points);
    notifyClipsChanged(clip->laneId);
    return point.id;
}

void AutomationManager::deletePoint(AutomationLaneId laneId, AutomationPointId pointId) {
    auto* lane = getLane(laneId);
    if (!lane || !lane->isAbsolute()) return;

    lane->absolutePoints.erase(
        std::remove_if(lane->absolutePoints.begin(), lane->absolutePoints.end(),
                       [pointId](const AutomationPoint& p) { return p.id == pointId; }),
        lane->absolutePoints.end());
    notifyPointsChanged(laneId);
}

void AutomationManager::clearLanePoints(AutomationLaneId laneId) {
    auto* lane = getLane(laneId);
    if (!lane || !lane->isAbsolute() || lane->absolutePoints.empty()) return;
    lane->absolutePoints.clear();
    notifyPointsChanged(laneId);
}

void AutomationManager::deletePointFromClip(AutomationClipId clipId, AutomationPointId pointId) {
    auto* clip = getClip(clipId);
    if (!clip) return;

    clip->points.erase(
        std::remove_if(clip->points.begin(), clip->points.end(),
                       [pointId](const AutomationPoint& p) { return p.id == pointId; }),
        clip->points.end());
    notifyClipsChanged(clip->laneId);
}

void AutomationManager::movePoint(AutomationLaneId laneId, AutomationPointId pointId,
                                  double newBeatPosition, double newValue) {
    auto* lane = getLane(laneId);
    if (!lane || !lane->isAbsolute()) return;

    if (auto* point = findPoint(lane->absolutePoints, pointId)) {
        point->beatPosition = juce::jmax(0.0, newBeatPosition);
        point->value = juce::jlimit(0.0, 1.0, newValue);
        sortPoints(lane->absolutePoints);
        notifyPointsChanged(laneId);
    }
}

void AutomationManager::movePointInClip(AutomationClipId clipId, AutomationPointId pointId,
                                        double newBeatPosition, double newValue) {
    auto* clip = getClip(clipId);
    if (!clip) return;

    if (auto* point = findPoint(clip->points, pointId)) {
        point->beatPosition = juce::jlimit(0.0, clip->lengthBeats, newBeatPosition);
        point->value = juce::jlimit(0.0, 1.0, newValue);
        sortPoints(clip->points);
        notifyClipsChanged(clip->laneId);
    }
}

void AutomationManager::setPointCurveType(AutomationLaneId laneId, AutomationPointId pointId,
                                          AutomationCurveType curveType) {
    auto* lane = getLane(laneId);
    if (!lane || !lane->isAbsolute()) return;
    if (auto* point = findPoint(lane->absolutePoints, pointId)) {
        point->curveType = curveType;
        notifyPointsChanged(laneId);
    }
}

void AutomationManager::setPointTension(AutomationLaneId laneId, AutomationPointId pointId,
                                        double tension) {
    auto* lane = getLane(laneId);
    if (!lane || !lane->isAbsolute()) return;
    if (auto* point = findPoint(lane->absolutePoints, pointId)) {
        point->tension = juce::jlimit(-3.0, 3.0, tension);
        notifyPointsChanged(laneId);
    }
}

void AutomationManager::setPointHandles(AutomationLaneId laneId, AutomationPointId pointId,
                                        const BezierHandle& inHandle,
                                        const BezierHandle& outHandle) {
    auto* lane = getLane(laneId);
    if (!lane || !lane->isAbsolute()) return;
    if (auto* point = findPoint(lane->absolutePoints, pointId)) {
        point->inHandle = inHandle;
        point->outHandle = outHandle;
        notifyPointsChanged(laneId);
    }
}

// Value Interpolation

static double interpolateWithTension(double t, double v1, double v2, double tension) {
    if (std::abs(tension) < 0.001)
        return v1 + t * (v2 - v1);

    double curvedT;
    if (tension > 0)
        curvedT = std::pow(t, 1.0 + tension * 2.0);
    else
        curvedT = 1.0 - std::pow(1.0 - t, 1.0 - tension * 2.0);

    return v1 + curvedT * (v2 - v1);
}

double AutomationManager::interpolatePoints(const std::vector<AutomationPoint>& points,
                                            double beatPosition) const {
    if (points.empty()) return 0.5;
    if (beatPosition <= points.front().beatPosition) return points.front().value;
    if (beatPosition >= points.back().beatPosition) return points.back().value;

    for (size_t i = 0; i < points.size() - 1; ++i) {
        const auto& p1 = points[i];
        const auto& p2 = points[i + 1];

        if (beatPosition >= p1.beatPosition && beatPosition < p2.beatPosition) {
            double duration = p2.beatPosition - p1.beatPosition;
            if (duration <= 0.0) return p1.value;
            double t = (beatPosition - p1.beatPosition) / duration;

            switch (p1.curveType) {
                case AutomationCurveType::Linear:
                    return interpolateWithTension(t, p1.value, p2.value, p1.tension);
                case AutomationCurveType::Bezier: {
                    double t2 = t * t, t3 = t2 * t;
                    double mt = 1.0 - t, mt2 = mt * mt, mt3 = mt2 * mt;
                    double cp1 = p1.value + p1.outHandle.value;
                    double cp2 = p2.value + p2.inHandle.value;
                    return mt3 * p1.value + 3.0 * mt2 * t * cp1 +
                           3.0 * mt * t2 * cp2 + t3 * p2.value;
                }
                case AutomationCurveType::Step:
                    return p1.value;
            }
        }
    }
    return 0.5;
}

double AutomationManager::getValueAtBeat(AutomationLaneId laneId, double beatPosition) const {
    const auto* lane = getLane(laneId);
    if (!lane) return 0.5;

    if (lane->isAbsolute())
        return interpolatePoints(lane->absolutePoints, beatPosition);

    for (auto clipId : lane->clipIds) {
        const auto* clip = getClip(clipId);
        if (clip && clip->containsBeat(beatPosition)) {
            double localBeat = clip->getLocalBeat(beatPosition);
            return interpolatePoints(clip->points, localBeat);
        }
    }
    return 0.5;
}

double AutomationManager::getClipValueAtBeat(AutomationClipId clipId,
                                             double localBeatPosition) const {
    const auto* clip = getClip(clipId);
    if (!clip) return 0.5;
    return interpolatePoints(clip->points, localBeatPosition);
}

// Listener Management

void AutomationManager::addListener(AutomationManagerListener* listener) {
    listeners_.add(listener);
}

void AutomationManager::removeListener(AutomationManagerListener* listener) {
    listeners_.remove(listener);
}

void AutomationManager::notifyLanesChanged() {
    listeners_.call([](AutomationManagerListener& l) { l.automationLanesChanged(); });
}

void AutomationManager::notifyLanePropertyChanged(AutomationLaneId laneId) {
    listeners_.call([laneId](AutomationManagerListener& l) {
        l.automationLanePropertyChanged(laneId);
    });
}

void AutomationManager::notifyClipsChanged(AutomationLaneId laneId) {
    listeners_.call([laneId](AutomationManagerListener& l) {
        l.automationClipsChanged(laneId);
    });
}

void AutomationManager::notifyPointsChanged(AutomationLaneId laneId) {
    if (notificationBatchDepth_ > 0) {
        if (std::find(pendingPointsChangedLanes_.begin(), pendingPointsChangedLanes_.end(),
                      laneId) == pendingPointsChangedLanes_.end())
            pendingPointsChangedLanes_.push_back(laneId);
        return;
    }
    listeners_.call([laneId](AutomationManagerListener& l) {
        l.automationPointsChanged(laneId);
    });
}

void AutomationManager::beginNotificationBatch() {
    ++notificationBatchDepth_;
}

void AutomationManager::endNotificationBatch() {
    if (notificationBatchDepth_ == 0) return;
    --notificationBatchDepth_;
    if (notificationBatchDepth_ > 0) return;
    auto pending = std::move(pendingPointsChangedLanes_);
    pendingPointsChangedLanes_.clear();
    for (auto laneId : pending) {
        listeners_.call([laneId](AutomationManagerListener& l) {
            l.automationPointsChanged(laneId);
        });
    }
}

// Project Management

void AutomationManager::clearAll() {
    lanes_.clear();
    clips_.clear();
    nextLaneId_ = 1;
    nextClipId_ = 1;
    nextPointId_ = 1;
    notifyLanesChanged();
}

void AutomationManager::restoreLane(AutomationLaneInfo& lane) {
    if (getLaneForTarget(lane.target) != INVALID_AUTOMATION_LANE_ID)
        return;
    lanes_.push_back(std::move(lane));
    notifyLanesChanged();
}

void AutomationManager::restoreClip(AutomationClipInfo& clip) {
    clips_.push_back(std::move(clip));
    notifyClipsChanged(clip.laneId);
}

void AutomationManager::refreshIdCountersFromLanes() {
    int maxLaneId = 0, maxClipId = 0, maxPointId = 0;

    for (const auto& lane : lanes_) {
        if (lane.id > maxLaneId) maxLaneId = lane.id;
        for (const auto& point : lane.absolutePoints)
            if (point.id > maxPointId) maxPointId = point.id;
        for (auto clipId : lane.clipIds)
            if (clipId > maxClipId) maxClipId = clipId;
    }
    for (const auto& clip : clips_) {
        if (clip.id > maxClipId) maxClipId = clip.id;
        for (const auto& point : clip.points)
            if (point.id > maxPointId) maxPointId = point.id;
    }

    nextLaneId_ = maxLaneId + 1;
    nextClipId_ = maxClipId + 1;
    nextPointId_ = maxPointId + 1;
}

// Helpers

AutomationPoint* AutomationManager::findPoint(std::vector<AutomationPoint>& points,
                                              AutomationPointId pointId) {
    for (auto& point : points)
        if (point.id == pointId) return &point;
    return nullptr;
}

void AutomationManager::sortPoints(std::vector<AutomationPoint>& points) {
    std::sort(points.begin(), points.end());
}

}  // namespace aidaw
