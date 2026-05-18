#pragma once

#include <juce_core/juce_core.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace aidaw {

class ConfigListener {
  public:
    virtual ~ConfigListener() = default;
    virtual void configChanged() = 0;
};

/**
 * @brief Configuration class to manage all configurable settings in the DAW
 */
class Config {
  public:
    static Config& getInstance();

    void addListener(ConfigListener* l) { listeners_.push_back(l); }
    void removeListener(ConfigListener* l) {
        listeners_.erase(std::remove(listeners_.begin(), listeners_.end(), l), listeners_.end());
    }

    // Timeline Configuration
    int getDefaultTimelineLengthBars() const { return defaultTimelineLengthBars; }
    void setDefaultTimelineLengthBars(int bars) { defaultTimelineLengthBars = bars; }

    // Zoom Configuration
    double getMinZoomLevel() const { return minZoomLevel; }
    void setMinZoomLevel(double level) { minZoomLevel = level; }
    double getMaxZoomLevel() const { return maxZoomLevel; }
    void setMaxZoomLevel(double level) { maxZoomLevel = level; }

    double getZoomInSensitivity() const { return zoomInSensitivity; }
    void setZoomInSensitivity(double s) { zoomInSensitivity = s; }
    double getZoomOutSensitivity() const { return zoomOutSensitivity; }
    void setZoomOutSensitivity(double s) { zoomOutSensitivity = s; }

    // Panel Visibility
    bool getShowLeftPanel() const { return showLeftPanel; }
    void setShowLeftPanel(bool show) { showLeftPanel = show; }
    bool getShowRightPanel() const { return showRightPanel; }
    void setShowRightPanel(bool show) { showRightPanel = show; }
    bool getShowBottomPanel() const { return showBottomPanel; }
    void setShowBottomPanel(bool show) { showBottomPanel = show; }

    // Audio Device Configuration
    std::string getPreferredAudioDevice() const { return preferredAudioDevice; }
    void setPreferredAudioDevice(const std::string& d) { preferredAudioDevice = d; }

    // Custom Plugin Paths
    std::vector<std::string> getCustomPluginPaths() const { return customPluginPaths; }
    void setCustomPluginPaths(const std::vector<std::string>& paths) { customPluginPaths = paths; }

    // Recent Projects
    std::vector<std::string> getRecentProjects() const { return recentProjects; }
    void addRecentProject(const std::string& path);
    void clearRecentProjects() { recentProjects.clear(); }

    // Auto-save
    bool getAutoSaveEnabled() const { return autoSaveEnabled; }
    void setAutoSaveEnabled(bool enabled) { autoSaveEnabled = enabled; }
    int getAutoSaveIntervalSeconds() const { return autoSaveIntervalSeconds; }
    void setAutoSaveIntervalSeconds(int seconds) { autoSaveIntervalSeconds = std::max(10, seconds); }

    // UI Scale
    double getUIScale() const { return uiScale; }
    void setUIScale(double scale) { uiScale = scale; }

    // AI Configuration
    struct AgentLLMConfig {
        std::string provider = "openai_chat";
        std::string baseUrl;
        std::string apiKey;
        std::string model;
    };

    AgentLLMConfig getAgentLLMConfig(const std::string& role) const {
        auto it = agentConfigs.find(role);
        if (it != agentConfigs.end()) return it->second;
        return {};
    }
    void setAgentLLMConfig(const std::string& role, const AgentLLMConfig& config) {
        agentConfigs[role] = config;
    }

    std::string getAICredential(const std::string& provider) const {
        auto it = aiCredentials.find(provider);
        if (it != aiCredentials.end()) return it->second;
        return {};
    }
    void setAICredential(const std::string& provider, const std::string& key) {
        aiCredentials[provider] = key;
    }

    // Save/load
    void save();
    void load();

  private:
    Config() = default;

    int defaultTimelineLengthBars = 256;
    double minZoomLevel = 0.01;
    double maxZoomLevel = 10000.0;
    double zoomInSensitivity = 25.0;
    double zoomOutSensitivity = 40.0;

    bool showLeftPanel = true;
    bool showRightPanel = true;
    bool showBottomPanel = true;

    std::string preferredAudioDevice;
    std::vector<std::string> customPluginPaths;
    std::vector<std::string> recentProjects;

    bool autoSaveEnabled = true;
    int autoSaveIntervalSeconds = 60;

    double uiScale = 0.0;

    std::map<std::string, AgentLLMConfig> agentConfigs;
    std::map<std::string, std::string> aiCredentials;

    std::vector<ConfigListener*> listeners_;
};

}  // namespace aidaw
