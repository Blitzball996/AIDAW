#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "UndoManager.hpp"

namespace aidaw {

/**
 * @brief Enhanced base for commands with state validation
 */
class ValidatedCommand : public UndoableCommand {
  public:
    virtual bool canExecute() const { return true; }
    virtual bool validateState() const { return true; }

    bool wasExecuted() const { return executed_; }

  protected:
    bool executed_ = false;
};

/**
 * @brief Command that stores complete state snapshots for reliable undo
 */
template <typename StateT>
class SnapshotCommand : public ValidatedCommand {
  public:
    void execute() override {
        if (!canExecute())
            return;

        beforeState_ = captureState();
        performAction();
        afterState_ = captureState();

        if (!validateState()) {
            restoreState(beforeState_);
            executed_ = false;
            return;
        }

        executed_ = true;
    }

    void undo() override {
        if (!executed_)
            return;

        restoreState(beforeState_);
    }

  protected:
    virtual StateT captureState() = 0;
    virtual void restoreState(const StateT& state) = 0;
    virtual void performAction() = 0;

  private:
    StateT beforeState_;
    StateT afterState_;
};

}  // namespace aidaw
