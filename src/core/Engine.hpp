#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <memory>

namespace aidaw {

class Engine {
public:
    Engine();
    ~Engine();

    void initialize();
    void shutdown();

    void setBufferSize(int size);
    void setSampleRate(int rate);

    juce::AudioDeviceManager& getDeviceManager() { return deviceManager; }

private:
    juce::AudioDeviceManager deviceManager;
    bool initialized = false;
};

}  // namespace aidaw
