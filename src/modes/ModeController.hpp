#pragma once

#include "ModeState.hpp"
#include <vector>

namespace aidaw {

class ModeListener {
public:
    virtual ~ModeListener() = default;
    virtual void modeChanged(AppMode newMode, const ModeProfile& profile) = 0;
};

class ModeController {
public:
    static ModeController& getInstance();

    ModeController(const ModeController&) = delete;
    ModeController& operator=(const ModeController&) = delete;

    AppMode getCurrentMode() const { return currentMode; }
    ModeProfile getCurrentProfile() const { return ModeProfile::getProfileForMode(currentMode); }

    void switchMode(AppMode newMode);
    void toggleMode();

    void addListener(ModeListener* listener);
    void removeListener(ModeListener* listener);

private:
    ModeController() = default;

    AppMode currentMode = AppMode::Professional;
    std::vector<ModeListener*> listeners;

    void notifyListeners();
};

}  // namespace aidaw
