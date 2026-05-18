#include "Engine.hpp"

namespace aidaw {

Engine::Engine() = default;
Engine::~Engine() { shutdown(); }

void Engine::initialize() {
    if (initialized) return;

#ifdef AIDAW_HAS_TRACKTION
    tracktionEngine = std::make_unique<te::Engine>("AIDAW");
    createNewEdit();
#else
    deviceManager.initialiseWithDefaultDevices(2, 2);
#endif

    initialized = true;
}

void Engine::shutdown() {
    if (!initialized) return;

#ifdef AIDAW_HAS_TRACKTION
    currentEdit.reset();
    tracktionEngine.reset();
#else
    deviceManager.closeAudioDevice();
#endif

    initialized = false;
}

juce::AudioDeviceManager& Engine::getDeviceManager() {
#ifdef AIDAW_HAS_TRACKTION
    return tracktionEngine->getDeviceManager().deviceManager;
#else
    return deviceManager;
#endif
}

void Engine::setBufferSize(int size) {
    auto& dm = getDeviceManager();
    if (dm.getCurrentAudioDevice()) {
        auto setup = dm.getAudioDeviceSetup();
        setup.bufferSize = size;
        dm.setAudioDeviceSetup(setup, true);
    }
}

void Engine::setSampleRate(int rate) {
    auto& dm = getDeviceManager();
    if (dm.getCurrentAudioDevice()) {
        auto setup = dm.getAudioDeviceSetup();
        setup.sampleRate = rate;
        dm.setAudioDeviceSetup(setup, true);
    }
}

#ifdef AIDAW_HAS_TRACKTION
te::Engine& Engine::getTracktionEngine() {
    jassert(tracktionEngine != nullptr);
    return *tracktionEngine;
}

void Engine::createNewEdit() {
    currentEdit = te::Edit::createSingleTrackEdit(getTracktionEngine());
}

void Engine::loadEdit(const juce::ValueTree& state) {
    te::Edit::Options options {
        .engine = *tracktionEngine,
        .editState = state
    };
    currentEdit = te::Edit::createEdit(options);
}
#endif

}  // namespace aidaw
