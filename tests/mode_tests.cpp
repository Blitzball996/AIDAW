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

    // Reset to known state
    mc.switchMode(AppMode::AiMinimal);
    mc.switchMode(AppMode::Professional);
    listener.callCount = 0;

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

    std::cout << "testModeController passed\n";
}

void testModeProfile() {
    auto pro = ModeProfile::getProProfile();
    assert(pro.bufferSize == 512);

    auto ai = ModeProfile::getAiProfile();
    assert(ai.bufferSize == 1024);

    std::cout << "testModeProfile passed\n";
}

void testModeControllerMultipleListeners() {
    auto& mc = ModeController::getInstance();

    // Ensure we start at Professional
    mc.switchMode(AppMode::AiMinimal);
    mc.switchMode(AppMode::Professional);

    TestListener listener1;
    TestListener listener2;
    TestListener listener3;

    mc.addListener(&listener1);
    mc.addListener(&listener2);
    mc.addListener(&listener3);

    mc.switchMode(AppMode::AiMinimal);

    // All listeners should be notified
    assert(listener1.callCount == 1);
    assert(listener2.callCount == 1);
    assert(listener3.callCount == 1);
    assert(listener1.lastMode == AppMode::AiMinimal);
    assert(listener2.lastMode == AppMode::AiMinimal);
    assert(listener3.lastMode == AppMode::AiMinimal);

    // Remove one listener and switch again
    mc.removeListener(&listener2);
    mc.switchMode(AppMode::Professional);

    assert(listener1.callCount == 2);
    assert(listener2.callCount == 1);  // Not notified
    assert(listener3.callCount == 2);

    mc.removeListener(&listener1);
    mc.removeListener(&listener3);

    std::cout << "testModeControllerMultipleListeners passed\n";
}

void testModeControllerSwitchToSameMode() {
    auto& mc = ModeController::getInstance();

    // Ensure we start at Professional
    mc.switchMode(AppMode::AiMinimal);
    mc.switchMode(AppMode::Professional);

    TestListener listener;
    mc.addListener(&listener);

    // Switch to same mode should be a no-op (no notification)
    mc.switchMode(AppMode::Professional);
    assert(listener.callCount == 0);
    assert(mc.getCurrentMode() == AppMode::Professional);

    // Switch to different mode should notify
    mc.switchMode(AppMode::AiMinimal);
    assert(listener.callCount == 1);

    // Switch to same mode again - no-op
    mc.switchMode(AppMode::AiMinimal);
    assert(listener.callCount == 1);

    mc.removeListener(&listener);
    mc.switchMode(AppMode::Professional);

    std::cout << "testModeControllerSwitchToSameMode passed\n";
}

void testModeControllerToggle() {
    auto& mc = ModeController::getInstance();

    // Ensure we start at Professional
    mc.switchMode(AppMode::AiMinimal);
    mc.switchMode(AppMode::Professional);

    // Toggle from Professional -> AiMinimal
    mc.toggleMode();
    assert(mc.getCurrentMode() == AppMode::AiMinimal);

    // Toggle from AiMinimal -> Professional
    mc.toggleMode();
    assert(mc.getCurrentMode() == AppMode::Professional);

    // Toggle twice returns to original
    mc.toggleMode();
    mc.toggleMode();
    assert(mc.getCurrentMode() == AppMode::Professional);

    std::cout << "testModeControllerToggle passed\n";
}

void testModeControllerRemoveNonexistentListener() {
    auto& mc = ModeController::getInstance();

    TestListener listener;
    // Removing a listener that was never added should not crash
    mc.removeListener(&listener);

    std::cout << "testModeControllerRemoveNonexistentListener passed\n";
}

void testModeControllerGetCurrentProfile() {
    auto& mc = ModeController::getInstance();

    mc.switchMode(AppMode::AiMinimal);
    mc.switchMode(AppMode::Professional);
    auto profile = mc.getCurrentProfile();
    assert(profile.bufferSize == 512);
    assert(profile.latencyMs == 12);

    mc.switchMode(AppMode::AiMinimal);
    profile = mc.getCurrentProfile();
    assert(profile.bufferSize == 1024);
    assert(profile.latencyMs == 23);

    mc.switchMode(AppMode::Professional);

    std::cout << "testModeControllerGetCurrentProfile passed\n";
}

void testModeProfileAllProfilesValidBufferSizes() {
    auto pro = ModeProfile::getProProfile();
    auto ai = ModeProfile::getAiProfile();

    // Buffer sizes must be positive and power of 2
    assert(pro.bufferSize > 0);
    assert(ai.bufferSize > 0);
    assert((pro.bufferSize & (pro.bufferSize - 1)) == 0);  // Power of 2
    assert((ai.bufferSize & (ai.bufferSize - 1)) == 0);    // Power of 2

    // Buffer sizes should be in reasonable audio range (64 - 8192)
    assert(pro.bufferSize >= 64 && pro.bufferSize <= 8192);
    assert(ai.bufferSize >= 64 && ai.bufferSize <= 8192);

    // Latency must be positive
    assert(pro.latencyMs > 0);
    assert(ai.latencyMs > 0);

    std::cout << "testModeProfileAllProfilesValidBufferSizes passed\n";
}

void testModeProfileGetProfileForMode() {
    auto proProfile = ModeProfile::getProfileForMode(AppMode::Professional);
    auto aiProfile = ModeProfile::getProfileForMode(AppMode::AiMinimal);

    // Should match the dedicated getters
    auto expectedPro = ModeProfile::getProProfile();
    auto expectedAi = ModeProfile::getAiProfile();

    assert(proProfile.bufferSize == expectedPro.bufferSize);
    assert(proProfile.latencyMs == expectedPro.latencyMs);
    assert(proProfile.lowLatencyMode == expectedPro.lowLatencyMode);

    assert(aiProfile.bufferSize == expectedAi.bufferSize);
    assert(aiProfile.latencyMs == expectedAi.latencyMs);
    assert(aiProfile.lowLatencyMode == expectedAi.lowLatencyMode);

    std::cout << "testModeProfileGetProfileForMode passed\n";
}

void testModeListenerReceivesCorrectProfile() {
    auto& mc = ModeController::getInstance();

    // Ensure starting state
    mc.switchMode(AppMode::AiMinimal);
    mc.switchMode(AppMode::Professional);

    class ProfileCheckListener : public ModeListener {
    public:
        ModeProfile receivedProfile{0, 0, false};
        void modeChanged(AppMode /*newMode*/, const ModeProfile& profile) override {
            receivedProfile = profile;
        }
    };

    ProfileCheckListener listener;
    mc.addListener(&listener);

    mc.switchMode(AppMode::AiMinimal);
    assert(listener.receivedProfile.bufferSize == 1024);
    assert(listener.receivedProfile.latencyMs == 23);

    mc.switchMode(AppMode::Professional);
    assert(listener.receivedProfile.bufferSize == 512);
    assert(listener.receivedProfile.latencyMs == 12);

    mc.removeListener(&listener);

    std::cout << "testModeListenerReceivesCorrectProfile passed\n";
}

int main() {
    testModeController();
    testModeProfile();
    testModeControllerMultipleListeners();
    testModeControllerSwitchToSameMode();
    testModeControllerToggle();
    testModeControllerRemoveNonexistentListener();
    testModeControllerGetCurrentProfile();
    testModeProfileAllProfilesValidBufferSizes();
    testModeProfileGetProfileForMode();
    testModeListenerReceivesCorrectProfile();
    std::cout << "All mode tests passed!\n";
    return 0;
}
