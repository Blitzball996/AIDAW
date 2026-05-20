#include "LocationCommands.hpp"

namespace magda {

// --- AddMarkerCommand ---

AddMarkerCommand::AddMarkerCommand(const std::string& name, double time,
                                   juce::Colour colour)
    : name_(name), time_(time), colour_(colour) {}

bool AddMarkerCommand::canExecute() const {
    return time_ >= 0.0;
}

void AddMarkerCommand::execute() {
    if (!canExecute())
        return;

    auto& lm = LocationManager::getInstance();
    createdId_ = lm.addMarker(name_, time_, colour_);
    executed_ = true;
}

void AddMarkerCommand::undo() {
    if (!executed_)
        return;

    auto& lm = LocationManager::getInstance();
    lm.removeLocation(createdId_);
}

// --- AddRangeCommand ---

AddRangeCommand::AddRangeCommand(const std::string& name, double startTime,
                                 double endTime, juce::Colour colour)
    : name_(name), startTime_(startTime), endTime_(endTime), colour_(colour) {}

bool AddRangeCommand::canExecute() const {
    return startTime_ >= 0.0 && endTime_ > startTime_;
}

void AddRangeCommand::execute() {
    if (!canExecute())
        return;

    auto& lm = LocationManager::getInstance();
    createdId_ = lm.addRange(name_, startTime_, endTime_, colour_);
    executed_ = true;
}

void AddRangeCommand::undo() {
    if (!executed_)
        return;

    auto& lm = LocationManager::getInstance();
    lm.removeLocation(createdId_);
}

// --- DeleteLocationCommand ---

DeleteLocationCommand::DeleteLocationCommand(LocationId id) : id_(id) {}

bool DeleteLocationCommand::canExecute() const {
    auto& lm = LocationManager::getInstance();
    const auto* loc = lm.getLocation(id_);
    return loc != nullptr && !loc->locked;
}

void DeleteLocationCommand::execute() {
    if (!canExecute())
        return;

    auto& lm = LocationManager::getInstance();
    const auto* loc = lm.getLocation(id_);
    if (!loc)
        return;

    savedInfo_ = *loc;
    lm.removeLocation(id_);
    executed_ = true;
}

void DeleteLocationCommand::undo() {
    if (!executed_)
        return;

    auto& lm = LocationManager::getInstance();
    lm.restoreLocation(savedInfo_);
}

// --- MoveMarkerCommand ---

MoveMarkerCommand::MoveMarkerCommand(LocationId id, double newTime)
    : id_(id), newTime_(newTime) {}

bool MoveMarkerCommand::canExecute() const {
    auto& lm = LocationManager::getInstance();
    const auto* loc = lm.getLocation(id_);
    return loc != nullptr && !loc->locked && newTime_ >= 0.0;
}

void MoveMarkerCommand::execute() {
    if (!canExecute())
        return;

    auto& lm = LocationManager::getInstance();
    const auto* loc = lm.getLocation(id_);
    if (!loc)
        return;

    oldTime_ = loc->startTime;
    lm.setLocationTime(id_, newTime_);
    executed_ = true;
}

void MoveMarkerCommand::undo() {
    if (!executed_)
        return;

    auto& lm = LocationManager::getInstance();
    lm.setLocationTime(id_, oldTime_);
}

bool MoveMarkerCommand::canMergeWith(const UndoableCommand* other) const {
    auto* otherMove = dynamic_cast<const MoveMarkerCommand*>(other);
    return otherMove != nullptr && otherMove->id_ == id_;
}

void MoveMarkerCommand::mergeWith(const UndoableCommand* other) {
    auto* otherMove = dynamic_cast<const MoveMarkerCommand*>(other);
    if (otherMove)
        newTime_ = otherMove->newTime_;
}

// --- MoveRangeCommand ---

MoveRangeCommand::MoveRangeCommand(LocationId id, double newStartTime,
                                   double newEndTime)
    : id_(id), newStartTime_(newStartTime), newEndTime_(newEndTime) {}

bool MoveRangeCommand::canExecute() const {
    auto& lm = LocationManager::getInstance();
    const auto* loc = lm.getLocation(id_);
    return loc != nullptr && !loc->locked && loc->isRange() &&
           newStartTime_ >= 0.0 && newEndTime_ > newStartTime_;
}

void MoveRangeCommand::execute() {
    if (!canExecute())
        return;

    auto& lm = LocationManager::getInstance();
    const auto* loc = lm.getLocation(id_);
    if (!loc)
        return;

    oldStartTime_ = loc->startTime;
    oldEndTime_ = loc->endTime;
    lm.setLocationRange(id_, newStartTime_, newEndTime_);
    executed_ = true;
}

void MoveRangeCommand::undo() {
    if (!executed_)
        return;

    auto& lm = LocationManager::getInstance();
    lm.setLocationRange(id_, oldStartTime_, oldEndTime_);
}

bool MoveRangeCommand::canMergeWith(const UndoableCommand* other) const {
    auto* otherMove = dynamic_cast<const MoveRangeCommand*>(other);
    return otherMove != nullptr && otherMove->id_ == id_;
}

void MoveRangeCommand::mergeWith(const UndoableCommand* other) {
    auto* otherMove = dynamic_cast<const MoveRangeCommand*>(other);
    if (otherMove) {
        newStartTime_ = otherMove->newStartTime_;
        newEndTime_ = otherMove->newEndTime_;
    }
}

// --- RenameLocationCommand ---

RenameLocationCommand::RenameLocationCommand(LocationId id,
                                             const std::string& newName)
    : id_(id), newName_(newName) {}

bool RenameLocationCommand::canExecute() const {
    auto& lm = LocationManager::getInstance();
    const auto* loc = lm.getLocation(id_);
    return loc != nullptr && !newName_.empty();
}

void RenameLocationCommand::execute() {
    if (!canExecute())
        return;

    auto& lm = LocationManager::getInstance();
    const auto* loc = lm.getLocation(id_);
    if (!loc)
        return;

    oldName_ = loc->name;
    lm.setLocationName(id_, newName_);
    executed_ = true;
}

void RenameLocationCommand::undo() {
    if (!executed_)
        return;

    auto& lm = LocationManager::getInstance();
    lm.setLocationName(id_, oldName_);
}

}  // namespace magda
