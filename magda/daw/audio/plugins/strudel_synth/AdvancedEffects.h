#pragma once
#include <cmath>
#include <algorithm>
#include <vector>
#include <array>

namespace magda::daw::audio::tidal {

// Phaser — chain of allpass filters modulated by LFO
class Phaser {
  public:
    void setRate(float hz) { rate_ = std::max(0.01f, hz); }
    void setDepth(float d) { depth_ = std::clamp(d, 0.0f, 1.0f); }
    void setCenterFrequency(float freq) { centerFreq_ = std::clamp(freq, 100.0f, 10000.0f); }
    void setFeedback(float fb) { feedback_ = std::clamp(fb, -0.95f, 0.95f); }
    void setStages(int n) { stages_ = std::clamp(n, 2, 12); }

    float process(float input, float sampleRate) {
        // LFO
        lfoPhase_ += rate_ / sampleRate;
        if (lfoPhase_ >= 1.0f) lfoPhase_ -= 1.0f;
        float lfo = std::sin(6.283185307f * lfoPhase_);

        // Modulated frequency
        float minFreq = centerFreq_ * 0.5f;
        float maxFreq = std::min(centerFreq_ * 2.0f, sampleRate * 0.45f);
        float modFreq = centerFreq_ + (maxFreq - minFreq) * 0.5f * lfo * depth_;
        modFreq = std::clamp(modFreq, minFreq, maxFreq);

        // Allpass coefficient
        float w = 6.283185307f * modFreq / sampleRate;
        float coeff = (1.0f - std::tan(w * 0.5f)) / (1.0f + std::tan(w * 0.5f));

        // Process allpass chain
        float sample = input + lastOutput_ * feedback_;
        for (int i = 0; i < stages_; ++i) {
            float temp = coeff * sample + apState_[i];
            apState_[i] = sample - coeff * temp;
            sample = temp;
        }
        lastOutput_ = sample;

        return input * 0.5f + sample * 0.5f;
    }

    void reset() {
        lfoPhase_ = 0.0f;
        lastOutput_ = 0.0f;
        apState_.fill(0.0f);
    }

  private:
    float rate_ = 0.5f;
    float depth_ = 0.7f;
    float centerFreq_ = 1000.0f;
    float feedback_ = 0.3f;
    int stages_ = 6;
    float lfoPhase_ = 0.0f;
    float lastOutput_ = 0.0f;
    std::array<float, 12> apState_ = {};
};

// Compressor — dynamic range compression
class Compressor {
  public:
    void setThreshold(float dB) { thresholdDb_ = std::clamp(dB, -60.0f, 0.0f); }
    void setRatio(float r) { ratio_ = std::max(1.0f, r); }
    void setAttack(float ms) { attackMs_ = std::max(0.1f, ms); }
    void setRelease(float ms) { releaseMs_ = std::max(1.0f, ms); }
    void setKnee(float dB) { kneeDb_ = std::max(0.0f, dB); }
    void setMakeupGain(float dB) { makeupDb_ = dB; }

    float process(float input, float sampleRate) {
        // Convert to dB
        float inputAbs = std::abs(input);
        float inputDb = (inputAbs > 1e-6f) ? 20.0f * std::log10(inputAbs) : -120.0f;

        // Gain computation with soft knee
        float gainDb = 0.0f;
        float diff = inputDb - thresholdDb_;
        if (kneeDb_ > 0.0f && std::abs(diff) < kneeDb_ * 0.5f) {
            // Soft knee region
            float x = diff + kneeDb_ * 0.5f;
            gainDb = -(1.0f - 1.0f / ratio_) * x * x / (2.0f * kneeDb_);
        } else if (diff > 0.0f) {
            gainDb = diff * (1.0f - 1.0f / ratio_);
            gainDb = -gainDb;
        }

        // Envelope follower (smooth gain reduction)
        float targetGain = gainDb;
        float coeff;
        if (targetGain < envelopeDb_) {
            coeff = std::exp(-1.0f / (attackMs_ * 0.001f * sampleRate));
        } else {
            coeff = std::exp(-1.0f / (releaseMs_ * 0.001f * sampleRate));
        }
        envelopeDb_ = coeff * envelopeDb_ + (1.0f - coeff) * targetGain;

        // Apply gain
        float totalGainDb = envelopeDb_ + makeupDb_;
        float gain = std::pow(10.0f, totalGainDb / 20.0f);
        return input * gain;
    }

    void reset() { envelopeDb_ = 0.0f; }

