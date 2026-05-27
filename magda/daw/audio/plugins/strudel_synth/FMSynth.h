#pragma once
#include <cmath>
#include <algorithm>
#include <cstring>
#include <array>

namespace magda::daw::audio::tidal {

// ============================================================================
// FM Synthesis Engine — 4 Operator with Modulation Matrix
// ============================================================================

// FM Algorithm presets (inspired by classic FM synths)
enum class FMAlgorithm {
    Serial,      // 4 -> 3 -> 2 -> 1 (out)
    Parallel,    // all operators output directly
    TwoSerial,   // (4->3) + (2->1), both to output
    ThreeToOne,  // 4,3,2 all modulate 1 (out)
    Mixed,       // 4->3->2, 4->1, both 2 and 1 to output
    Feedback     // 4->3->2->1 with op4 self-feedback
};

// Single FM Operator: oscillator + envelope + ratio
class FMOperator {
  public:
    enum Waveform { Sine, Saw, Square };

    void init(float sampleRate) { sampleRate_ = sampleRate; }

    void setRatio(float ratio) { ratio_ = std::max(0.01f, ratio); }
    void setModIndex(float idx) { modIndex_ = std::max(0.0f, idx); }
    void setWaveform(Waveform w) { waveform_ = w; }
    void setLevel(float level) { level_ = std::clamp(level, 0.0f, 1.0f); }

    void setADSR(float a, float d, float s, float r) {
        attack_ = std::max(0.001f, a);
        decay_ = std::max(0.001f, d);
        sustain_ = std::clamp(s, 0.0f, 1.0f);
        release_ = std::max(0.001f, r);
    }

    void trigger() {
        envStage_ = Attack;
        envLevel_ = 0.0f;
    }

    void releaseNote() {
        if (envStage_ != Idle)
            envStage_ = Release;
    }

    bool isActive() const { return envStage_ != Idle; }

    // Render one sample given base frequency and phase modulation input
    float render(float baseFreq, float phaseModulation) {
        // Advance envelope
        float env = renderEnvelope();

        // Calculate operator frequency
        float freq = baseFreq * ratio_;

        // Advance phase with modulation
        phase_ += freq / sampleRate_;
        if (phase_ >= 1.0f) phase_ -= 1.0f;
        if (phase_ < 0.0f) phase_ += 1.0f;

        float p = phase_ + phaseModulation * modIndex_;
        // Wrap phase
        p -= std::floor(p);

        float out;
        switch (waveform_) {
            case Sine:   out = std::sin(p * 6.283185307f); break;
            case Saw:    out = 2.0f * p - 1.0f; break;
            case Square: out = p < 0.5f ? 1.0f : -1.0f; break;
            default:     out = std::sin(p * 6.283185307f); break;
        }

        return out * env * level_;
    }

    void reset() {
        phase_ = 0.0f;
        envLevel_ = 0.0f;
        envStage_ = Idle;
    }

  private:
    enum EnvStage { Idle, Attack, Decay, Sustain, Release };

    float renderEnvelope() {
        switch (envStage_) {
            case Attack:
                envLevel_ += 1.0f / (attack_ * sampleRate_);
                if (envLevel_ >= 1.0f) { envLevel_ = 1.0f; envStage_ = Decay; }
                break;
            case Decay:
                envLevel_ -= (1.0f - sustain_) / (decay_ * sampleRate_);
                if (envLevel_ <= sustain_) { envLevel_ = sustain_; envStage_ = Sustain; }
                break;
            case Sustain:
                envLevel_ = sustain_;
                break;
            case Release:
                envLevel_ -= envLevel_ / (release_ * sampleRate_ + 1.0f);
                if (envLevel_ < 0.001f) { envLevel_ = 0.0f; envStage_ = Idle; }
                break;
            case Idle:
                envLevel_ = 0.0f;
                break;
        }
        return envLevel_;
    }

    float sampleRate_ = 44100.0f;
    Waveform waveform_ = Sine;
    float ratio_ = 1.0f;
    float modIndex_ = 1.0f;
    float level_ = 1.0f;
    float phase_ = 0.0f;

    // Envelope
    EnvStage envStage_ = Idle;
    float envLevel_ = 0.0f;
    float attack_ = 0.005f;
    float decay_ = 0.1f;
    float sustain_ = 0.7f;
    float release_ = 0.3f;
};

// 4-Operator FM Synthesizer with modulation matrix
class FMSynth {
  public:
    static constexpr int NumOperators = 4;

