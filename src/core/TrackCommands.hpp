#pragma once

#include <juce_core/juce_core.h>

#include "TypeIds.hpp"
#include "UndoManager.hpp"

namespace aidaw {

/**
 * @brief Command for creating a new track
 */
class CreateTrackCommand : public UndoableCommand {
  public:
    explicit CreateTrackCommand(const juce::String& name = juce::String(),
                                TrackId afterTrackId = INVALID_TRACK_ID);

    void execute() override;
    void undo() override;
    juce::String getDescription() const override { return "Create Track"; }

    TrackId getCreatedTrackId() const { return createdTrackId_; }

  private:
    juce::String name_;
    TrackId afterTrackId_ = INVALID_TRACK_ID;
    TrackId createdTrackId_ = INVALID_TRACK_ID;
    bool executed_ = false;
};

/**
 * @brief Command for deleting a track
 */
class DeleteTrackCommand : public UndoableCommand {
  public:
    explicit DeleteTrackCommand(TrackId trackId);

    void execute() override;
    void undo() override;
    juce::String getDescription() const override { return "Delete Track"; }

  private:
    TrackId trackId_;
    bool executed_ = false;
};

/**
 * @brief Command for duplicating a track
 */
class DuplicateTrackCommand : public UndoableCommand {
  public:
    explicit DuplicateTrackCommand(TrackId sourceTrackId);

    void execute() override;
    void undo() override;
    juce::String getDescription() const override { return "Duplicate Track"; }

    TrackId getDuplicatedTrackId() const { return duplicatedTrackId_; }

  private:
    TrackId sourceTrackId_;
    TrackId duplicatedTrackId_ = INVALID_TRACK_ID;
    bool executed_ = false;
};

}  // namespace aidaw
