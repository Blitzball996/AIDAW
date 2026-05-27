#pragma once
#include "Oscillator.h"
#include "Envelope.h"
#include "Filter.h"
#include "Effects.h"
#include "FMSynth.h"
#include <cmath>

namespace magda::daw::audio::tidal {

// LFO for modulation
class LFO {
  public:
    enum Shape { Triangle, Sine, Saw, Square, Ramp };

    void setRate(float hz) { rate_ = hz; }
    void setDepth(float d) { depth_ = d; }
    void setShape(Shape s) { shape_ = s; }

    float render(float sampleRate) {
        phase_ += rate_ / sampleRate;
        if (phase_ >= 1.0f) phase_ -= 1.0f;

        float val;
        switch (shape_) {
            case Triangle: val = 4.0f * std::abs(phase_ - 0.5f) - 1.0f; break;
            case Sine:     val = std::sin(phase_ * 6.283185307f); break;
            case Saw:      val = 2.0f * phase_ - 1.0f; break;
            case Square:   val = phase_ < 0.5f ? 1.0f : -1.0f; break;
            case Ramp:     val = 1.0f - 2.0f * phase_; break;
            default:       val = 0.0f;
        }
        return val * depth_;
    }

    void reset() { phase_ = 0.0f; }

  private:
    float rate_ = 1.0f;
    float depth_ = 1.0f;
    Shape shape_ = Sine;
    float phase_ = 0.0f;
};

// Complete synth voice with oscillator + filter + envelope + effects
struct SynthVoice {
    bool active = false;
    int note = 60;
    float velocity = 1.0f;
    double startTime = 0.0;
    double duration = 0.0;

    Oscillator osc;
    SuperSaw supersaw;
    Envelope ampEnv;
    Envelope filterEnv;
    Filter filter;
    LFO filterLfo;

    // Parameters
    Oscillator::Waveform waveform = Oscillator::Saw;
    bool useSupersaw = false;
    float cutoff = 8000.0f;
    float resonance = 1.0f;
    float filterEnvDepth = 0.0f;  // in octaves
    float lfoRate = 0.0f;
    float lfoDepth = 0.0f;

    void trigger(int n, float vel, double start, double dur, float sampleRate) {
        active = true;
        note = n;
        velocity = vel;
        startTime = start;
        duration = dur;

        float freq = 440.0f * std::pow(2.0f, (n - 69) / 12.0f);
        osc.setWaveform(waveform);
        osc.setFrequency(freq);
        osc.reset();

        supersaw.setFrequency(freq);
        supersaw.reset();

        ampEnv.trigger();
        filterEnv.trigger();
        filter.reset();
        filter.setType(Filter::Lowpass);
        filterLfo.reset();
    }

    void noteOff() {
        ampEnv.release();
        filterEnv.release();
    }

    float render(float sampleRate) {
        if (!active) return 0.0f;

        // Oscillator
        float oscOut = useSupersaw ? supersaw.render(sampleRate) : osc.render(sampleRate);

        // Amplitude envelope
        float amp = ampEnv.render(sampleRate);
        if (!ampEnv.isActive()) { active = false; return 0.0f; }

        // Filter with envelope + LFO modulation
        float fEnv = filterEnv.render(sampleRate);
        float fLfo = (lfoRate > 0.0f) ? filterLfo.render(sampleRate) : 0.0f;
        float modCutoff = cutoff * std::pow(2.0f, fEnv * filterEnvDepth + fLfo * lfoDepth);
        modCutoff = std::clamp(modCutoff, 20.0f, 20000.0f);
        filter.setCutoff(modCutoff);
        filter.setResonance(resonance);

        float filtered = filter.process(oscOut, sampleRate);

        return filtered * amp * velocity * 0.3f;
    }
};

// Full synth engine with polyphony and global effects
class SynthEngine {
  public:
    static constexpr int kMaxVoices = 16;

    void init(float sampleRate) {
        sampleRate_ = sampleRate;
        delay_.init(sampleRate, 2.0f);
        reverb_.init(sampleRate);
    }

    void setWaveform(Oscillator::Waveform w) { waveform_ = w; }
    void setCutoff(float f) { cutoff_ = f; }
    void setResonance(float q) { resonance_ = q; }
    void setFilterEnv(float depth) { filterEnvDepth_ = depth; }
    void setAttack(float a) { attack_ = a; }
    void setDecay(float d) { decay_ = d; }
    void setSustain(float s) { sustain_ = s; }
    void setRelease(float r) { release_ = r; }
    void setDelayMix(float m) { delay_.setMix(m); }
    void setDelayTime(float t) { delay_.setTime(t); }
    void setDelayFeedback(float f) { delay_.setFeedback(f); }
    void setReverbMix(float m) { reverb_.setMix(m); }
    void setReverbSize(float s) { reverb_.setRoomSize(s); }
    void setDistortion(float d) { distDrive_ = d; }
    void setUseSupersaw(bool s) { useSupersaw_ = s; }

    void noteOn(int note, float velocity, double startCycle, double durCycles) {
        // Find free voice or steal oldest
        SynthVoice* voice = nullptr;
        for (auto& v : voices_) {
            if (!v.active) { voice = &v; break; }
        }
        if (!voice) {
            voice = &voices_[0];
            for (auto& v : voices_)
                if (v.startTime < voice->startTime) voice = &v;
        }

        voice->waveform = waveform_;
        voice->useSupersaw = useSupersaw_;
        voice->cutoff = cutoff_;
        voice->resonance = resonance_;
        voice->filterEnvDepth = filterEnvDepth_;
        voice->ampEnv.setADSR(attack_, decay_, sustain_, release_);
        voice->filterEnv.setADSR(0.01f, 0.2f, 0.3f, 0.3f);
        voice->trigger(note, velocity, startCycle, durCycles, sampleRate_);
    }

    void releaseVoicesAfter(double currentCycle) {
        for (auto& v : voices_) {
            if (v.active && v.ampEnv.getStage() < Envelope::Release) {
                if (currentCycle - v.startTime > v.duration)
                    v.noteOff();
            }
        }
    }

    float render() {
        float sum = 0.0f;
        for (auto& v : voices_) {
            if (v.active)
                sum += v.render(sampleRate_);
        }

        // Global effects
        if (distDrive_ > 1.0f) {
            dist_.setDrive(distDrive_);
            sum = dist_.process(sum);
        }
        sum = delay_.process(sum);
        sum = reverb_.process(sum);

        return std::tanh(sum);  // Final soft clip
    }

    void reset() {
        for (auto& v : voices_) v.active = false;
        delay_.reset();
        reverb_.reset();
    }

  private:
    SynthVoice voices_[kMaxVoices];
    Delay delay_;
    Reverb reverb_;
    Distortion dist_;
    float sampleRate_ = 44100.0f;

    // Current settings
    Oscillator::Waveform waveform_ = Oscillator::Saw;
    bool useSupersaw_ = false;
    float cutoff_ = 8000.0f;
    float resonance_ = 1.0f;
    float filterEnvDepth_ = 0.0f;
    float attack_ = 0.005f;
    float decay_ = 0.1f;
    float sustain_ = 0.7f;
    float release_ = 0.2f;
    float distDrive_ = 1.0f;
};

}  // namespace magda::daw::audio::tidal