    void init(float sampleRate) {
        sampleRate_ = sampleRate;
        for (auto& op : ops_) op.init(sampleRate);
        // Default: serial algorithm, reasonable ratios
        ops_[0].setRatio(1.0f);
        ops_[1].setRatio(2.0f);
        ops_[2].setRatio(3.0f);
        ops_[3].setRatio(4.0f);
        setAlgorithm(FMAlgorithm::Serial);
    }

    FMOperator& getOperator(int idx) { return ops_[std::clamp(idx, 0, 3)]; }

    void setAlgorithm(FMAlgorithm algo) {
        algo_ = algo;
        // Clear modulation matrix
        for (auto& row : modMatrix_)
            for (auto& v : row) v = 0.0f;

        switch (algo) {
            case FMAlgorithm::Serial:
                // 4->3->2->1(out)
                modMatrix_[3][2] = 1.0f;  // op4 modulates op3
                modMatrix_[2][1] = 1.0f;  // op3 modulates op2
                modMatrix_[1][0] = 1.0f;  // op2 modulates op1
                outputMask_ = {1.0f, 0.0f, 0.0f, 0.0f};
                break;
            case FMAlgorithm::Parallel:
                // All to output
                outputMask_ = {1.0f, 1.0f, 1.0f, 1.0f};
                break;
            case FMAlgorithm::TwoSerial:
                // (4->3) + (2->1)
                modMatrix_[3][2] = 1.0f;
                modMatrix_[1][0] = 1.0f;
                outputMask_ = {1.0f, 0.0f, 1.0f, 0.0f};
                break;
            case FMAlgorithm::ThreeToOne:
                // 4,3,2 all modulate 1
                modMatrix_[3][0] = 1.0f;
                modMatrix_[2][0] = 1.0f;
                modMatrix_[1][0] = 1.0f;
                outputMask_ = {1.0f, 0.0f, 0.0f, 0.0f};
                break;
            case FMAlgorithm::Mixed:
                // 4->3->2(out), 4->1(out)
                modMatrix_[3][2] = 1.0f;
                modMatrix_[2][1] = 1.0f;
                modMatrix_[3][0] = 0.5f;
                outputMask_ = {1.0f, 1.0f, 0.0f, 0.0f};
                break;
            case FMAlgorithm::Feedback:
                // 4(self-fb)->3->2->1(out)
                modMatrix_[3][2] = 1.0f;
                modMatrix_[2][1] = 1.0f;
                modMatrix_[1][0] = 1.0f;
                selfFeedback_ = 0.5f;
                outputMask_ = {1.0f, 0.0f, 0.0f, 0.0f};
                break;
        }
    }

    // Direct modulation matrix access: source modulates dest
    void setModulation(int source, int dest, float amount) {
        if (source >= 0 && source < 4 && dest >= 0 && dest < 4)
            modMatrix_[source][dest] = amount;
    }

    void setSelfFeedback(float fb) { selfFeedback_ = std::clamp(fb, 0.0f, 1.0f); }

    void trigger(float frequency) {
        frequency_ = frequency;
        for (auto& op : ops_) op.trigger();
        lastOutput_[3] = 0.0f;
    }

    void releaseNote() {
        for (auto& op : ops_) op.releaseNote();
    }

    bool isActive() const {
        for (int i = 0; i < NumOperators; ++i)
            if (ops_[i].isActive() && outputMask_[i] > 0.0f) return true;
        return false;
    }

    float render() {
        // Render operators from top (4) to bottom (1)
        // Collect outputs first, then apply modulation
        float outputs[NumOperators] = {};

        // Op4 (index 3) — may have self-feedback
        float op4Mod = lastOutput_[3] * selfFeedback_;
        outputs[3] = ops_[3].render(frequency_, op4Mod);

        // Op3 (index 2)
        float mod2 = 0.0f;
        for (int src = 0; src < NumOperators; ++src)
            if (modMatrix_[src][2] != 0.0f)
                mod2 += outputs[src] * modMatrix_[src][2];
        outputs[2] = ops_[2].render(frequency_, mod2);

        // Op2 (index 1)
        float mod1 = 0.0f;
        for (int src = 0; src < NumOperators; ++src)
            if (modMatrix_[src][1] != 0.0f)
                mod1 += outputs[src] * modMatrix_[src][1];
        outputs[1] = ops_[1].render(frequency_, mod1);

        // Op1 (index 0) — carrier
        float mod0 = 0.0f;
        for (int src = 0; src < NumOperators; ++src)
            if (modMatrix_[src][0] != 0.0f)
                mod0 += outputs[src] * modMatrix_[src][0];
        outputs[0] = ops_[0].render(frequency_, mod0);

        // Store for feedback
        lastOutput_[3] = outputs[3];

        // Mix to output based on output mask
        float out = 0.0f;
        float maskSum = 0.0f;
        for (int i = 0; i < NumOperators; ++i) {
            out += outputs[i] * outputMask_[i];
            maskSum += outputMask_[i];
        }
        if (maskSum > 1.0f) out /= maskSum;

        return out;
    }

