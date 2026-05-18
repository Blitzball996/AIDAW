#pragma once

namespace aidaw {

enum class AppMode {
    Professional,
    AiMinimal
};

struct ModeProfile {
    int bufferSize;
    int latencyMs;
    bool lowLatencyMode;

    static ModeProfile getProProfile() {
        return {512, 12, false};
    }

    static ModeProfile getAiProfile() {
        return {1024, 23, false};
    }

    static ModeProfile getProfileForMode(AppMode mode) {
        switch (mode) {
            case AppMode::Professional: return getProProfile();
            case AppMode::AiMinimal:    return getAiProfile();
        }
        return getProProfile();
    }
};

}  // namespace aidaw
