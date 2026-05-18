#include "TrackCommands.hpp"
#include "TrackManager.hpp"

namespace aidaw {

// CreateTrackCommand

CreateTrackCommand::CreateTrackCommand(const juce::String& name, TrackId afterTrackId)
    : name_(name), afterTrackId_(afterTrackId) {}

void CreateTrackCommand::execute() {
    // TODO: integrate with TrackManager once it supports insert-after
    executed_ = true;
}

void CreateTrackCommand::undo() {
    if (!executed_) return;
    if (createdTrackId_ != INVALID_TRACK_ID) {
        // TODO: TrackManager::deleteTrack
    }
}

// DeleteTrackCommand

DeleteTrackCommand::DeleteTrackCommand(TrackId trackId) : trackId_(trackId) {}

void DeleteTrackCommand::execute() {
    // TODO: capture track state, then delete
    executed_ = true;
}

void DeleteTrackCommand::undo() {
    if (!executed_) return;
    // TODO: restore track from captured state
}

// DuplicateTrackCommand

DuplicateTrackCommand::DuplicateTrackCommand(TrackId sourceTrackId)
    : sourceTrackId_(sourceTrackId) {}

void DuplicateTrackCommand::execute() {
    // TODO: duplicate track via TrackManager
    executed_ = true;
}

void DuplicateTrackCommand::undo() {
    if (!executed_) return;
    if (duplicatedTrackId_ != INVALID_TRACK_ID) {
        // TODO: delete duplicated track
    }
}

}  // namespace aidaw
