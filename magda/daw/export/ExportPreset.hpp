#pragma once

#include <juce_core/juce_core.h>

#include <vector>

#include "ExportFormat.hpp"

namespace magda {

/**
 * @brief Named export preset that can be saved/loaded as JSON
 */
struct ExportPreset {
    juce::String name;
    ExportSettings settings;
};

/**
 * @brief Manages saving and loading of export presets
 */
class ExportPresetManager {
  public:
    ExportPresetManager();
    ~ExportPresetManager();

    /**
     * @brief Save a preset to the presets directory
     */
    bool savePreset(const ExportPreset& preset);

    /**
     * @brief Load a preset by name
     * @return true if loaded successfully
     */
    bool loadPreset(const juce::String& name, ExportPreset& outPreset);

    /**
     * @brief Delete a preset by name
     */
    bool deletePreset(const juce::String& name);

    /**
     * @brief Get all available preset names
     */
    std::vector<juce::String> getPresetNames() const;

    /**
     * @brief Get the directory where presets are stored
     */
    juce::File getPresetsDirectory() const;

  private:
    juce::var settingsToJson(const ExportSettings& settings) const;
    ExportSettings jsonToSettings(const juce::var& json) const;
    juce::File getPresetFile(const juce::String& name) const;
};

}  // namespace magda
