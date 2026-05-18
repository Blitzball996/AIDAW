#include "SelectionManager.hpp"

#include <algorithm>

namespace aidaw {

SelectionManager& SelectionManager::getInstance() {
    static SelectionManager instance;
    return instance;
}

// Track Selection

void SelectionManager::selectTrack(TrackId trackId) {
    selectedClipId_ = INVALID_CLIP_ID;
    selectedClipIds_.clear();
    timeRangeSelection_ = TimeRangeSelection{};

    selectionType_ = SelectionType::Track;
    selectedTrackId_ = trackId;
    selectedTrackIds_.clear();
    if (trackId != INVALID_TRACK_ID)
        selectedTrackIds_.insert(trackId);

    notifySelectionTypeChanged(SelectionType::Track);
    notifyTrackSelectionChanged(trackId);
}

void SelectionManager::selectTracks(const std::unordered_set<TrackId>& trackIds) {
    if (trackIds.empty()) { clearSelection(); return; }
    if (trackIds.size() == 1) { selectTrack(*trackIds.begin()); return; }

    selectedClipId_ = INVALID_CLIP_ID;
    selectedClipIds_.clear();
    timeRangeSelection_ = TimeRangeSelection{};

    selectionType_ = SelectionType::MultiTrack;
    selectedTrackIds_ = trackIds;
    if (selectedTrackIds_.count(selectedTrackId_) == 0)
        selectedTrackId_ = *selectedTrackIds_.begin();

    notifySelectionTypeChanged(SelectionType::MultiTrack);
}

void SelectionManager::addTrackToSelection(TrackId trackId) {
    if (trackId == INVALID_TRACK_ID) return;
    if (selectionType_ == SelectionType::Track && selectedTrackId_ != INVALID_TRACK_ID)
        selectedTrackIds_.insert(selectedTrackId_);

    selectedTrackIds_.insert(trackId);
    selectedTrackId_ = trackId;

    if (selectedTrackIds_.size() == 1) {
        selectionType_ = SelectionType::Track;
        notifySelectionTypeChanged(SelectionType::Track);
        notifyTrackSelectionChanged(selectedTrackId_);
    } else {
        selectionType_ = SelectionType::MultiTrack;
        notifySelectionTypeChanged(SelectionType::MultiTrack);
    }
}

void SelectionManager::removeTrackFromSelection(TrackId trackId) {
    selectedTrackIds_.erase(trackId);
    if (selectedTrackIds_.empty()) { clearSelection(); return; }
    if (selectedTrackIds_.size() == 1) { selectTrack(*selectedTrackIds_.begin()); return; }
    if (selectedTrackId_ == trackId)
        selectedTrackId_ = *selectedTrackIds_.begin();
    notifySelectionTypeChanged(SelectionType::MultiTrack);
}

bool SelectionManager::isTrackSelected(TrackId trackId) const {
    return selectedTrackIds_.count(trackId) > 0;
}

// Clip Selection

void SelectionManager::selectClip(ClipId clipId) {
    timeRangeSelection_ = TimeRangeSelection{};
    selectionType_ = SelectionType::Clip;
    selectedClipId_ = clipId;
    selectedClipIds_.clear();
    if (clipId != INVALID_CLIP_ID)
        selectedClipIds_.insert(clipId);

    notifySelectionTypeChanged(SelectionType::Clip);
    notifyClipSelectionChanged(clipId);
}

void SelectionManager::selectClips(const std::unordered_set<ClipId>& clipIds) {
    if (clipIds.empty()) { clearSelection(); return; }
    if (clipIds.size() == 1) { selectClip(*clipIds.begin()); return; }

    timeRangeSelection_ = TimeRangeSelection{};
    selectionType_ = SelectionType::MultiClip;
    selectedClipIds_ = clipIds;
    selectedClipId_ = *clipIds.begin();
    notifySelectionTypeChanged(SelectionType::MultiClip);
}

void SelectionManager::addClipToSelection(ClipId clipId) {
    if (selectionType_ == SelectionType::Clip && selectedClipId_ != INVALID_CLIP_ID)
        selectedClipIds_.insert(selectedClipId_);
    selectedClipIds_.insert(clipId);
    selectedClipId_ = clipId;

    if (selectedClipIds_.size() == 1) {
        selectionType_ = SelectionType::Clip;
        notifySelectionTypeChanged(SelectionType::Clip);
    } else {
        selectionType_ = SelectionType::MultiClip;
        notifySelectionTypeChanged(SelectionType::MultiClip);
    }
}

void SelectionManager::removeClipFromSelection(ClipId clipId) {
    selectedClipIds_.erase(clipId);
    if (selectedClipIds_.empty()) { clearSelection(); return; }
    if (selectedClipIds_.size() == 1) { selectClip(*selectedClipIds_.begin()); return; }
    if (selectedClipId_ == clipId)
        selectedClipId_ = *selectedClipIds_.begin();
}

bool SelectionManager::isClipSelected(ClipId clipId) const {
    return selectedClipIds_.count(clipId) > 0;
}

// Time Range Selection

void SelectionManager::selectTimeRange(double startBeats, double endBeats,
                                       const std::vector<TrackId>& trackIds) {
    selectionType_ = SelectionType::TimeRange;
    timeRangeSelection_.startBeats = startBeats;
    timeRangeSelection_.endBeats = endBeats;
    timeRangeSelection_.trackIds = trackIds;
    notifySelectionTypeChanged(SelectionType::TimeRange);
}

// Note Selection

void SelectionManager::selectNote(ClipId clipId, size_t noteIndex) {
    selectionType_ = SelectionType::Note;
    noteSelection_.clipId = clipId;
    noteSelection_.noteIndices = {noteIndex};
    notifySelectionTypeChanged(SelectionType::Note);
}

void SelectionManager::selectNotes(ClipId clipId, const std::vector<size_t>& noteIndices) {
    selectionType_ = SelectionType::Note;
    noteSelection_.clipId = clipId;
    noteSelection_.noteIndices = noteIndices;
    notifySelectionTypeChanged(SelectionType::Note);
}

bool SelectionManager::isNoteSelected(ClipId clipId, size_t noteIndex) const {
    if (selectionType_ != SelectionType::Note || noteSelection_.clipId != clipId)
        return false;
    return std::find(noteSelection_.noteIndices.begin(), noteSelection_.noteIndices.end(),
                     noteIndex) != noteSelection_.noteIndices.end();
}

// Automation Point Selection

void SelectionManager::selectAutomationPoint(AutomationLaneId laneId, AutomationPointId pointId,
                                             AutomationClipId clipId) {
    selectionType_ = SelectionType::AutomationPoint;
    automationPointSelection_.laneId = laneId;
    automationPointSelection_.clipId = clipId;
    automationPointSelection_.pointIds = {pointId};
    notifySelectionTypeChanged(SelectionType::AutomationPoint);
}

void SelectionManager::selectAutomationPoints(AutomationLaneId laneId,
                                              const std::vector<AutomationPointId>& pointIds,
                                              AutomationClipId clipId) {
    selectionType_ = SelectionType::AutomationPoint;
    automationPointSelection_.laneId = laneId;
    automationPointSelection_.clipId = clipId;
    automationPointSelection_.pointIds = pointIds;
    notifySelectionTypeChanged(SelectionType::AutomationPoint);
}

bool SelectionManager::isAutomationPointSelected(AutomationPointId pointId) const {
    if (selectionType_ != SelectionType::AutomationPoint) return false;
    return std::find(automationPointSelection_.pointIds.begin(),
                     automationPointSelection_.pointIds.end(),
                     pointId) != automationPointSelection_.pointIds.end();
}

// Clear

void SelectionManager::clearSelection() {
    selectionType_ = SelectionType::None;
    selectedTrackId_ = INVALID_TRACK_ID;
    selectedTrackIds_.clear();
    selectedClipId_ = INVALID_CLIP_ID;
    selectedClipIds_.clear();
    timeRangeSelection_ = TimeRangeSelection{};
    noteSelection_ = NoteSelection{};
    automationPointSelection_ = AutomationPointSelection{};
    notifySelectionTypeChanged(SelectionType::None);
}

void SelectionManager::clearNoteSelection() {
    if (selectionType_ == SelectionType::Note) {
        ClipId clipId = noteSelection_.clipId;
        noteSelection_ = NoteSelection{};
        if (clipId != INVALID_CLIP_ID)
            selectClip(clipId);
        else
            clearSelection();
    }
}

// Listeners

void SelectionManager::addListener(SelectionManagerListener* listener) {
    if (listener && std::find(listeners_.begin(), listeners_.end(), listener) == listeners_.end())
        listeners_.push_back(listener);
}

void SelectionManager::removeListener(SelectionManagerListener* listener) {
    listeners_.erase(std::remove(listeners_.begin(), listeners_.end(), listener), listeners_.end());
}

void SelectionManager::notifySelectionTypeChanged(SelectionType type) {
    for (auto* l : listeners_)
        l->selectionTypeChanged(type);
}

void SelectionManager::notifyTrackSelectionChanged(TrackId trackId) {
    for (auto* l : listeners_)
        l->trackSelectionChanged(trackId);
}

void SelectionManager::notifyClipSelectionChanged(ClipId clipId) {
    for (auto* l : listeners_)
        l->clipSelectionChanged(clipId);
}

}  // namespace aidaw
