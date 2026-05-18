#include "ClipCommands.hpp"
#include "ClipManager.hpp"

namespace aidaw {

// MoveClipCommand

MoveClipCommand::MoveClipCommand(ClipId clipId, BeatPosition newStartBeat)
    : clipId_(clipId), newStartBeat_(newStartBeat.value) {}

void MoveClipCommand::execute() {
    // Store old position for undo (implementation depends on ClipManager API)
    executed_ = true;
}

void MoveClipCommand::undo() {
    if (!executed_) return;
    // Restore old position
}

bool MoveClipCommand::canMergeWith(const UndoableCommand* other) const {
    if (auto* o = dynamic_cast<const MoveClipCommand*>(other))
        return o->clipId_ == clipId_;
    return false;
}

void MoveClipCommand::mergeWith(const UndoableCommand* other) {
    auto* o = static_cast<const MoveClipCommand*>(other);
    newStartBeat_ = o->newStartBeat_;
}

// DeleteClipCommand

DeleteClipCommand::DeleteClipCommand(ClipId clipId) : clipId_(clipId) {}

void DeleteClipCommand::execute() {
    executed_ = true;
}

void DeleteClipCommand::undo() {
    if (!executed_) return;
}

// CreateClipCommand

CreateClipCommand::CreateClipCommand(TrackId trackId, BeatPosition startBeat,
                                     BeatDuration lengthBeats)
    : trackId_(trackId), startBeat_(startBeat.value), lengthBeats_(lengthBeats.value) {}

bool CreateClipCommand::canExecute() const {
    return trackId_ != INVALID_TRACK_ID && lengthBeats_ > 0.0;
}

void CreateClipCommand::execute() {
    if (!canExecute()) return;
    executed_ = true;
}

void CreateClipCommand::undo() {
    if (!executed_) return;
    if (createdClipId_ != INVALID_CLIP_ID) {
        createdClipId_ = INVALID_CLIP_ID;
    }
}

// DuplicateClipCommand

DuplicateClipCommand::DuplicateClipCommand(ClipId sourceClipId)
    : sourceClipId_(sourceClipId) {}

bool DuplicateClipCommand::canExecute() const {
    return sourceClipId_ != INVALID_CLIP_ID;
}

void DuplicateClipCommand::execute() {
    if (!canExecute()) return;
    executed_ = true;
}

void DuplicateClipCommand::undo() {
    if (!executed_) return;
    if (duplicatedClipId_ != INVALID_CLIP_ID) {
        duplicatedClipId_ = INVALID_CLIP_ID;
    }
}

}  // namespace aidaw
