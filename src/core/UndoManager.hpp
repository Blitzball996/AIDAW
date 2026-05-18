#pragma once

#include <juce_core/juce_core.h>

#include <deque>
#include <memory>
#include <vector>

namespace aidaw {

/**
 * @brief Base class for all undoable commands
 */
class UndoableCommand {
  public:
    virtual ~UndoableCommand() = default;

    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual juce::String getDescription() const = 0;

    virtual bool canMergeWith(const UndoableCommand* /*other*/) const {
        return false;
    }

    virtual void mergeWith(const UndoableCommand* /*other*/) {}
};

/**
 * @brief Listener interface for undo state changes
 */
class UndoManagerListener {
  public:
    virtual ~UndoManagerListener() = default;
    virtual void undoStateChanged() = 0;
};

/**
 * @brief Central manager for undo/redo operations
 */
class UndoManager {
  public:
    static UndoManager& getInstance();

    UndoManager(const UndoManager&) = delete;
    UndoManager& operator=(const UndoManager&) = delete;

    void executeCommand(std::unique_ptr<UndoableCommand> command);
    bool undo();
    bool redo();

    bool canUndo() const { return !undoStack_.empty(); }
    bool canRedo() const { return !redoStack_.empty(); }

    juce::String getUndoDescription() const;
    juce::String getRedoDescription() const;

    void clearHistory();

    void beginCompoundOperation(const juce::String& description);
    void endCompoundOperation();

    bool isInCompoundOperation() const { return compoundDepth_ > 0; }

    void setMaxUndoSteps(size_t maxSteps) {
        maxUndoSteps_ = maxSteps;
        trimUndoStack();
    }

    size_t getMaxUndoSteps() const { return maxUndoSteps_; }

    void addListener(UndoManagerListener* listener);
    void removeListener(UndoManagerListener* listener);

  private:
    UndoManager();
    ~UndoManager() = default;

    void notifyListeners();
    void trimUndoStack();

    std::deque<std::unique_ptr<UndoableCommand>> undoStack_;
    std::deque<std::unique_ptr<UndoableCommand>> redoStack_;

    int compoundDepth_ = 0;
    juce::String compoundDescription_;
    std::vector<std::unique_ptr<UndoableCommand>> compoundCommands_;

    size_t maxUndoSteps_ = 100;

    std::vector<UndoManagerListener*> listeners_;
};

/**
 * @brief Compound command that groups multiple commands as one undo step
 */
class CompoundCommand : public UndoableCommand {
  public:
    explicit CompoundCommand(const juce::String& description,
                             std::vector<std::unique_ptr<UndoableCommand>> commands);

    void execute() override;
    void undo() override;
    juce::String getDescription() const override { return description_; }

  private:
    juce::String description_;
    std::vector<std::unique_ptr<UndoableCommand>> commands_;
};

/**
 * @brief RAII helper for compound operations
 */
class CompoundOperationScope {
  public:
    explicit CompoundOperationScope(const juce::String& description);
    ~CompoundOperationScope();

    CompoundOperationScope(const CompoundOperationScope&) = delete;
    CompoundOperationScope& operator=(const CompoundOperationScope&) = delete;
};

}  // namespace aidaw
