#include "modes/ModeController.hpp"
#include <cassert>
#include <iostream>

using namespace aidaw;

class TestListener : public ModeListener {
public:
    AppMode lastMode = AppMode::Professional;
    int callCount = 0;

    void modeChanged(AppMode newMode, const ModeProfile& /*profile*/) override {
        lastMode = newMode;
        callCount++;
    }
};

void testModeController() {
    auto& mc = ModeController::getInstance();
    TestListener listener;
    mc.addListener(&listener);

    assert(mc.getCurrentMode() == AppMode::Professional);

    mc.switchMode(AppMode::AiMinimal);
    assert(mc.getCurrentMode() == AppMode::AiMinimal);
    assert(listener.lastMode == AppMode::AiMinimal);
    assert(listener.callCount == 1);

    mc.toggleMode();
    assert(mc.getCurrentMode() == AppMode::Professional);
    assert(listener.callCount == 2);

    mc.removeListener(&listener);
    mc.switchMode(AppMode::Professional);

    std::cout << "ModeController tests passed\n";
}

void testModeProfile() {
    auto pro = ModeProfile::getProProfile();
    assert(pro.bufferSize == 512);

    auto ai = ModeProfile::getAiProfile();
    assert(ai.bufferSize == 1024);

    std::cout << "ModeProfile tests passed\n";
}

int main() {
    testModeController();
    testModeProfile();
    std::cout << "All mode tests passed!\n";
    return 0;
}
