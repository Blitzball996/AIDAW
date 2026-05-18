#pragma once

namespace aidaw {

enum class AppMode {
    Professional,
    AiMinimal
};

/**
 * @brief View modes for the DAW (from magda ViewModeState)
 *
 * Each mode optimizes the audio engine for different workflows:
 * - Live: Real-time performance with lowest latency
 * - Arrange: Composing and editing with balanced settings
 * - Mix: Mixing and processing with higher buffer for plugins
 * - Master: Mastering with maximum quality settings
 */
enum class ViewMode { Live, Arrange, Mix, Master };

inline const char* getViewModeName(ViewMode mode) {
    switch (mode) {
        case ViewMode::Live:    return "Live";
        case ViewMode::Arrange: return "Arrange";
        case ViewMode::Mix:     return "Mix";
        case ViewMode::Master:  return "Master";
    }
    return "Unknown";
}

struct ModeProfile {
    int bufferSize;
    int latencyMs;
    bool lowLatencyMode;
    bool multiThreaded;

    static ModeProfile getProProfile() {
        return {512, 12, false, true};
    }

    static ModeProfile getAiProfile() {
        return {1024, 23, false, true};
    }

    static ModeProfile getProfileForMode(AppMode mode) {
        switch (mode) {
            case AppMode::Professional: return getProProfile();
            case AppMode::AiMinimal:    return getAiProfile();
        }
        return getProProfile();
    }

    // Audio engine profiles per view mode
    static ModeProfile getLiveProfile()    { return {128, 3, true, false}; }
    static ModeProfile getArrangeProfile() { return {512, 12, false, true}; }
    static ModeProfile getMixProfile()     { return {1024, 23, false, true}; }
    static ModeProfile getMasterProfile()  { return {2048, 46, false, true}; }

    static ModeProfile getProfileForViewMode(ViewMode mode) {
        switch (mode) {
            case ViewMode::Live:    return getLiveProfile();
            case ViewMode::Arrange: return getArrangeProfile();
            case ViewMode::Mix:     return getMixProfile();
            case ViewMode::Master:  return getMasterProfile();
        }
        return getArrangeProfile();
    }
};

}  // namespace aidaw
