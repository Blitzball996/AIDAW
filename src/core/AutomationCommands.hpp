#pragma once

#include "AutomationInfo.hpp"
#include "AutomationManager.hpp"
#include "UndoManager.hpp"

namespace aidaw {

/**
 * @brief Command for adding an automation point
 */
class AddAutomationPointCommand : public UndoableCommand {
  public:
    AddAutomationPointCommand(AutomationLaneId laneId, AutomationClipId clipId,
                              double beatPosition, double value,
                              AutomationCurveType curveType = AutomationCurveType::Linear)
        : laneId_(laneId), clipId_(clipId), beatPosition_(beatPosition),
          value_(value), curveType_(curveType),
          isClip_(clipId != INVALID_AUTOMATION_CLIP_ID) {}

    void execute() override {
        auto& mgr = AutomationManager::getInstance();
        if (isClip_)
            addedPointId_ = mgr.addPointToClip(clipId_, beatPosition_, value_, curveType_);
        else
            addedPointId_ = mgr.addPoint(laneId_, beatPosition_, value_, curveType_);
    }

    void undo() override {
        auto& mgr = AutomationManager::getInstance();
        if (isClip_)
            mgr.deletePointFromClip(clipId_, addedPointId_);
        else
            mgr.deletePoint(laneId_, addedPointId_);
    }

    juce::String getDescription() const override { return "Add Automation Point"; }
    AutomationPointId getAddedPointId() const { return addedPointId_; }

  private:
    AutomationLaneId laneId_;
    AutomationClipId clipId_;
    double beatPosition_, value_;
    AutomationCurveType curveType_;
    bool isClip_;
    AutomationPointId addedPointId_ = INVALID_AUTOMATION_POINT_ID;
};

/**
 * @brief Command for deleting an automation point
 */
class DeleteAutomationPointCommand : public UndoableCommand {
  public:
    DeleteAutomationPointCommand(AutomationLaneId laneId, AutomationClipId clipId,
                                 AutomationPointId pointId)
        : laneId_(laneId), clipId_(clipId), pointId_(pointId),
          isClip_(clipId != INVALID_AUTOMATION_CLIP_ID) {
        capturePoint();
    }

    void execute() override {
        auto& mgr = AutomationManager::getInstance();
        if (isClip_)
            mgr.deletePointFromClip(clipId_, pointId_);
        else
            mgr.deletePoint(laneId_, pointId_);
    }

    void undo() override {
        auto& mgr = AutomationManager::getInstance();
        if (isClip_)
            mgr.addPointToClip(clipId_, storedPoint_.beatPosition, storedPoint_.value,
                               storedPoint_.curveType);
        else
            mgr.addPoint(laneId_, storedPoint_.beatPosition, storedPoint_.value,
                         storedPoint_.curveType);
    }

    juce::String getDescription() const override { return "Delete Automation Point"; }

  private:
    void capturePoint() {
        auto& mgr = AutomationManager::getInstance();
        if (isClip_) {
            if (auto* clip = mgr.getClip(clipId_)) {
                for (const auto& p : clip->points)
                    if (p.id == pointId_) { storedPoint_ = p; break; }
            }
        } else {
            if (auto* lane = mgr.getLane(laneId_)) {
                for (const auto& p : lane->absolutePoints)
                    if (p.id == pointId_) { storedPoint_ = p; break; }
            }
        }
    }

    AutomationLaneId laneId_;
    AutomationClipId clipId_;
    AutomationPointId pointId_;
    bool isClip_;
    AutomationPoint storedPoint_;
};

/**
 * @brief Command for moving an automation point (supports merging)
 */
class MoveAutomationPointCommand : public UndoableCommand {
  public:
    MoveAutomationPointCommand(AutomationLaneId laneId, AutomationClipId clipId,
                               AutomationPointId pointId, double newBeatPosition, double newValue)
        : laneId_(laneId), clipId_(clipId), pointId_(pointId),
          newBeatPosition_(newBeatPosition), newValue_(newValue),
          isClip_(clipId != INVALID_AUTOMATION_CLIP_ID) {
        captureOldPosition();
    }

    void execute() override {
        auto& mgr = AutomationManager::getInstance();
        if (isClip_)
            mgr.movePointInClip(clipId_, pointId_, newBeatPosition_, newValue_);
        else
            mgr.movePoint(laneId_, pointId_, newBeatPosition_, newValue_);
    }

    void undo() override {
        auto& mgr = AutomationManager::getInstance();
        if (isClip_)
            mgr.movePointInClip(clipId_, pointId_, oldBeatPosition_, oldValue_);
        else
            mgr.movePoint(laneId_, pointId_, oldBeatPosition_, oldValue_);
    }

    juce::String getDescription() const override { return "Move Automation Point"; }

    bool canMergeWith(const UndoableCommand* other) const override {
        if (auto* o = dynamic_cast<const MoveAutomationPointCommand*>(other))
            return o->pointId_ == pointId_ && o->laneId_ == laneId_ && o->clipId_ == clipId_;
        return false;
    }

    void mergeWith(const UndoableCommand* other) override {
        auto* o = static_cast<const MoveAutomationPointCommand*>(other);
        newBeatPosition_ = o->newBeatPosition_;
        newValue_ = o->newValue_;
    }

  private:
    void captureOldPosition() {
        auto& mgr = AutomationManager::getInstance();
        if (isClip_) {
            if (auto* clip = mgr.getClip(clipId_)) {
                for (const auto& p : clip->points)
                    if (p.id == pointId_) {
                        oldBeatPosition_ = p.beatPosition;
                        oldValue_ = p.value;
                        break;
                    }
            }
        } else {
            if (auto* lane = mgr.getLane(laneId_)) {
                for (const auto& p : lane->absolutePoints)
                    if (p.id == pointId_) {
                        oldBeatPosition_ = p.beatPosition;
                        oldValue_ = p.value;
                        break;
                    }
            }
        }
    }

    AutomationLaneId laneId_;
    AutomationClipId clipId_;
    AutomationPointId pointId_;
    double newBeatPosition_, newValue_;
    double oldBeatPosition_ = 0.0, oldValue_ = 0.5;
    bool isClip_;
};

/**
 * @brief Command for deleting an entire automation lane
 */
class DeleteAutomationLaneCommand : public UndoableCommand {
  public:
    explicit DeleteAutomationLaneCommand(AutomationLaneId laneId) : laneId_(laneId) {
        auto& mgr = AutomationManager::getInstance();
        if (auto* lane = mgr.getLane(laneId)) {
            storedLane_ = *lane;
            captured_ = true;
        }
    }

    void execute() override {
        if (captured_)
            AutomationManager::getInstance().deleteLane(laneId_);
    }

    void undo() override {
        if (captured_)
            AutomationManager::getInstance().restoreLane(storedLane_);
    }

    juce::String getDescription() const override { return "Delete Automation Lane"; }

  private:
    AutomationLaneId laneId_;
    AutomationLaneInfo storedLane_;
    bool captured_ = false;
};

}  // namespace aidaw
