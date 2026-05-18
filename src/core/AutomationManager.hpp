#pragma once

#include <juce_events/juce_events.h>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "AutomationInfo.hpp"
#include "AutomationTypes.hpp"
#include "TypeIds.hpp"

namespace aidaw {

/**
 * @brief Listener interface for automation changes
 */
class AutomationManagerListener {
  public:
    virtual ~AutomationManagerListener() = default;

    virtual void automationLanesChanged() = 0;
    virtual void automationLanePropertyChanged(AutomationLaneId laneId) {
        juce::ignoreUnused(laneId);
    }
    virtual void automationClipsChanged(AutomationLaneId laneId) {
        juce::ignoreUnused(laneId);
    }
    virtual void automationPointsChanged(AutomationLaneId laneId) {
        juce::ignoreUnused(laneId);
    }
    virtual void automationPointDragPreview(AutomationLaneId laneId, AutomationPointId pointId,
                                            double previewTime, double previewValue) {
        juce::ignoreUnused(laneId, pointId, previewTime, previewValue);
    }
    virtual void automationValueChanged(AutomationLaneId laneId, double normalizedValue) {
        juce::ignoreUnused(laneId, normalizedValue);
    }
};

/**
 * @brief Singleton manager for automation data
 */
class AutomationManager {
  public:
    static AutomationManager& getInstance();

    AutomationManager(const AutomationManager&) = delete;
    AutomationManager& operator=(const AutomationManager&) = delete;

    // Lane Management
    AutomationLaneId createLane(const AutomationTarget& target, AutomationLaneType type);
    AutomationLaneId getOrCreateLane(const AutomationTarget& target, AutomationLaneType type);
    void deleteLane(AutomationLaneId laneId);

    AutomationLaneInfo* getLane(AutomationLaneId laneId);
    const AutomationLaneInfo* getLane(AutomationLaneId laneId) const;

    const std::vector<AutomationLaneInfo>& getLanes() const { return lanes_; }
    const std::vector<AutomationClipInfo>& getClips() const { return clips_; }

    std::vector<AutomationLaneId> getLanesForTrack(TrackId trackId) const;
    AutomationLaneId getLaneForTarget(const AutomationTarget& target) const;

    // Lane Properties
    void setLaneName(AutomationLaneId laneId, const juce::String& name);
    void setLaneVisible(AutomationLaneId laneId, bool visible);
    void setLaneExpanded(AutomationLaneId laneId, bool expanded);
    void setLaneBypass(AutomationLaneId laneId, bool bypass);
    void setLaneHeight(AutomationLaneId laneId, int height);

    bool isGlobalLaneVisibilityEnabled() const { return globalLaneVisibilityEnabled_; }
    void setGlobalLaneVisibility(bool enabled);

    AutomationVisualState getVisualState(const AutomationTarget& target) const;

    // Automation Clips
    AutomationClipId createClip(AutomationLaneId laneId, double startBeats, double lengthBeats);
    void deleteClip(AutomationClipId clipId);
    AutomationClipInfo* getClip(AutomationClipId clipId);
    const AutomationClipInfo* getClip(AutomationClipId clipId) const;
    void moveClip(AutomationClipId clipId, double newStartTime);
    void resizeClip(AutomationClipId clipId, double newLength, bool fromStart = false);
    AutomationClipId duplicateClip(AutomationClipId clipId);

    // Point Management
    AutomationPointId addPoint(AutomationLaneId laneId, double beatPosition, double value,
                               AutomationCurveType curveType = AutomationCurveType::Linear);
    AutomationPointId addPointToClip(AutomationClipId clipId, double localBeatPosition,
                                     double value,
                                     AutomationCurveType curveType = AutomationCurveType::Linear);
    void deletePoint(AutomationLaneId laneId, AutomationPointId pointId);
    void clearLanePoints(AutomationLaneId laneId);
    void deletePointFromClip(AutomationClipId clipId, AutomationPointId pointId);
    void movePoint(AutomationLaneId laneId, AutomationPointId pointId,
                   double newBeatPosition, double newValue);
    void movePointInClip(AutomationClipId clipId, AutomationPointId pointId,
                         double newBeatPosition, double newValue);
    void setPointCurveType(AutomationLaneId laneId, AutomationPointId pointId,
                           AutomationCurveType curveType);
    void setPointTension(AutomationLaneId laneId, AutomationPointId pointId, double tension);
    void setPointHandles(AutomationLaneId laneId, AutomationPointId pointId,
                         const BezierHandle& inHandle, const BezierHandle& outHandle);

    // Value Interpolation
    double getValueAtBeat(AutomationLaneId laneId, double beatPosition) const;
    double getClipValueAtBeat(AutomationClipId clipId, double localBeatPosition) const;

    // Listener Management
    void addListener(AutomationManagerListener* listener);
    void removeListener(AutomationManagerListener* listener);

    void beginNotificationBatch();
    void endNotificationBatch();

    // Project Management
    void clearAll();
    void restoreLane(AutomationLaneInfo& lane);
    void restoreClip(AutomationClipInfo& clip);
    void refreshIdCountersFromLanes();

  private:
    AutomationManager();
    ~AutomationManager() = default;

    std::vector<AutomationLaneInfo> lanes_;
    std::vector<AutomationClipInfo> clips_;
    juce::ListenerList<AutomationManagerListener> listeners_;

    bool globalLaneVisibilityEnabled_ = true;

    int nextLaneId_ = 1;
    int nextClipId_ = 1;
    int nextPointId_ = 1;

    int notificationBatchDepth_ = 0;
    std::vector<AutomationLaneId> pendingPointsChangedLanes_;

    void notifyLanesChanged();
    void notifyLanePropertyChanged(AutomationLaneId laneId);
    void notifyClipsChanged(AutomationLaneId laneId);
    void notifyPointsChanged(AutomationLaneId laneId);

    double interpolatePoints(const std::vector<AutomationPoint>& points, double beatPosition) const;
    AutomationPoint* findPoint(std::vector<AutomationPoint>& points, AutomationPointId pointId);
    void sortPoints(std::vector<AutomationPoint>& points);
};

}  // namespace aidaw
