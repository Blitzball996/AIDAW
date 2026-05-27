#pragma once
#include <cmath>
#include <algorithm>

namespace magda::daw::audio::tidal {

// Wavetable oscillator with multiple waveforms
class Oscillator {
  public:
    enum Waveform { Sine, Saw, Square, Triangle, Pulse, Noise };

    void setWaveform(Waveform w) { waveform_ = w; }
    void setFrequency(float freq) { frequency_ = freq; }
    void setPulseWidth(float pw) { pulseWidth_ = std::clamp(pw, 0.01f, 0.99f); }

    float render(float sampleRate) {
        phase_ += frequency_ / sampleRate;
        if (phase_ >= 1.0f) phase_ -= 1.0f;

        switch (waveform_) {
            case Sine:     return std::sin(phase_ * 6.283185307f);
            case Saw:      return 2.0f * phase_ - 1.0f;
            case Square:   return phase_ < 0.5f ? 1.0f : -1.0f;
            case Triangle: return 4.0f * std::abs(phase_ - 0.5f) - 1.0f;
            case Pulse:    return phase_ < pulseWidth_ ? 1.0f : -1.0f;
            case Noise:    return ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        }
        return 0.0f;
    }

    void reset() { phase_ = 0.0f; }

  private:
    Waveform waveform_ = Saw;
    float frequency_ = 440.0f;
    float phase_ = 0.0f;
    float pulseWidth_ = 0.5f;
};

// Supersaw: multiple detuned sawtooth oscillators
class SuperSaw {
  public:
    void setFrequency(float freq) { baseFreq_ = freq; }
    void setDetune(float cents) { detune_ = cents; }
    void setVoices(int n) { numVoices_ = std::clamp(n, 1, 7); }

    float render(float sampleRate) {
        float sum = 0.0f;
        for (int i = 0; i < numVoices_; ++i) {
            float detuneRatio = (i - numVoices_ / 2.0f) / numVoices_;
            float freq = baseFreq_ * std::pow(2.0f, detuneRatio * detune_ / 1200.0f);
            phases_[i] += freq / sampleRate;
            if (phases_[i] >= 1.0f) phases_[i] -= 1.0f;
            sum += 2.0f * phases_[i] - 1.0f;
        }
        return sum / std::sqrt((float)numVoices_);
    }

    void reset() { for (auto& p : phases_) p = (float)rand() / RAND_MAX; }

  private:
    float baseFreq_ = 440.0f;
    float detune_ = 20.0f;
    int numVoices_ = 5;
    float phases_[7] = {};
};

}  // namespace magda::daw::audio::tidal
