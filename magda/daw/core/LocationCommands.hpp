#pragma once

#include <juce_core/juce_core.h>

#include "CommandPattern.hpp"
#include "LocationManager.hpp"
#include "LocationTypes.hpp"

namespace magda {

/**
 * @brief Command to add a point marker
 */
class AddMarkerCommand : public ValidatedCommand {
  public:
    AddMarkerCommand(const std::string& name, double time,
                     juce::Colour colour = juce::Colours::yellow);

    juce::String getDescription() const override { return "Add Marker"; }

    bool canExecute() const override;
    void execute() override;
    void undo() override;

  private:
    std::string name_;
    double time_;
    juce::Colour colour_;
    LocationId createdId_ = INVALID_LOCATION_ID;
};

/**
 * @brief Command to add a range marker
 */
class AddRangeCommand : public ValidatedCommand {
  public:
    AddRangeCommand(const std::string& name, double startTime, double endTime,
                    juce::Colour colour = juce::Colours::cyan);

    juce::String getDescription() const override { return "Add Range"; }

    bool canExecute() const override;
    void execute() override;
    void undo() override;

  private:
    std::string name_;
    double startTime_;
    double endTime_;
    juce::Colour colour_;
    LocationId createdId_ = INVALID_LOCATION_ID;
};

/**
 * @brief Command to delete a location marker or range
 */
class DeleteLocationCommand : public ValidatedCommand {
  public:
    explicit DeleteLocationCommand(LocationId id);

    juce::String getDescription() const override { return "Delete Marker"; }

    bool canExecute() const override;
    void execute() override;
    void undo() override;

  private:
    LocationId id_;
    LocationInfo savedInfo_;
};

/**
 * @brief Command to move a marker to a new time position
 */
class MoveMarkerCommand : public ValidatedCommand {
  public:
    MoveMarkerCommand(LocationId id, double newTime);

    juce::String getDescription() const override { return "Move Marker"; }

    bool canExecute() const override;
    void execute() override;
    void undo() override;

    bool canMergeWith(const UndoableCommand* other) const override;
    void mergeWith(const UndoableCommand* other) override;

  private:
    LocationId id_;
    double newTime_;
    double oldTime_ = 0.0;
};

/**
 * @brief Command to move a range marker's start and end times
 */
class MoveRangeCommand : public ValidatedCommand {
  public:
    MoveRangeCommand(LocationId id, double newStartTime, double newEndTime);

    juce::String getDescription() const override { return "Move Range"; }

    bool canExecute() const override;
    void execute() override;
    void undo() override;

    bool canMergeWith(const UndoableCommand* other) const override;
    void mergeWith(const UndoableCommand* other) override;

  private:
    LocationId id_;
    double newStartTime_;
    double newEndTime_;
    double oldStartTime_ = 0.0;
    double oldEndTime_ = 0.0;
};

/**
 * @brief Command to rename a location
 */
class RenameLocationCommand : public ValidatedCommand {
  public:
    RenameLocationCommand(LocationId id, const std::string& newName);

    juce::String getDescription() const override { return "Rename Marker"; }

    bool canExecute() const override;
    void execute() override;
    void undo() override;

  private:
    LocationId id_;
    std::string newName_;
    std::string oldName_;
};

}  // namespace magda
