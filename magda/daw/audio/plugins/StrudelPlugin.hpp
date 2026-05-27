#pragma once

#include <tracktion_engine/tracktion_engine.h>
#include "strudel_synth/SynthEngine.h"

#include <vector>

namespace magda::daw::audio {

namespace te = tracktion::engine;

class StrudelPlugin : public te::Plugin {
  public:
    StrudelPlugin(const te::PluginCreationInfo&);
    ~StrudelPlugin() override;

    static const char* getPluginName() { return "Tidal"; }
    static const char* xmlTypeName;

    juce::String getName() const override { return getPluginName(); }
    juce::String getPluginType() override { return xmlTypeName; }
    juce::String getShortName(int) override { return "TIDAL"; }
    juce::String getSelectableDescription() override { return getName(); }

    void initialise(const te::PluginInitialisationInfo&) override;
    void deinitialise() override;
    void reset() override;
    void applyToBuffer(const te::PluginRenderContext&) override;

    std::unique_ptr<EditorComponent> createEditor() override;

    bool takesMidiInput() override { return false; }
    bool takesAudioInput() override { return false; }
    bool isSynth() override { return true; }
    bool producesAudioWhenNoAudioInput() override { return true; }
    double getTailLength() const override { return 0.5; }

    void restorePluginStateFromValueTree(const juce::ValueTree&) override;

    juce::String evaluate(const juce::String& code);
    juce::String getCode() const;
    void setCode(const juce::String& code);
    juce::String getLastError() const;
    bool hasActivePattern() const;

    juce::CachedValue<juce::String> codeValue;

  private:
    struct NoteEvent {
        int note;
        double onset;
        double duration;
        float velocity;
    };

    struct JSEngine;
    std::unique_ptr<JSEngine> js_;

    tidal::SynthEngine synth_;
    double sampleRate_ = 44100.0;
    double lastCyclePos_ = 0.0;
    juce::String lastError_;
    bool patternActive_ = false;

    std::vector<NoteEvent> cachedEvents_;
    juce::String cachedCode_;

    void initJS();
    static int noteNameToMidi(const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StrudelPlugin)
};

}  // namespace magda::daw::audio