    void reset() {
        for (auto& op : ops_) op.reset();
        lastOutput_[3] = 0.0f;
    }

  private:
    float sampleRate_ = 44100.0f;
    float frequency_ = 440.0f;
    FMAlgorithm algo_ = FMAlgorithm::Serial;
    FMOperator ops_[NumOperators];
    float modMatrix_[NumOperators][NumOperators] = {};  // [source][dest]
    std::array<float, NumOperators> outputMask_ = {1.0f, 0.0f, 0.0f, 0.0f};
    float selfFeedback_ = 0.0f;
    float lastOutput_[NumOperators] = {};
};

// ============================================================================
// Drum Synthesizer — Kick, Snare, HiHat, Tom, Clap
// ============================================================================

// Kick Drum: sine with pitch envelope + noise burst
class KickDrum {
  public:
    void init(float sampleRate) { sampleRate_ = sampleRate; }

    void setStartFreq(float f) { startFreq_ = std::max(50.0f, f); }
    void setEndFreq(float f) { endFreq_ = std::max(20.0f, f); }
    void setPitchDecay(float d) { pitchDecay_ = std::max(0.001f, d); }
    void setAmpDecay(float d) { ampDecay_ = std::max(0.01f, d); }
    void setNoiseAmount(float n) { noiseAmount_ = std::clamp(n, 0.0f, 1.0f); }
    void setDrive(float d) { drive_ = std::max(1.0f, d); }

    void trigger() {
        phase_ = 0.0f;
        time_ = 0.0f;
        active_ = true;
    }

    bool isActive() const { return active_; }

    float render() {
        if (!active_) return 0.0f;

        float dt = 1.0f / sampleRate_;
        time_ += dt;

        // Pitch envelope: exponential decay from startFreq to endFreq
        float pitchEnv = std::exp(-time_ / pitchDecay_);
        float freq = endFreq_ + (startFreq_ - endFreq_) * pitchEnv;

        // Amplitude envelope: exponential decay
        float ampEnv = std::exp(-time_ / ampDecay_);

        // Sine body
        phase_ += freq / sampleRate_;
        if (phase_ >= 1.0f) phase_ -= 1.0f;
        float body = std::sin(phase_ * 6.283185307f);

        // Noise burst (very short)
        float noiseEnv = std::exp(-time_ / 0.005f);
        float noise = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * noiseEnv;

        // Mix and apply drive
        float out = body + noise * noiseAmount_;
        out *= ampEnv;
        out = std::tanh(out * drive_) / drive_;  // Soft saturation

        if (ampEnv < 0.001f) active_ = false;
        return out;
    }

    void reset() { active_ = false; phase_ = 0.0f; time_ = 0.0f; }

  private:
    float sampleRate_ = 44100.0f;
    float startFreq_ = 300.0f;
    float endFreq_ = 45.0f;
    float pitchDecay_ = 0.03f;
    float ampDecay_ = 0.4f;
    float noiseAmount_ = 0.3f;
    float drive_ = 2.0f;
    float phase_ = 0.0f;
    float time_ = 0.0f;
    bool active_ = false;
};

// Snare Drum: bandpass-filtered noise + sine body
class SnareDrum {
  public:
    void init(float sampleRate) { sampleRate_ = sampleRate; }

    void setBodyFreq(float f) { bodyFreq_ = std::max(80.0f, f); }
    void setNoiseDecay(float d) { noiseDecay_ = std::max(0.01f, d); }
    void setBodyDecay(float d) { bodyDecay_ = std::max(0.01f, d); }
    void setNoiseTone(float t) { noiseTone_ = std::clamp(t, 0.0f, 1.0f); }
    void setMix(float m) { mix_ = std::clamp(m, 0.0f, 1.0f); }

