#pragma once

#include <juce_core/juce_core.h>

#include <memory>
#include <vector>

#include "CommandPattern.hpp"
#include "TypeIds.hpp"

namespace aidaw {

// Forward declarations
class TracktionEngineWrapper;

struct BeatPosition { double value = 0.0; };
struct BeatDuration { double value = 0.0; };

/**
 * @brief Command for moving a clip to a new beat position
 */
class MoveClipCommand : public ValidatedCommand {
  public:
    MoveClipCommand(ClipId clipId, BeatPosition newStartBeat);

    juce::String getDescription() const override { return "Move Clip"; }
    void execute() override;
    void undo() override;

    bool canMergeWith(const UndoableCommand* other) const override;
    void mergeWith(const UndoableCommand* other) override;

  private:
    ClipId clipId_;
    double newStartBeat_;
    double oldStartBeat_ = 0.0;
};

/**
 * @brief Command for deleting a clip
 */
class DeleteClipCommand : public ValidatedCommand {
  public:
    explicit DeleteClipCommand(ClipId clipId);

    juce::String getDescription() const override { return "Delete Clip"; }
    void execute() override;
    void undo() override;

  private:
    ClipId clipId_;
    // Clip state stored for undo (implementation uses ClipManager)
};

/**
 * @brief Command for creating a new clip
 */
class CreateClipCommand : public ValidatedCommand {
  public:
    CreateClipCommand(TrackId trackId, BeatPosition startBeat, BeatDuration lengthBeats);

    juce::String getDescription() const override { return "Create Clip"; }
    bool canExecute() const override;
    void execute() override;
    void undo() override;

    ClipId getCreatedClipId() const { return createdClipId_; }

  private:
    TrackId trackId_;
    double startBeat_;
    double lengthBeats_;
    ClipId createdClipId_ = INVALID_CLIP_ID;
};

/**
 * @brief Command for duplicating a clip
 */
class DuplicateClipCommand : public ValidatedCommand {
  public:
    explicit DuplicateClipCommand(ClipId sourceClipId);

    juce::String getDescription() const override { return "Duplicate Clip"; }
    bool canExecute() const override;
    void execute() override;
    void undo() override;

    ClipId getDuplicatedClipId() const { return duplicatedClipId_; }

  private:
    ClipId sourceClipId_;
    ClipId duplicatedClipId_ = INVALID_CLIP_ID;
};

#ifdef AIDAW_HAS_TRACKTION
/**
 * @brief Command for rendering a clip to audio (requires Tracktion engine)
 */
class RenderClipCommand : public UndoableCommand {
  public:
    RenderClipCommand(ClipId clipId, TracktionEngineWrapper* engine);

    juce::String getDescription() const override { return "Render Clip"; }
    void execute() override;
    void undo() override;

    bool wasSuccessful() const { return success_; }

  private:
    ClipId clipId_;
    TracktionEngineWrapper* engine_;
    bool success_ = false;
};
#endif

}  // namespace aidaw
