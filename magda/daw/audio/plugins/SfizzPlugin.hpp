#pragma once

#include <tracktion_engine/tracktion_engine.h>

#include <memory>
#include <string>

namespace sfz { class Sfizz; }

namespace magda::daw::audio {

namespace te = tracktion::engine;

class SfizzPlugin : public te::Plugin {
  public:
    SfizzPlugin(const te::PluginCreationInfo&);
    ~SfizzPlugin() override;

    static const char* getPluginName() { return "Sfizz Sampler"; }
    static const char* xmlTypeName;

    juce::String getName() const override { return getPluginName(); }
    juce::String getPluginType() override { return xmlTypeName; }
    juce::String getShortName(int) override { return "SFZ"; }
    juce::String getSelectableDescription() override { return getName(); }

    void initialise(const te::PluginInitialisationInfo&) override;
    void deinitialise() override;
    void reset() override;
    void applyToBuffer(const te::PluginRenderContext&) override;

    bool takesMidiInput() override { return true; }
    bool takesAudioInput() override { return false; }
    bool isSynth() override { return true; }
    bool producesAudioWhenNoAudioInput() override { return true; }
    double getTailLength() const override { return 2.0; }

    void restorePluginStateFromValueTree(const juce::ValueTree&) override;

    // SFZ file management
    juce::StringArray getAvailableSfzFiles() const;
    juce::String getCurrentSfzFile() const;
    void loadSfzByName(const juce::String& name);

    juce::CachedValue<juce::String> sfzFileValue;
    te::AutomatableParameter::Ptr volumeParam;

  private:
    std::unique_ptr<sfz::Sfizz> synth_;
    double sampleRate_ = 44100.0;
    int blockSize_ = 512;

    void loadSfzFile();
    juce::File getSfzDirectory() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SfizzPlugin)
};

}  // namespace magda::daw::audio