    void trigger() {
        phase_ = 0.0f;
        time_ = 0.0f;
        active_ = true;
        bpState_[0] = bpState_[1] = 0.0f;
    }

    bool isActive() const { return active_; }

    float render() {
        if (!active_) return 0.0f;

        float dt = 1.0f / sampleRate_;
        time_ += dt;

        // Sine body with fast decay
        float bodyEnv = std::exp(-time_ / bodyDecay_);
        phase_ += bodyFreq_ / sampleRate_;
        if (phase_ >= 1.0f) phase_ -= 1.0f;
        float body = std::sin(phase_ * 6.283185307f) * bodyEnv;

        // Noise with bandpass filter
        float noiseEnv = std::exp(-time_ / noiseDecay_);
        float noise = (float)rand() / RAND_MAX * 2.0f - 1.0f;

        // Simple 2-pole bandpass (resonant)
        float cutoff = 2000.0f + noiseTone_ * 6000.0f;
        float w = 2.0f * 3.14159265f * cutoff / sampleRate_;
        float q = 1.5f;
        float alpha = std::sin(w) / (2.0f * q);
        float a0 = 1.0f + alpha;
        float b0 = alpha / a0;
        float a1 = -2.0f * std::cos(w) / a0;
        float a2 = (1.0f - alpha) / a0;

        float filtered = b0 * noise - a1 * bpState_[0] - a2 * bpState_[1];
        bpState_[1] = bpState_[0];
        bpState_[0] = filtered;

        float noiseOut = filtered * noiseEnv;

        // Mix body and noise
        float out = body * (1.0f - mix_) + noiseOut * mix_;

        if (bodyEnv < 0.001f && noiseEnv < 0.001f) active_ = false;
        return out;
    }

    void reset() { active_ = false; phase_ = 0.0f; time_ = 0.0f; }

  private:
    float sampleRate_ = 44100.0f;
    float bodyFreq_ = 180.0f;
    float noiseDecay_ = 0.15f;
    float bodyDecay_ = 0.08f;
    float noiseTone_ = 0.5f;
    float mix_ = 0.6f;
    float phase_ = 0.0f;
    float time_ = 0.0f;
    float bpState_[2] = {};
    bool active_ = false;
};

// Hi-Hat: high-passed noise with very short envelope
class HiHat {
  public:
    void init(float sampleRate) { sampleRate_ = sampleRate; }

    void setDecay(float d) { decay_ = std::max(0.005f, d); }
    void setTone(float t) { tone_ = std::clamp(t, 0.0f, 1.0f); }
    void setOpen(bool open) { open_ = open; }

    void trigger() {
        time_ = 0.0f;
        active_ = true;
        hpState_ = 0.0f;
    }

    bool isActive() const { return active_; }

    float render() {
        if (!active_) return 0.0f;

        float dt = 1.0f / sampleRate_;
        time_ += dt;

        // Envelope: short for closed, longer for open
        float envTime = open_ ? decay_ * 4.0f : decay_;
        float env = std::exp(-time_ / envTime);

        // Metallic noise (sum of high-frequency square waves for metallic character)
        float noise = 0.0f;
        for (int i = 0; i < 6; ++i) {
            float freq = 800.0f + i * 1340.0f + tone_ * 2000.0f;
            metalPhases_[i] += freq / sampleRate_;
            if (metalPhases_[i] >= 1.0f) metalPhases_[i] -= 1.0f;
            noise += (metalPhases_[i] < 0.5f ? 1.0f : -1.0f);
        }
        noise /= 6.0f;

        // High-pass filter
        float cutoff = 6000.0f + tone_ * 6000.0f;
        float rc = 1.0f / (2.0f * 3.14159265f * cutoff);
        float alpha = rc / (rc + dt);
        float hpOut = alpha * (hpState_ + noise - prevInput_);
        prevInput_ = noise;
        hpState_ = hpOut;

        float out = hpOut * env;

        if (env < 0.001f) active_ = false;
        return out;
    }

    void reset() {
        active_ = false;
        time_ = 0.0f;
        hpState_ = 0.0f;
        prevInput_ = 0.0f;
        for (auto& p : metalPhases_) p = 0.0f;
    }

  private:
    float sampleRate_ = 44100.0f;
    float decay_ = 0.03f;
    float tone_ = 0.5f;
    bool open_ = false;
    float time_ = 0.0f;
    float hpState_ = 0.0f;
    float prevInput_ = 0.0f;
    float metalPhases_[6] = {};
    bool active_ = false;
};

// Tom: sine with pitch envelope (slower than kick)
class TomDrum {
  public:
    void init(float sampleRate) { sampleRate_ = sampleRate; }