  private:
    float thresholdDb_ = -20.0f;
    float ratio_ = 4.0f;
    float attackMs_ = 5.0f;
    float releaseMs_ = 50.0f;
    float kneeDb_ = 6.0f;
    float makeupDb_ = 0.0f;
    float envelopeDb_ = 0.0f;
};

// Vowel Filter — formant filter using 3 parallel bandpass filters
class VowelFilter {
  public:
    enum Vowel { A, E, I, O, U };

    void setVowel(Vowel v) { vowel_ = v; dirty_ = true; }
    void setVowelMix(float mix) {
        // Interpolate between vowels (0.0=A, 0.25=E, 0.5=I, 0.75=O, 1.0=U)
        vowelMix_ = std::clamp(mix, 0.0f, 1.0f);
        dirty_ = true;
    }
    void setResonance(float q) { resonance_ = std::clamp(q, 1.0f, 20.0f); dirty_ = true; }

    float process(float input, float sampleRate) {
        if (dirty_) {
            recalculate(sampleRate);
            dirty_ = false;
        }

        float output = 0.0f;
        for (int i = 0; i < 3; ++i) {
            output += processBandpass(input, i);
        }
        return output * 0.33f;
    }

    void reset() {
        for (int i = 0; i < 3; ++i) {
            x1_[i] = x2_[i] = y1_[i] = y2_[i] = 0.0f;
        }
    }

  private:
    // Formant frequencies for each vowel [vowel][formant]
    static constexpr float formants_[5][3] = {
        {800.0f,  1150.0f, 2900.0f},  // A
        {350.0f,  2000.0f, 2800.0f},  // E
        {270.0f,  2140.0f, 3200.0f},  // I
        {450.0f,  800.0f,  2830.0f},  // O
        {325.0f,  700.0f,  2530.0f},  // U
    };

    void recalculate(float sampleRate) {
        // Get formant frequencies (direct or interpolated)
        float freqs[3];
        if (vowelMix_ < 0.0f) {
            // Direct vowel mode
            for (int i = 0; i < 3; ++i)
                freqs[i] = formants_[(int)vowel_][i];
        } else {
            float pos = vowelMix_ * 4.0f;
            int idx = std::min((int)pos, 3);
            float frac = pos - idx;
            for (int i = 0; i < 3; ++i)
                freqs[i] = formants_[idx][i] * (1.0f - frac) + formants_[idx + 1][i] * frac;
        }

        // Calculate bandpass coefficients for each formant
        for (int i = 0; i < 3; ++i) {
            float w0 = 6.283185307f * freqs[i] / sampleRate;
            float sinw0 = std::sin(w0);
            float alpha = sinw0 / (2.0f * resonance_);
            float a0 = 1.0f + alpha;
            a0_[i] = alpha / a0;
            a1_[i] = 0.0f;
            a2_[i] = -alpha / a0;
            b1_[i] = -2.0f * std::cos(w0) / a0;
            b2_[i] = (1.0f - alpha) / a0;
        }
    }

    float processBandpass(float input, int idx) {
        float output = a0_[idx] * input + a1_[idx] * x1_[idx] + a2_[idx] * x2_[idx]
                     - b1_[idx] * y1_[idx] - b2_[idx] * y2_[idx];
        x2_[idx] = x1_[idx]; x1_[idx] = input;
        y2_[idx] = y1_[idx]; y1_[idx] = output;
        return output;
    }

    Vowel vowel_ = A;
    float vowelMix_ = -1.0f;  // Negative = use discrete vowel
    float resonance_ = 5.0f;
    bool dirty_ = true;

    float a0_[3] = {}, a1_[3] = {}, a2_[3] = {};
    float b1_[3] = {}, b2_[3] = {};
    float x1_[3] = {}, x2_[3] = {}, y1_[3] = {}, y2_[3] = {};
};

// Ladder Filter — 4-pole resonant lowpass (Moog-style)
class LadderFilter {
  public:
    void setCutoff(float freq) { cutoff_ = std::clamp(freq, 20.0f, 20000.0f); }
    void setResonance(float r) { resonance_ = std::clamp(r, 0.0f, 1.1f); }  // >1 = self-oscillation
    void setDrive(float d) { drive_ = std::max(0.1f, d); }

    float process(float input, float sampleRate) {
        float fc = cutoff_ / sampleRate;
        float f = fc * 1.16f;
        float fb = resonance_ * 4.0f * (1.0f - 0.15f * f * f);

        // Drive / saturation at input
        input *= drive_;
        input -= stage_[3] * fb;
        input = std::tanh(input * 0.5f) * 2.0f;

        // 4 cascaded one-pole filters
        for (int i = 0; i < 4; ++i) {
            float prev = (i == 0) ? input : stage_[i - 1];
            stage_[i] += f * (std::tanh(prev) - std::tanh(stage_[i]));
        }

        return stage_[3];
    }

