#pragma once

#include "core/MacroInfo.hpp"
#include "core/ModInfo.hpp"
#include "core/SelectionManager.hpp"
#include "core/TypeIds.hpp"

namespace aidaw::daw::ui {

/**
 * @brief Context for resolving mod/macro links to a specific parameter.
 *
 * Built once from ParamSlotComponent state, then passed to pure query functions
 * that don't need access to the component itself.
 */
struct ParamLinkContext {
    aidaw::DeviceId deviceId = aidaw::INVALID_DEVICE_ID;
    int paramIndex = -1;
    aidaw::ChainNodePath devicePath;
    const aidaw::ModArray* deviceMods = nullptr;
    const aidaw::ModArray* rackMods = nullptr;
    const aidaw::MacroArray* deviceMacros = nullptr;
    const aidaw::MacroArray* rackMacros = nullptr;
    const aidaw::ModArray* trackMods = nullptr;
    const aidaw::MacroArray* trackMacros = nullptr;
    int selectedModIndex = -1;    // -1 = show all
    int selectedMacroIndex = -1;  // -1 = show all
};

/**
 * @brief A resolved mod link returned by value (no static temporaries).
 */
struct ResolvedModLink {
    enum class Scope { Device, Rack, Track };
    int modIndex;
    aidaw::ModLink link;  // Copied by value — safe across threads
    Scope scope = Scope::Device;
};

/**
 * @brief A resolved macro link returned by value.
 */
struct ResolvedMacroLink {
    enum class Scope { Device, Rack, Track };
    int macroIndex;
    aidaw::MacroLink link;
    Scope scope = Scope::Device;
};

// Pure query functions — no side effects, no Component access
std::vector<ResolvedModLink> getLinkedMods(const ParamLinkContext& ctx);
std::vector<ResolvedMacroLink> getLinkedMacros(const ParamLinkContext& ctx);
bool hasActiveLinks(const ParamLinkContext& ctx);
float computeTotalModModulation(const ParamLinkContext& ctx);
float computeTotalMacroModulation(const ParamLinkContext& ctx);

/**
 * @brief Resolve a ModSelection to a concrete ModInfo pointer.
 *
 * Uses the parentPath in the selection to decide whether the mod is
 * device-level or rack-level, then validates the index.
 */
const aidaw::ModInfo* resolveModPtr(const aidaw::ModSelection& sel,
                                    const aidaw::ChainNodePath& devicePath,
                                    const aidaw::ModArray* deviceMods,
                                    const aidaw::ModArray* rackMods,
                                    const aidaw::ModArray* trackMods = nullptr);

/**
 * @brief Resolve a MacroSelection to a concrete MacroInfo pointer.
 */
const aidaw::MacroInfo* resolveMacroPtr(const aidaw::MacroSelection& sel,
                                        const aidaw::ChainNodePath& devicePath,
                                        const aidaw::MacroArray* deviceMacros,
                                        const aidaw::MacroArray* rackMacros,
                                        const aidaw::MacroArray* trackMacros = nullptr);

/**
 * @brief Check if a device path is within the scope of a parent path.
 *
 * Used to determine whether a parameter should respond to link-mode
 * events from a given mod/macro parent.
 */
bool isInScopeOf(const aidaw::ChainNodePath& devicePath, const aidaw::ChainNodePath& parentPath);

}  // namespace aidaw::daw::ui
