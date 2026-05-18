#pragma once

#include <algorithm>
#include <unordered_set>
#include <vector>

#include "TypeIds.hpp"

namespace aidaw {

/**
 * @brief Selection types in the DAW
 */
enum class SelectionType {
    None,
    Track,
    MultiTrack,
    Clip,
    MultiClip,
    TimeRange,
    Note,
    AutomationLane,
    AutomationPoint
};

/**
 * @brief MIDI note selection data
 */
struct NoteSelection {
    ClipId clipId = INVALID_CLIP_ID;
    std::vector<size_t> noteIndices;

    bool isValid() const { return clipId != INVALID_CLIP_ID && !noteIndices.empty(); }
    size_t getCount() const { return noteIndices.size(); }
};

/**
 * @brief Time range selection data
 */
struct TimeRangeSelection {
    double startBeats = 0.0;
    double endBeats = 0.0;
    std::vector<TrackId> trackIds;

    bool isValid() const { return endBeats > startBeats && !trackIds.empty(); }
    double getLengthBeats() const { return endBeats - startBeats; }
};

/**
 * @brief Automation point selection data
 */
struct AutomationPointSelection {
    AutomationLaneId laneId = INVALID_AUTOMATION_LANE_ID;
    AutomationClipId clipId = INVALID_AUTOMATION_CLIP_ID;
    std::vector<AutomationPointId> pointIds;

    bool isValid() const { return laneId != INVALID_AUTOMATION_LANE_ID && !pointIds.empty(); }
    size_t getCount() const { return pointIds.size(); }
};

/**
 * @brief Listener interface for selection changes
 */
class SelectionManagerListener {
  public:
    virtual ~SelectionManagerListener() = default;

    virtual void selectionTypeChanged(SelectionType newType) = 0;
    virtual void trackSelectionChanged([[maybe_unused]] TrackId trackId) {}
    virtual void clipSelectionChanged([[maybe_unused]] ClipId clipId) {}
    virtual void timeRangeSelectionChanged([[maybe_unused]] const TimeRangeSelection& selection) {}
    virtual void noteSelectionChanged([[maybe_unused]] const NoteSelection& selection) {}
};

/**
 * @brief Singleton manager that coordinates selection state across the DAW
 */
class SelectionManager {
  public:
    static SelectionManager& getInstance();

    SelectionManager(const SelectionManager&) = delete;
    SelectionManager& operator=(const SelectionManager&) = delete;

    SelectionType getSelectionType() const { return selectionType_; }

    // Track Selection
    void selectTrack(TrackId trackId);
    TrackId getSelectedTrack() const { return selectedTrackId_; }
    void selectTracks(const std::unordered_set<TrackId>& trackIds);
    void addTrackToSelection(TrackId trackId);
    void removeTrackFromSelection(TrackId trackId);
    bool isTrackSelected(TrackId trackId) const;
    const std::unordered_set<TrackId>& getSelectedTracks() const { return selectedTrackIds_; }

    // Clip Selection
    void selectClip(ClipId clipId);
    ClipId getSelectedClip() const { return selectedClipId_; }
    void selectClips(const std::unordered_set<ClipId>& clipIds);
    void addClipToSelection(ClipId clipId);
    void removeClipFromSelection(ClipId clipId);
    bool isClipSelected(ClipId clipId) const;
    const std::unordered_set<ClipId>& getSelectedClips() const { return selectedClipIds_; }

    // Time Range Selection
    void selectTimeRange(double startBeats, double endBeats, const std::vector<TrackId>& trackIds);
    const TimeRangeSelection& getTimeRangeSelection() const { return timeRangeSelection_; }
    bool hasTimeRangeSelection() const {
        return selectionType_ == SelectionType::TimeRange && timeRangeSelection_.isValid();
    }

    // Note Selection
    void selectNote(ClipId clipId, size_t noteIndex);
    void selectNotes(ClipId clipId, const std::vector<size_t>& noteIndices);
    const NoteSelection& getNoteSelection() const { return noteSelection_; }
    bool isNoteSelected(ClipId clipId, size_t noteIndex) const;

    // Automation Point Selection
    void selectAutomationPoint(AutomationLaneId laneId, AutomationPointId pointId,
                               AutomationClipId clipId = INVALID_AUTOMATION_CLIP_ID);
    void selectAutomationPoints(AutomationLaneId laneId,
                                const std::vector<AutomationPointId>& pointIds,
                                AutomationClipId clipId = INVALID_AUTOMATION_CLIP_ID);
    const AutomationPointSelection& getAutomationPointSelection() const {
        return automationPointSelection_;
    }
    bool isAutomationPointSelected(AutomationPointId pointId) const;

    // Clear
    void clearSelection();
    void clearNoteSelection();

    // Listeners
    void addListener(SelectionManagerListener* listener);
    void removeListener(SelectionManagerListener* listener);

  private:
    SelectionManager() = default;
    ~SelectionManager() = default;

    SelectionType selectionType_ = SelectionType::None;
    TrackId selectedTrackId_ = INVALID_TRACK_ID;
    std::unordered_set<TrackId> selectedTrackIds_;
    ClipId selectedClipId_ = INVALID_CLIP_ID;
    std::unordered_set<ClipId> selectedClipIds_;
    TimeRangeSelection timeRangeSelection_;
    NoteSelection noteSelection_;
    AutomationPointSelection automationPointSelection_;

    std::vector<SelectionManagerListener*> listeners_;

    void notifySelectionTypeChanged(SelectionType type);
    void notifyTrackSelectionChanged(TrackId trackId);
    void notifyClipSelectionChanged(ClipId clipId);
};

}  // namespace aidaw
