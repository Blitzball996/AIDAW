#pragma once

#include "ModeState.hpp"
#include <vector>

namespace aidaw {

class ModeListener {
public:
    virtual ~ModeListener() = default;
    virtual void modeChanged(AppMode newMode, const ModeProfile& profile) = 0;
    virtual void viewModeChanged(ViewMode mode, const ModeProfile& profile) {
        (void)mode; (void)profile;
    }
};

class ModeController {
public:
    static ModeController& getInstance();

    ModeController(const ModeController&) = delete;
    ModeController& operator=(const ModeController&) = delete;

    AppMode getCurrentMode() const { return currentMode; }
    ModeProfile getCurrentProfile() const { return ModeProfile::getProfileForMode(currentMode); }

    ViewMode getViewMode() const { return currentViewMode; }
    ModeProfile getViewModeProfile() const {
        return ModeProfile::getProfileForViewMode(currentViewMode);
    }

    void switchMode(AppMode newMode);
    void toggleMode();

    void setViewMode(ViewMode mode);

    void addListener(ModeListener* listener);
    void removeListener(ModeListener* listener);

private:
    ModeController() = default;

    AppMode currentMode = AppMode::Professional;
    ViewMode currentViewMode = ViewMode::Arrange;
    std::vector<ModeListener*> listeners;

    void notifyListeners();
    void notifyViewModeListeners();
};

}  // namespace aidaw
