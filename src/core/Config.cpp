#include "Config.hpp"

#include <juce_core/juce_core.h>
#include <algorithm>

namespace aidaw {

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

void Config::addRecentProject(const std::string& path) {
    recentProjects.erase(std::remove(recentProjects.begin(), recentProjects.end(), path),
                         recentProjects.end());
    recentProjects.insert(recentProjects.begin(), path);
    if (recentProjects.size() > 10)
        recentProjects.resize(10);
}

void Config::save() {
    auto root = juce::DynamicObject::Ptr(new juce::DynamicObject());

    root->setProperty("defaultTimelineLengthBars", defaultTimelineLengthBars);
    root->setProperty("minZoomLevel", minZoomLevel);
    root->setProperty("maxZoomLevel", maxZoomLevel);
    root->setProperty("zoomInSensitivity", zoomInSensitivity);
    root->setProperty("zoomOutSensitivity", zoomOutSensitivity);
    root->setProperty("showLeftPanel", showLeftPanel);
    root->setProperty("showRightPanel", showRightPanel);
    root->setProperty("showBottomPanel", showBottomPanel);
    root->setProperty("autoSaveEnabled", autoSaveEnabled);
    root->setProperty("autoSaveIntervalSeconds", autoSaveIntervalSeconds);
    root->setProperty("uiScale", uiScale);

    if (!preferredAudioDevice.empty())
        root->setProperty("preferredAudioDevice",
                          juce::String::fromUTF8(preferredAudioDevice.c_str()));

    // Recent projects
    juce::Array<juce::var> recentArr;
    for (const auto& p : recentProjects)
        recentArr.add(juce::String::fromUTF8(p.c_str()));
    root->setProperty("recentProjects", recentArr);

    // Write to file
    auto configDir = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory).getChildFile("AIDAW");
    configDir.createDirectory();
    auto configFile = configDir.getChildFile("config.json");

    juce::String json = juce::JSON::toString(juce::var(root.get()));
    configFile.replaceWithText(json);
}

void Config::load() {
    auto configDir = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory).getChildFile("AIDAW");
    auto configFile = configDir.getChildFile("config.json");

    if (!configFile.existsAsFile())
        return;

    auto parsed = juce::JSON::parse(configFile.loadFileAsString());
    if (auto* obj = parsed.getDynamicObject()) {
        defaultTimelineLengthBars = obj->getProperty("defaultTimelineLengthBars");
        if (obj->hasProperty("minZoomLevel"))
            minZoomLevel = obj->getProperty("minZoomLevel");
        if (obj->hasProperty("maxZoomLevel"))
            maxZoomLevel = obj->getProperty("maxZoomLevel");
        if (obj->hasProperty("zoomInSensitivity"))
            zoomInSensitivity = obj->getProperty("zoomInSensitivity");
        if (obj->hasProperty("zoomOutSensitivity"))
            zoomOutSensitivity = obj->getProperty("zoomOutSensitivity");
        if (obj->hasProperty("showLeftPanel"))
            showLeftPanel = obj->getProperty("showLeftPanel");
        if (obj->hasProperty("showRightPanel"))
            showRightPanel = obj->getProperty("showRightPanel");
        if (obj->hasProperty("showBottomPanel"))
            showBottomPanel = obj->getProperty("showBottomPanel");
        if (obj->hasProperty("autoSaveEnabled"))
            autoSaveEnabled = obj->getProperty("autoSaveEnabled");
        if (obj->hasProperty("autoSaveIntervalSeconds"))
            autoSaveIntervalSeconds = obj->getProperty("autoSaveIntervalSeconds");
        if (obj->hasProperty("uiScale"))
            uiScale = obj->getProperty("uiScale");
        if (obj->hasProperty("preferredAudioDevice"))
            preferredAudioDevice = obj->getProperty("preferredAudioDevice").toString().toStdString();

        if (auto* arr = obj->getProperty("recentProjects").getArray()) {
            recentProjects.clear();
            for (const auto& item : *arr)
                recentProjects.push_back(item.toString().toStdString());
        }
    }

    for (auto* l : listeners_)
        l->configChanged();
}

}  // namespace aidaw
