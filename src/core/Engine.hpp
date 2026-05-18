#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <memory>

#ifdef AIDAW_HAS_TRACKTION
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion;
#endif

namespace aidaw {

class Engine {
public:
    Engine();
    ~Engine();

    void initialize();
    void shutdown();

    void setBufferSize(int size);
    void setSampleRate(int rate);

    juce::AudioDeviceManager& getDeviceManager();

#ifdef AIDAW_HAS_TRACKTION
    te::Engine& getTracktionEngine();
    te::Edit* getEdit() { return currentEdit.get(); }

    /** Create a new empty edit (discards the current one). */
    void createNewEdit();

    /** Load an edit from a ValueTree state. */
    void loadEdit(const juce::ValueTree& state);
#endif

private:
#ifdef AIDAW_HAS_TRACKTION
    std::unique_ptr<te::Engine> tracktionEngine;
    std::unique_ptr<te::Edit> currentEdit;
#else
    juce::AudioDeviceManager deviceManager;
#endif
    bool initialized = false;
};

}  // namespace aidaw
