#pragma once

#include <juce_core/juce_core.h>

#include <vector>

#include "MidiTypes.hpp"
#include "TypeIds.hpp"
#include "UndoManager.hpp"

namespace aidaw {

/**
 * @brief Mode for quantizing MIDI notes
 */
enum class QuantizeMode { StartOnly, LengthOnly, StartAndLength };

/**
 * @brief Command for adding a MIDI note to a clip
 */
class AddMidiNoteCommand : public UndoableCommand {
  public:
    AddMidiNoteCommand(ClipId clipId, double startBeat, int noteNumber,
                       double lengthBeats, int velocity);

    void execute() override;
    void undo() override;
    juce::String getDescription() const override { return "Add MIDI Note"; }

  private:
    ClipId clipId_;
    double startBeat_;
    int noteNumber_;
    double lengthBeats_;
    int velocity_;
    size_t insertedIndex_ = 0;
    bool executed_ = false;
};

/**
 * @brief Command for moving a MIDI note
 */
class MoveMidiNoteCommand : public UndoableCommand {
  public:
    MoveMidiNoteCommand(ClipId clipId, size_t noteIndex, double newStartBeat, int newNoteNumber);

    void execute() override;
    void undo() override;
    juce::String getDescription() const override { return "Move MIDI Note"; }

    bool canMergeWith(const UndoableCommand* other) const override;
    void mergeWith(const UndoableCommand* other) override;

  private:
    ClipId clipId_;
    size_t noteIndex_;
    double oldStartBeat_, newStartBeat_;
    int oldNoteNumber_, newNoteNumber_;
    bool executed_ = false;
};

/**
 * @brief Command for resizing a MIDI note
 */
class ResizeMidiNoteCommand : public UndoableCommand {
  public:
    ResizeMidiNoteCommand(ClipId clipId, size_t noteIndex, double newLengthBeats);

    void execute() override;
    void undo() override;
    juce::String getDescription() const override { return "Resize MIDI Note"; }

    bool canMergeWith(const UndoableCommand* other) const override;
    void mergeWith(const UndoableCommand* other) override;

  private:
    ClipId clipId_;
    size_t noteIndex_;
    double oldLengthBeats_, newLengthBeats_;
    bool executed_ = false;
};

/**
 * @brief Command for deleting a MIDI note
 */
class DeleteMidiNoteCommand : public UndoableCommand {
  public:
    DeleteMidiNoteCommand(ClipId clipId, size_t noteIndex);

    void execute() override;
    void undo() override;
    juce::String getDescription() const override { return "Delete MIDI Note"; }

  private:
    ClipId clipId_;
    size_t noteIndex_;
    bool executed_ = false;
};

/**
 * @brief Command for setting velocity of a MIDI note
 */
class SetMidiNoteVelocityCommand : public UndoableCommand {
  public:
    SetMidiNoteVelocityCommand(ClipId clipId, size_t noteIndex, int newVelocity);

    void execute() override;
    void undo() override;
    juce::String getDescription() const override { return "Set Note Velocity"; }

    bool canMergeWith(const UndoableCommand* other) const override;
    void mergeWith(const UndoableCommand* other) override;

  private:
    ClipId clipId_;
    size_t noteIndex_;
    int oldVelocity_, newVelocity_;
    bool executed_ = false;
};

/**
 * @brief Command for moving multiple MIDI notes at once
 */
class MoveMultipleMidiNotesCommand : public UndoableCommand {
  public:
    struct NoteMove {
        size_t noteIndex;
        double newStartBeat;
        int newNoteNumber;
    };

    MoveMultipleMidiNotesCommand(ClipId clipId, std::vector<NoteMove> moves);

    void execute() override;
    void undo() override;
    juce::String getDescription() const override { return "Move MIDI Notes"; }

  private:
    ClipId clipId_;
    std::vector<NoteMove> moves_;
    struct OldValues {
        size_t noteIndex;
        double startBeat;
        int noteNumber;
    };
    std::vector<OldValues> oldValues_;
    bool executed_ = false;
};

/**
 * @brief Command for deleting multiple MIDI notes at once
 */
class DeleteMultipleMidiNotesCommand : public UndoableCommand {
  public:
    DeleteMultipleMidiNotesCommand(ClipId clipId, std::vector<size_t> noteIndices);

    void execute() override;
    void undo() override;
    juce::String getDescription() const override { return "Delete MIDI Notes"; }

  private:
    ClipId clipId_;
    std::vector<size_t> noteIndices_;
    bool executed_ = false;
};

/**
 * @brief Command for quantizing multiple MIDI notes to grid
 */
class QuantizeMidiNotesCommand : public UndoableCommand {
  public:
    QuantizeMidiNotesCommand(ClipId clipId, std::vector<size_t> noteIndices,
                             double gridResolution, QuantizeMode mode);

    void execute() override;
    void undo() override;
    juce::String getDescription() const override { return "Quantize MIDI Notes"; }

  private:
    ClipId clipId_;
    std::vector<size_t> noteIndices_;
    double gridResolution_;
    QuantizeMode mode_;
    bool executed_ = false;
};

}  // namespace aidaw
