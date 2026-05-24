#pragma once

#include <tracktion_engine/tracktion_engine.h>

struct tsf;

namespace magda::daw::audio {

namespace te = tracktion::engine;

class SoundFontPlugin : public te::Plugin {
  public:
    SoundFontPlugin(const te::PluginCreationInfo&);
    ~SoundFontPlugin() override;

    static const char* getPluginName() {
        return "SoundFont Player";
    }
    static const char* xmlTypeName;

    juce::String getName() const override {
        return getPluginName();
    }
    juce::String getPluginType() override {
        return xmlTypeName;
    }
    juce::String getShortName(int) override {
        return "SF2";
    }
    juce::String getSelectableDescription() override {
        return getName();
    }

    void initialise(const te::PluginInitialisationInfo&) override;
    void deinitialise() override;
    void reset() override;

    void applyToBuffer(const te::PluginRenderContext&) override;

    bool takesMidiInput() override {
        return true;
    }
    bool takesAudioInput() override {
        return false;
    }
    bool isSynth() override {
        return true;
    }
    bool producesAudioWhenNoAudioInput() override {
        return true;
    }
    double getTailLength() const override {
        return 1.0;
    }

    void restorePluginStateFromValueTree(const juce::ValueTree&) override;

    juce::CachedValue<int> programValue, bankValue;
    te::AutomatableParameter::Ptr volumeParam;

  private:
    tsf* soundFont_ = nullptr;
    double sampleRate_ = 44100.0;
    int lastProgram_ = 0;
    int lastBank_ = 0;

    void loadSoundFont();
    void applyProgramChange();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundFontPlugin)
};

}  // namespace magda::daw::audio
