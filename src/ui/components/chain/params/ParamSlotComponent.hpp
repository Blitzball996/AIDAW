#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "core/LinkModeManager.hpp"
#include "core/MacroInfo.hpp"
#include "core/ModInfo.hpp"
#include "core/ParameterInfo.hpp"
#include "core/SelectionManager.hpp"
#include "core/TypeIds.hpp"
#include "core/controllers/ControllerRegistry.hpp"
#include "core/controllers/MidiLearnCoordinator.hpp"
#include "params/ParamLinkResolver.hpp"
#include "ui/components/common/TextSlider.hpp"

namespace aidaw::daw::ui {

/**
 * @brief A parameter slot with modulation indicator and linking support
 *
 * Displays a parameter name and value, with visual indicators for any
 * mods/macros linked to this parameter.
 *
 * Supports drag-and-drop: drop a ModKnobComponent here to create a link.
 * Supports link mode: when a mod/macro is in link mode, clicking this param creates a link.
 */
class ParamSlotComponent : public juce::Component,
                           public juce::DragAndDropTarget,
                           public aidaw::LinkModeManagerListener,
                           public aidaw::MidiLearnCoordinatorListener,
                           public aidaw::BindingRegistryListener,
                           public aidaw::ControllerRegistryListener,
                           private juce::Timer {
  public:
    ParamSlotComponent(int paramIndex);
    ~ParamSlotComponent() override;

    void setParamName(const juce::String& name);
    void setParamValue(double value);
    void setParameterInfo(const aidaw::ParameterInfo& info);  // Set full param info for formatting
    void cancelGesture();
    void setShowEmptyText(bool show);  // Show "-" instead of value for empty slots
    void setFonts(const juce::Font& labelFont, const juce::Font& valueFont);
    bool isBeingDragged() const;  // Check if user is actively dragging this parameter
    void setOverlayOnly(bool overlayOnly);
    void setLinkOverlayVertical(bool vertical) {
        linkOverlayVertical_ = vertical;
    }
    void refreshLinkModeState();

    // Set the actual parameter index (mapped from visibility filter)
    void setParamIndex(int paramIndex) {
        paramIndex_ = paramIndex;
        refreshAutomationTarget();
        refreshMidiBindingState();
    }
    int getParamIndex() const {
        return paramIndex_;
    }

    // Set the device this param belongs to (for mod/macro lookups)
    void setDeviceId(aidaw::DeviceId deviceId) {
        deviceId_ = deviceId;
    }

    // Set the device path (for param selection)
    void setDevicePath(const aidaw::ChainNodePath& path) {
        devicePath_ = path;
        refreshAutomationTarget();
        refreshMidiBindingState();
    }

  private:
    // Wire the underlying TextSlider's automation target so the slot paints
    // the purple "automated" tint when a lane exists for this (device,
    // param), and drag gestures trigger the touch/override bookkeeping.
    // Idempotent — safe to call whenever the device path or param index
    // changes.
    void refreshAutomationTarget();

  public:
    // Set available mods and macros for linking
    void setAvailableMods(const aidaw::ModArray* mods) {
        availableMods_ = mods;
    }
    void setAvailableMacros(const aidaw::MacroArray* macros) {
        availableMacros_ = macros;
    }
    void setAvailableRackMacros(const aidaw::MacroArray* rackMacros) {
        availableRackMacros_ = rackMacros;
    }
    void setAvailableRackMods(const aidaw::ModArray* rackMods) {
        availableRackMods_ = rackMods;
    }
    void setAvailableTrackMods(const aidaw::ModArray* trackMods) {
        availableTrackMods_ = trackMods;
    }
    void setAvailableTrackMacros(const aidaw::MacroArray* trackMacros) {
        availableTrackMacros_ = trackMacros;
    }

    // Contextual selection - when set, only shows this mod's/macro's link
    void setSelectedModIndex(int modIndex) {
        selectedModIndex_ = modIndex;
        repaint();
    }
    void clearSelectedMod() {
        selectedModIndex_ = -1;
        repaint();
    }
    int getSelectedModIndex() const {
        return selectedModIndex_;
    }

    void setSelectedMacroIndex(int macroIndex) {
        selectedMacroIndex_ = macroIndex;
        repaint();
    }
    void clearSelectedMacro() {
        selectedMacroIndex_ = -1;
        repaint();
    }
    int getSelectedMacroIndex() const {
        return selectedMacroIndex_;
    }

    // Selection state (this param cell is selected)
    void setSelected(bool selected) {
        selected_ = selected;
        repaint();
    }
    bool isSelected() const {
        return selected_;
    }

    // Callbacks
    std::function<void(double)> onValueChanged;
    std::function<void(int modIndex, aidaw::ControlTarget target)> onModLinked;
    std::function<void(int modIndex, aidaw::ControlTarget target, float amount)>
        onModLinkedWithAmount;
    std::function<void(int modIndex, aidaw::ControlTarget target)> onModUnlinked;
    std::function<void(int modIndex, aidaw::ControlTarget target)> onRackModUnlinked;
    std::function<void(int modIndex, aidaw::ControlTarget target)> onTrackModUnlinked;
    std::function<void(int modIndex, aidaw::ControlTarget target, float amount)> onModAmountChanged;
    std::function<void(int macroIndex, aidaw::ControlTarget target)> onMacroLinked;
    std::function<void(int macroIndex, aidaw::ControlTarget target, float amount)>
        onMacroLinkedWithAmount;
    std::function<void(int macroIndex, aidaw::ControlTarget target)> onMacroUnlinked;
    std::function<void(int macroIndex, aidaw::ControlTarget target)> onRackMacroLinked;
    std::function<void(int macroIndex, aidaw::ControlTarget target)> onTrackMacroLinked;
    std::function<void(int macroIndex, aidaw::ControlTarget target)> onRackMacroUnlinked;
    std::function<void(int macroIndex, aidaw::ControlTarget target)> onTrackMacroUnlinked;
    std::function<void(int macroIndex, aidaw::ControlTarget target, float amount)>
        onMacroAmountChanged;
    std::function<void(int macroIndex, float value)> onMacroValueChanged;
    std::function<void()> onShowAutomationLane;

    // MIDI Learn callbacks (wired to MidiLearnCoordinator by this component)
    std::function<void(aidaw::ChainNodePath, int paramIndex, juce::String paramName)> onMidiLearn;
    std::function<void(aidaw::ChainNodePath, int paramIndex)> onMidiClear;

    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;
    void resized() override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    // DragAndDropTarget
    bool isInterestedInDragSource(const SourceDetails& details) override;
    void itemDragEnter(const SourceDetails& details) override;
    void itemDragExit(const SourceDetails& details) override;
    void itemDropped(const SourceDetails& details) override;

  private:
    // Build a ParamLinkContext from current state
    ParamLinkContext buildLinkContext() const;

    // LinkModeManagerListener implementation
    void modLinkModeChanged(bool active, const aidaw::ModSelection& selection) override;
    void macroLinkModeChanged(bool active, const aidaw::MacroSelection& selection) override;

    // MidiLearnCoordinatorListener implementation
    void midiLearnStateChanged(const aidaw::ChainNodePath& path, int paramIndex,
                               aidaw::ControlTarget::Kind owner, bool learning) override;
    void midiLearnCompleted(const aidaw::ChainNodePath& path, int paramIndex,
                            aidaw::ControlTarget::Kind owner, const aidaw::Binding&) override;
    void midiLearnCleared(const aidaw::ChainNodePath& path, int paramIndex,
                          aidaw::ControlTarget::Kind owner, int numRemoved) override;

    // BindingRegistryListener implementation
    void bindingRegistryChanged(aidaw::BindingScope scope) override;

    // ControllerRegistryListener — toggling a controller's enabled state flips
    // whether its bindings count as active, so the indicator must refresh.
    void controllerRegistryChanged() override {
        refreshMidiBindingState();
    }

    // Refresh hasMidiBinding_ from the registry + repaint if changed.
    void refreshMidiBindingState();

    // Timer callback for animating LFO modulation bars and MIDI learn pulsing
    void timerCallback() override;

    // Update timer state based on whether there are active mod links
    void updateModTimerState();

    int paramIndex_;
    aidaw::DeviceId deviceId_ = aidaw::INVALID_DEVICE_ID;
    aidaw::ChainNodePath devicePath_;                          // For param selection
    const aidaw::ModArray* availableMods_ = nullptr;           // Device-level mods
    const aidaw::ModArray* availableRackMods_ = nullptr;       // Rack-level mods
    const aidaw::MacroArray* availableMacros_ = nullptr;       // Device-level macros
    const aidaw::MacroArray* availableRackMacros_ = nullptr;   // Rack-level macros
    const aidaw::ModArray* availableTrackMods_ = nullptr;      // Track-level mods
    const aidaw::MacroArray* availableTrackMacros_ = nullptr;  // Track-level macros
    int selectedModIndex_ = -1;    // -1 means no mod selected (show all)
    int selectedMacroIndex_ = -1;  // -1 means no macro selected (show all)
    bool selected_ = false;        // This param cell is selected

    juce::Label nameLabel_;
    TextSlider valueSlider_{TextSlider::Format::Decimal};
    std::unique_ptr<juce::ComboBox> discreteCombo_;   // For discrete/choice parameters
    std::unique_ptr<juce::ToggleButton> boolToggle_;  // For boolean parameters
    aidaw::ParameterInfo paramInfo_;                  // Parameter metadata for formatting

    // Shift+drag state for mod amount editing
    bool isModAmountDrag_ = false;
    int modAmountDragModIndex_ = -1;

    // Amount label shown during Shift+drag
    juce::Label amountLabel_;

    // Drag-and-drop state
    bool isDragOver_ = false;

    // Link mode state
    bool isInLinkMode_ = false;
    aidaw::ModSelection activeMod_;
    aidaw::MacroSelection activeMacro_;

    // MIDI Learn state
    bool isInMidiLearnMode_ = false;
    bool hasMidiBinding_ = false;  // Persistent badge for already-mapped params
    bool overlayOnly_ = false;
    bool linkOverlayVertical_ = false;

    // Link mode drag state (for setting modulation amount via drag)
    bool isLinkModeDrag_ = false;
    float linkModeDragStartAmount_ = 0.0f;
    float linkModeDragCurrentAmount_ = 0.0f;
    int linkModeDragStartY_ = 0;

    // Overlay slider for link mode
    std::unique_ptr<juce::Slider> linkModeSlider_;
    void showLinkModeSlider(bool isNewLink, float initialAmount);
    void hideLinkModeSlider();
    void handleLinkModeClick();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParamSlotComponent)
};

}  // namespace aidaw::daw::ui
