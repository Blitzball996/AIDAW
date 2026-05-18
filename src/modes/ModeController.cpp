#include "ModeController.hpp"
#include <algorithm>

namespace aidaw {

ModeController& ModeController::getInstance() {
    static ModeController instance;
    return instance;
}

void ModeController::switchMode(AppMode newMode) {
    if (currentMode == newMode) return;
    currentMode = newMode;
    notifyListeners();
}

void ModeController::toggleMode() {
    switchMode(currentMode == AppMode::Professional
        ? AppMode::AiMinimal
        : AppMode::Professional);
}

void ModeController::setViewMode(ViewMode mode) {
    if (currentViewMode == mode) return;
    currentViewMode = mode;
    notifyViewModeListeners();
}

void ModeController::addListener(ModeListener* listener) {
    listeners.push_back(listener);
}

void ModeController::removeListener(ModeListener* listener) {
    listeners.erase(
        std::remove(listeners.begin(), listeners.end(), listener),
        listeners.end());
}

void ModeController::notifyListeners() {
    auto profile = getCurrentProfile();
    for (auto* l : listeners) {
        l->modeChanged(currentMode, profile);
    }
}

void ModeController::notifyViewModeListeners() {
    auto profile = getViewModeProfile();
    for (auto* l : listeners) {
        l->viewModeChanged(currentViewMode, profile);
    }
}

}  // namespace aidaw
