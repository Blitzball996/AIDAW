#pragma once

#include <tracktion_engine/tracktion_engine.h>
#include <cmath>
#include <vector>

namespace magda::daw::audio {

namespace te = tracktion::engine;

// Simple built-in synth voice
struct TidalVoice {
    bool active = false;
    int note = 60;
    float frequency = 440.0f;
    float velocity = 1.0f;
    float phase = 0.0f;
    float envLevel = 0.0f;
    float envStage = 0;  // 0=attack, 1=sustain, 2=release
    double startTime = 0.0;
    double endTime = 0.0;

    void trigger(int n, float vel, double start, double dur, float sampleRate) {
        active = true;
        note = n;
        velocity = vel;
        frequency = 440.0f * std::pow(2.0f, (n - 69) / 12.0f);
        phase = 0.0f;
        envLevel = 0.0f;
        envStage = 0;
        startTime = start;
        endTime = start + dur;
    }

    float render(float sampleRate) {
        if (!active) return 0.0f;

        // Sawtooth oscillator
        phase += frequency / sampleRate;
        if (phase >= 1.0f) phase -= 1.0f;
        float osc = 2.0f * phase - 1.0f;

        // Simple envelope
        const float attackRate = 1.0f / (0.005f * sampleRate);
        const float releaseRate = 1.0f / (0.1f * sampleRate);

        if (envStage == 0) {
            envLevel += attackRate;
            if (envLevel >= 1.0f) { envLevel = 1.0f; envStage = 1; }
        } else if (envStage == 2) {
            envLevel -= releaseRate;
            if (envLevel <= 0.0f) { envLevel = 0.0f; active = false; }
        }

        return osc * envLevel * velocity * 0.3f;
    }

    void release() {
        envStage = 2;
    }
};

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

    // JS engine (for pattern evaluation)
    struct JSEngine;
    std::unique_ptr<JSEngine> js_;

    // Synth
    static constexpr int kMaxVoices = 16;
    TidalVoice voices_[kMaxVoices];
    double sampleRate_ = 44100.0;
    double lastCyclePos_ = 0.0;
    juce::String lastError_;
    bool patternActive_ = false;

    // Cached events for current pattern
    std::vector<NoteEvent> cachedEvents_;
    juce::String cachedCode_;

    void initJS();
    void queryAndRender(double startCycle, double endCycle, float* outL, float* outR, int numSamples);
    static int noteNameToMidi(const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StrudelPlugin)
};

}  // namespace magda::daw::audio
