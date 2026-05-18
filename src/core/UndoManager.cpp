#include "UndoManager.hpp"

namespace aidaw {

UndoManager& UndoManager::getInstance() {
    static UndoManager instance;
    return instance;
}

UndoManager::UndoManager() = default;

void UndoManager::executeCommand(std::unique_ptr<UndoableCommand> command) {
    if (!command)
        return;

    command->execute();

    if (compoundDepth_ > 0) {
        compoundCommands_.push_back(std::move(command));
        return;
    }

    if (!undoStack_.empty() && undoStack_.back()->canMergeWith(command.get())) {
        undoStack_.back()->mergeWith(command.get());
    } else {
        undoStack_.push_back(std::move(command));
        trimUndoStack();
    }

    redoStack_.clear();
    notifyListeners();
}

bool UndoManager::undo() {
    if (undoStack_.empty())
        return false;

    auto command = std::move(undoStack_.back());
    undoStack_.pop_back();

    command->undo();

    redoStack_.push_back(std::move(command));
    notifyListeners();
    return true;
}

bool UndoManager::redo() {
    if (redoStack_.empty())
        return false;

    auto command = std::move(redoStack_.back());
    redoStack_.pop_back();

    command->execute();

    undoStack_.push_back(std::move(command));
    notifyListeners();
    return true;
}

juce::String UndoManager::getUndoDescription() const {
    if (undoStack_.empty())
        return {};
    return undoStack_.back()->getDescription();
}

juce::String UndoManager::getRedoDescription() const {
    if (redoStack_.empty())
        return {};
    return redoStack_.back()->getDescription();
}

void UndoManager::clearHistory() {
    undoStack_.clear();
    redoStack_.clear();
    compoundCommands_.clear();
    compoundDepth_ = 0;
    notifyListeners();
}

void UndoManager::beginCompoundOperation(const juce::String& description) {
    if (compoundDepth_ == 0) {
        compoundDescription_ = description;
        compoundCommands_.clear();
    }
    compoundDepth_++;
}

void UndoManager::endCompoundOperation() {
    if (compoundDepth_ <= 0)
        return;

    compoundDepth_--;

    if (compoundDepth_ == 0 && !compoundCommands_.empty()) {
        auto compound =
            std::make_unique<CompoundCommand>(compoundDescription_, std::move(compoundCommands_));
        undoStack_.push_back(std::move(compound));
        trimUndoStack();

        redoStack_.clear();
        compoundCommands_.clear();
        notifyListeners();
    }
}

void UndoManager::addListener(UndoManagerListener* listener) {
    if (listener && std::find(listeners_.begin(), listeners_.end(), listener) == listeners_.end()) {
        listeners_.push_back(listener);
    }
}

void UndoManager::removeListener(UndoManagerListener* listener) {
    listeners_.erase(std::remove(listeners_.begin(), listeners_.end(), listener), listeners_.end());
}

void UndoManager::notifyListeners() {
    for (auto* listener : listeners_) {
        listener->undoStateChanged();
    }
}

void UndoManager::trimUndoStack() {
    while (undoStack_.size() > maxUndoSteps_) {
        undoStack_.pop_front();
    }
}

// CompoundCommand

CompoundCommand::CompoundCommand(const juce::String& description,
                                 std::vector<std::unique_ptr<UndoableCommand>> commands)
    : description_(description), commands_(std::move(commands)) {}

void CompoundCommand::execute() {
    for (auto& cmd : commands_) {
        cmd->execute();
    }
}

void CompoundCommand::undo() {
    for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) {
        (*it)->undo();
    }
}

// CompoundOperationScope

CompoundOperationScope::CompoundOperationScope(const juce::String& description) {
    UndoManager::getInstance().beginCompoundOperation(description);
}

CompoundOperationScope::~CompoundOperationScope() {
    UndoManager::getInstance().endCompoundOperation();
}

}  // namespace aidaw
