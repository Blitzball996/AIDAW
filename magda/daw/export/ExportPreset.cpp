#include "ExportPreset.hpp"

namespace magda {

ExportPresetManager::ExportPresetManager() = default;
ExportPresetManager::~ExportPresetManager() = default;

bool ExportPresetManager::savePreset(const ExportPreset& preset) {
    auto file = getPresetFile(preset.name);
    file.getParentDirectory().createDirectory();

    auto json = settingsToJson(preset.settings);

    auto* obj = json.getDynamicObject();
    if (obj)
        obj->setProperty("presetName", preset.name);

    auto jsonStr = juce::JSON::toString(json);
    return file.replaceWithText(jsonStr);
}

bool ExportPresetManager::loadPreset(const juce::String& name,
                                     ExportPreset& outPreset) {
    auto file = getPresetFile(name);
    if (!file.existsAsFile())
        return false;

    auto jsonStr = file.loadFileAsString();
    auto json = juce::JSON::parse(jsonStr);

    if (!json.isObject())
        return false;

    outPreset.name = name;
    outPreset.settings = jsonToSettings(json);
    return true;
}

bool ExportPresetManager::deletePreset(const juce::String& name) {
    auto file = getPresetFile(name);
    return file.deleteFile();
}

std::vector<juce::String> ExportPresetManager::getPresetNames() const {
    std::vector<juce::String> names;
    auto dir = getPresetsDirectory();

    if (!dir.isDirectory())
        return names;

    for (const auto& entry :
         juce::RangedDirectoryIterator(dir, false, "*.json")) {
        names.push_back(entry.getFile().getFileNameWithoutExtension());
    }

    return names;
}

juce::File ExportPresetManager::getPresetsDirectory() const {
    return juce::File::getSpecialLocation(
               juce::File::userApplicationDataDirectory)
        .getChildFile("MAGDA")
        .getChildFile("ExportPresets");
}

juce::var ExportPresetManager::settingsToJson(
    const ExportSettings& settings) const {
    auto* obj = new juce::DynamicObject();

    obj->setProperty("format", static_cast<int>(settings.format));
    obj->setProperty("sampleRate", settings.sampleRate);
    obj->setProperty("normalize", settings.normalize);
    obj->setProperty("trimSilence", settings.trimSilence);
    obj->setProperty("dithering", settings.dithering);
    obj->setProperty("startTime", settings.startTime);
    obj->setProperty("endTime", settings.endTime);
    obj->setProperty("outputPath", settings.outputPath);

    return juce::var(obj);
}

ExportSettings ExportPresetManager::jsonToSettings(
    const juce::var& json) const {
    ExportSettings s;

    s.format =
        static_cast<ExportFormat>(static_cast<int>(json["format"]));
    s.sampleRate = json.getProperty("sampleRate", 44100);
    s.normalize = json.getProperty("normalize", false);
    s.trimSilence = json.getProperty("trimSilence", false);
    s.dithering = json.getProperty("dithering", false);
    s.startTime = json.getProperty("startTime", 0.0);
    s.endTime = json.getProperty("endTime", -1.0);
    s.outputPath = json.getProperty("outputPath", "").toString();

    return s;
}

juce::File ExportPresetManager::getPresetFile(
    const juce::String& name) const {
    return getPresetsDirectory().getChildFile(name + ".json");
}

}  // namespace magda
