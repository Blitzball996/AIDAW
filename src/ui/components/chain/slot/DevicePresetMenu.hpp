#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <optional>

#include "core/DeviceInfo.hpp"
#include "core/TypeIds.hpp"

namespace aidaw::daw::ui {

using PresetSnapshotProvider = std::function<std::optional<aidaw::DeviceInfo>()>;

struct MagdaPresetMenuActions {
    std::function<void()> saveAs;
    std::function<void()> saveCurrent;
    std::function<void(const juce::String& presetRelativePath)> loadPreset;
};

void showMagdaPresetMenu(juce::Component* targetComponent, const juce::String& pluginFolder,
                         const juce::String& currentPresetName, MagdaPresetMenuActions actions);

std::optional<aidaw::DeviceInfo> snapshotDeviceForPreset(const aidaw::DeviceInfo& fallbackDevice,
                                                         const aidaw::ChainNodePath& nodePath);

void showSaveMagdaPresetDialog(const aidaw::DeviceInfo& device,
                               const juce::String& currentPresetName,
                               PresetSnapshotProvider snapshotProvider,
                               std::function<void(const juce::String& presetName)> onSaved);

void saveCurrentMagdaPreset(const juce::String& currentPresetName,
                            PresetSnapshotProvider snapshotProvider);

void loadMagdaPreset(
    const juce::String& pluginFolder, const aidaw::ChainNodePath& nodePath,
    const juce::String& presetRelativePath,
    std::function<void(const aidaw::DeviceInfo& liveDevice, const juce::String& presetName)>
        onLoaded);

bool hasPluginPresetsAvailable(const aidaw::DeviceInfo& device, bool isInternalDevice);

struct PluginPresetMenuActions {
    std::function<void()> saveAs;
    std::function<void(const juce::File& file)> loadFile;
    std::function<void(const juce::File& currentFile, const juce::String& displayName)>
        selectionChanged;
};

void showPluginPresetMenu(juce::Component* targetComponent, const aidaw::DeviceInfo& device,
                          bool isInternalDevice, const juce::File& currentPluginPresetFile,
                          PluginPresetMenuActions actions);

void loadPluginPresetFile(
    aidaw::DeviceId deviceId, const juce::File& file,
    std::function<void(const juce::File& currentFile, const juce::String& displayName)> onLoaded);

void showSavePluginPresetDialog(
    const aidaw::DeviceInfo& device, const juce::String& currentPluginPresetName,
    std::function<void(const juce::File& currentFile, const juce::String& displayName)> onSaved);

}  // namespace aidaw::daw::ui
