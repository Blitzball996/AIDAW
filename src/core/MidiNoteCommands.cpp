#include "MidiNoteCommands.hpp"

namespace aidaw {

// AddMidiNoteCommand

AddMidiNoteCommand::AddMidiNoteCommand(ClipId clipId, double startBeat, int noteNumber,
                                       double lengthBeats, int velocity)
    : clipId_(clipId), startBeat_(startBeat), noteNumber_(noteNumber),
      lengthBeats_(lengthBeats), velocity_(velocity) {}

void AddMidiNoteCommand::execute() {
    // TODO: integrate with ClipManager MIDI note storage
    executed_ = true;
}

void AddMidiNoteCommand::undo() {
    if (!executed_) return;
    // TODO: remove inserted note
}

// MoveMidiNoteCommand

MoveMidiNoteCommand::MoveMidiNoteCommand(ClipId clipId, size_t noteIndex,
                                         double newStartBeat, int newNoteNumber)
    : clipId_(clipId), noteIndex_(noteIndex),
      oldStartBeat_(0.0), newStartBeat_(newStartBeat),
      oldNoteNumber_(0), newNoteNumber_(newNoteNumber) {}

void MoveMidiNoteCommand::execute() {
    // TODO: capture old values, apply new
    executed_ = true;
}

void MoveMidiNoteCommand::undo() {
    if (!executed_) return;
    // TODO: restore old values
}

bool MoveMidiNoteCommand::canMergeWith(const UndoableCommand* other) const {
    if (auto* o = dynamic_cast<const MoveMidiNoteCommand*>(other))
        return o->clipId_ == clipId_ && o->noteIndex_ == noteIndex_;
    return false;
}

void MoveMidiNoteCommand::mergeWith(const UndoableCommand* other) {
    auto* o = static_cast<const MoveMidiNoteCommand*>(other);
    newStartBeat_ = o->newStartBeat_;
    newNoteNumber_ = o->newNoteNumber_;
}

// ResizeMidiNoteCommand

ResizeMidiNoteCommand::ResizeMidiNoteCommand(ClipId clipId, size_t noteIndex,
                                             double newLengthBeats)
    : clipId_(clipId), noteIndex_(noteIndex),
      oldLengthBeats_(0.0), newLengthBeats_(newLengthBeats) {}

void ResizeMidiNoteCommand::execute() {
    executed_ = true;
}

void ResizeMidiNoteCommand::undo() {
    if (!executed_) return;
}

bool ResizeMidiNoteCommand::canMergeWith(const UndoableCommand* other) const {
    if (auto* o = dynamic_cast<const ResizeMidiNoteCommand*>(other))
        return o->clipId_ == clipId_ && o->noteIndex_ == noteIndex_;
    return false;
}

void ResizeMidiNoteCommand::mergeWith(const UndoableCommand* other) {
    newLengthBeats_ = static_cast<const ResizeMidiNoteCommand*>(other)->newLengthBeats_;
}

// DeleteMidiNoteCommand

DeleteMidiNoteCommand::DeleteMidiNoteCommand(ClipId clipId, size_t noteIndex)
    : clipId_(clipId), noteIndex_(noteIndex) {}

void DeleteMidiNoteCommand::execute() {
    executed_ = true;
}

void DeleteMidiNoteCommand::undo() {
    if (!executed_) return;
}

// SetMidiNoteVelocityCommand

SetMidiNoteVelocityCommand::SetMidiNoteVelocityCommand(ClipId clipId, size_t noteIndex,
                                                       int newVelocity)
    : clipId_(clipId), noteIndex_(noteIndex),
      oldVelocity_(0), newVelocity_(newVelocity) {}

void SetMidiNoteVelocityCommand::execute() {
    executed_ = true;
}

void SetMidiNoteVelocityCommand::undo() {
    if (!executed_) return;
}

bool SetMidiNoteVelocityCommand::canMergeWith(const UndoableCommand* other) const {
    if (auto* o = dynamic_cast<const SetMidiNoteVelocityCommand*>(other))
        return o->clipId_ == clipId_ && o->noteIndex_ == noteIndex_;
    return false;
}

void SetMidiNoteVelocityCommand::mergeWith(const UndoableCommand* other) {
    newVelocity_ = static_cast<const SetMidiNoteVelocityCommand*>(other)->newVelocity_;
}

// MoveMultipleMidiNotesCommand

MoveMultipleMidiNotesCommand::MoveMultipleMidiNotesCommand(ClipId clipId,
                                                           std::vector<NoteMove> moves)
    : clipId_(clipId), moves_(std::move(moves)) {}

void MoveMultipleMidiNotesCommand::execute() {
    executed_ = true;
}

void MoveMultipleMidiNotesCommand::undo() {
    if (!executed_) return;
}

// DeleteMultipleMidiNotesCommand

DeleteMultipleMidiNotesCommand::DeleteMultipleMidiNotesCommand(ClipId clipId,
                                                               std::vector<size_t> noteIndices)
    : clipId_(clipId), noteIndices_(std::move(noteIndices)) {}

void DeleteMultipleMidiNotesCommand::execute() {
    executed_ = true;
}

void DeleteMultipleMidiNotesCommand::undo() {
    if (!executed_) return;
}

// QuantizeMidiNotesCommand

QuantizeMidiNotesCommand::QuantizeMidiNotesCommand(ClipId clipId,
                                                   std::vector<size_t> noteIndices,
                                                   double gridResolution, QuantizeMode mode)
    : clipId_(clipId), noteIndices_(std::move(noteIndices)),
      gridResolution_(gridResolution), mode_(mode) {}

void QuantizeMidiNotesCommand::execute() {
    executed_ = true;
}

void QuantizeMidiNotesCommand::undo() {
    if (!executed_) return;
}

}  // namespace aidaw
