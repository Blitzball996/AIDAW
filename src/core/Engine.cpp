#include "Engine.hpp"

namespace aidaw {

Engine::Engine() = default;
Engine::~Engine() { shutdown(); }

void Engine::initialize() {
    if (initialized) return;
    deviceManager.initialiseWithDefaultDevices(2, 2);
    initialized = true;
}

void Engine::shutdown() {
    if (!initialized) return;
    deviceManager.closeAudioDevice();
    initialized = false;
}

void Engine::setBufferSize(int size) {
    auto* device = deviceManager.getCurrentAudioDevice();
    if (device) {
        auto setup = deviceManager.getAudioDeviceSetup();
        setup.bufferSize = size;
        deviceManager.setAudioDeviceSetup(setup, true);
    }
}

void Engine::setSampleRate(int rate) {
    auto* device = deviceManager.getCurrentAudioDevice();
    if (device) {
        auto setup = deviceManager.getAudioDeviceSetup();
        setup.sampleRate = rate;
        deviceManager.setAudioDeviceSetup(setup, true);
    }
}

}  // namespace aidaw