    void reset() { stage_.fill(0.0f); }

  private:
    float cutoff_ = 1000.0f;
    float resonance_ = 0.5f;
    float drive_ = 1.0f;
    std::array<float, 4> stage_ = {};
};

// Chorus — modulated delay for thickening
class Chorus {
  public:
    void init(float sampleRate) {
        sampleRate_ = sampleRate;
        int maxDelay = (int)(0.05f * sampleRate) + 1;  // 50ms max
        buffer_.resize(maxDelay, 0.0f);
        writePos_ = 0;
    }

    void setRate(float hz) { rate_ = std::max(0.01f, hz); }
    void setDepth(float d) { depth_ = std::clamp(d, 0.0f, 1.0f); }
    void setMix(float m) { mix_ = std::clamp(m, 0.0f, 1.0f); }
    void setVoices(int v) { voices_ = std::clamp(v, 1, 4); }

    float process(float input) {
        if (buffer_.empty()) return input;

        buffer_[writePos_] = input;

        float wet = 0.0f;
        for (int v = 0; v < voices_; ++v) {
            // Each voice has a phase offset
            float phase = lfoPhase_ + (float)v / (float)voices_;
            if (phase >= 1.0f) phase -= 1.0f;
            float lfo = std::sin(6.283185307f * phase);

            // Delay time: 7ms center + modulation
            float delayMs = 7.0f + lfo * depth_ * 5.0f;
            float delaySamples = delayMs * 0.001f * sampleRate_;

            // Read with linear interpolation
            float readPos = (float)writePos_ - delaySamples;
            if (readPos < 0.0f) readPos += (float)buffer_.size();
            int idx = (int)readPos;
            float frac = readPos - (float)idx;
            int next = (idx + 1) % (int)buffer_.size();

            wet += buffer_[idx] * (1.0f - frac) + buffer_[next] * frac;
        }
        wet /= (float)voices_;

        writePos_ = (writePos_ + 1) % (int)buffer_.size();
        lfoPhase_ += rate_ / sampleRate_;
        if (lfoPhase_ >= 1.0f) lfoPhase_ -= 1.0f;

        return input * (1.0f - mix_) + wet * mix_;
    }

    void reset() {
        std::fill(buffer_.begin(), buffer_.end(), 0.0f);
        lfoPhase_ = 0.0f;
    }

  private:
    std::vector<float> buffer_;
    int writePos_ = 0;
    float lfoPhase_ = 0.0f;
    float rate_ = 1.0f;
    float depth_ = 0.5f;
    float mix_ = 0.5f;
    int voices_ = 2;
    float sampleRate_ = 44100.0f;
};

// Bitcrusher — sample rate reduction + bit depth reduction
class Bitcrusher {
  public:
    void setBits(int b) { bits_ = std::clamp(b, 1, 16); }
    void setDownsample(int factor) { downsample_ = std::max(1, factor); }

    float process(float input) {
        // Sample rate reduction (sample-and-hold)
        counter_++;
        if (counter_ >= downsample_) {
            counter_ = 0;
            held_ = input;
        }

        // Bit depth reduction
        float scale = std::pow(2.0f, (float)(bits_ - 1));
        float crushed = std::round(held_ * scale) / scale;
        return crushed;
    }

    void reset() { counter_ = 0; held_ = 0.0f; }

  private:
    int bits_ = 8;
    int downsample_ = 4;
    int counter_ = 0;
    float held_ = 0.0f;
};

// Wavefolder — folds signal back on itself when exceeding threshold
class Wavefolder {
  public:
    void setDrive(float d) { drive_ = std::max(0.1f, d); }
    void setSymmetry(float s) { symmetry_ = std::clamp(s, -1.0f, 1.0f); }

    float process(float input) {
        // Apply drive and asymmetry offset
        float signal = input * drive_ + symmetry_ * 0.5f;

        // Multi-fold: repeatedly fold signal into [-1, 1]
        // Using triangle wave folding formula
        signal = signal * 0.25f + 0.25f;  // Scale to [0, 0.5] range for folding
        signal = signal - std::floor(signal);  // Wrap to [0, 1)
        signal = std::abs(signal * 4.0f - 2.0f) - 1.0f;  // Triangle fold

        // Remove DC offset from symmetry
        return signal;
    }

    void reset() {}  // Stateless effect

  private:
    float drive_ = 2.0f;
    float symmetry_ = 0.0f;
};

}  // namespace magda::daw::audio::tidal