    void setFrequency(float f) { baseFreq_ = std::max(40.0f, f); }
    void setPitchRange(float semitones) { pitchRange_ = std::max(0.0f, semitones); }
    void setPitchDecay(float d) { pitchDecay_ = std::max(0.01f, d); }
    void setAmpDecay(float d) { ampDecay_ = std::max(0.05f, d); }

    void trigger() {
        phase_ = 0.0f;
        time_ = 0.0f;
        active_ = true;
    }

    bool isActive() const { return active_; }

    float render() {
        if (!active_) return 0.0f;

        float dt = 1.0f / sampleRate_;
        time_ += dt;

        // Pitch envelope: slower than kick, sweeps down by pitchRange semitones
        float pitchEnv = std::exp(-time_ / pitchDecay_);
        float freqMult = std::pow(2.0f, pitchRange_ / 12.0f * pitchEnv);
        float freq = baseFreq_ * freqMult;

        // Amplitude envelope
        float ampEnv = std::exp(-time_ / ampDecay_);

        // Sine oscillator
        phase_ += freq / sampleRate_;
        if (phase_ >= 1.0f) phase_ -= 1.0f;
        float out = std::sin(phase_ * 6.283185307f) * ampEnv;

        if (ampEnv < 0.001f) active_ = false;
        return out;
    }

    void reset() { active_ = false; phase_ = 0.0f; time_ = 0.0f; }

  private:
    float sampleRate_ = 44100.0f;
    float baseFreq_ = 120.0f;
    float pitchRange_ = 7.0f;   // semitones
    float pitchDecay_ = 0.08f;
    float ampDecay_ = 0.3f;
    float phase_ = 0.0f;
    float time_ = 0.0f;
    bool active_ = false;
};

// Clap: filtered noise with double-trigger envelope
class ClapDrum {
  public:
    void init(float sampleRate) { sampleRate_ = sampleRate; }

    void setDecay(float d) { decay_ = std::max(0.05f, d); }
    void setTone(float t) { tone_ = std::clamp(t, 0.0f, 1.0f); }
    void setSpread(float s) { spread_ = std::clamp(s, 0.001f, 0.03f); }
    void setNumClaps(int n) { numClaps_ = std::clamp(n, 2, 5); }

    void trigger() {
        time_ = 0.0f;
        active_ = true;
        bpState_[0] = bpState_[1] = 0.0f;
    }

    bool isActive() const { return active_; }

    float render() {
        if (!active_) return 0.0f;

        float dt = 1.0f / sampleRate_;
        time_ += dt;

        // Double/multi-trigger envelope: multiple short bursts then decay
        float env = 0.0f;
        for (int i = 0; i < numClaps_; ++i) {
            float offset = i * spread_;
            float localTime = time_ - offset;
            if (localTime > 0.0f) {
                float burst = std::exp(-localTime / 0.005f);
                env += burst;
            }
        }
        // Overall decay
        float overallEnv = std::exp(-time_ / decay_);
        env *= overallEnv / numClaps_;

        // Noise source
        float noise = (float)rand() / RAND_MAX * 2.0f - 1.0f;

        // Bandpass filter for tonal character
        float cutoff = 1000.0f + tone_ * 3000.0f;
        float w = 2.0f * 3.14159265f * cutoff / sampleRate_;
        float q = 2.0f;
        float alpha = std::sin(w) / (2.0f * q);
        float a0 = 1.0f + alpha;
        float b0 = alpha / a0;
        float a1 = -2.0f * std::cos(w) / a0;
        float a2 = (1.0f - alpha) / a0;

        float filtered = b0 * noise - a1 * bpState_[0] - a2 * bpState_[1];
        bpState_[1] = bpState_[0];
        bpState_[0] = filtered;

        float out = filtered * env;

        if (overallEnv < 0.001f) active_ = false;
        return out;
    }

    void reset() { active_ = false; time_ = 0.0f; }

  private:
    float sampleRate_ = 44100.0f;
    float decay_ = 0.2f;
    float tone_ = 0.5f;
    float spread_ = 0.012f;
    int numClaps_ = 3;
    float time_ = 0.0f;
    float bpState_[2] = {};
    bool active_ = false;
};

}  // namespace magda::daw::audio::tidal

