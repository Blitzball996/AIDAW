#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

#include "params/ParamLinkResolver.hpp"

namespace aidaw::daw::ui {

/**
 * @brief Callbacks invoked by the link menu when the user picks an option.
 */
struct ParamLinkMenuCallbacks {
    std::function<void(int modIndex, aidaw::ControlTarget target)> onModUnlinked;
    std::function<void(int modIndex, aidaw::ControlTarget target)> onRackModUnlinked;
    std::function<void(int modIndex, aidaw::ControlTarget target)> onTrackModUnlinked;
    std::function<void(int modIndex, aidaw::ControlTarget target, float amount)>
        onModLinkedWithAmount;
    std::function<void(int macroIndex, aidaw::ControlTarget target)> onMacroLinked;
    std::function<void(int macroIndex, aidaw::ControlTarget target, float amount)>
        onMacroLinkedWithAmount;
    std::function<void(int macroIndex, aidaw::ControlTarget target)> onMacroUnlinked;
    std::function<void(int macroIndex, aidaw::ControlTarget target)> onRackMacroLinked;
    std::function<void(int macroIndex, aidaw::ControlTarget target)> onTrackMacroLinked;
    std::function<void(int macroIndex, aidaw::ControlTarget target)> onRackMacroUnlinked;
    std::function<void(int macroIndex, aidaw::ControlTarget target)> onTrackMacroUnlinked;
    std::function<void()> onShowAutomationLane;

    // MIDI Learn callbacks (item IDs 6000-6009)
    std::function<void(aidaw::ChainNodePath, int paramIndex, juce::String paramName)> onMidiLearn;
    std::function<void(aidaw::ChainNodePath, int paramIndex)> onMidiClear;
    std::function<void(aidaw::ChainNodePath, int paramIndex)> onMidiEdit;  // greyed out for now
};

/**
 * @brief Show the context menu for linking/unlinking mods and macros.
 *
 * Handles both the "contextual" mode (a specific mod is selected) and the
 * "full" mode (no mod selected — shows all options).
 *
 * @param anchor   Component used as SafePointer target — must stay alive until
 *                 the async menu callback fires.
 * @param ctx      Link context describing which parameter, device, and arrays
 *                 to use.
 * @param callbacks Callbacks invoked when the user picks a menu item.
 */
void showParamLinkMenu(juce::Component* anchor, const ParamLinkContext& ctx,
                       const ParamLinkMenuCallbacks& callbacks);

}  // namespace aidaw::daw::ui
