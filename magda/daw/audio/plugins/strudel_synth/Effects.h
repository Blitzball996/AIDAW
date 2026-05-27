#pragma once
#include <cmath>
#include <algorithm>
#include <cstring>

namespace magda::daw::audio::tidal {

// Feedback delay
class Delay {
  public:
    void init(float sampleRate, float maxDelaySeconds = 2.0f) {
        bufferSize_ = (int)(sampleRate * maxDelaySeconds) + 1;
        buffer_.resize(bufferSize_, 0.0f);
        writePos_ = 0;
        sampleRate_ = sampleRate;
    }

    void setTime(float seconds) { delaySamples_ = seconds * sampleRate_; }
    void setFeedback(float fb) { feedback_ = std::clamp(fb, 0.0f, 0.95f); }
    void setMix(float mix) { mix_ = std::clamp(mix, 0.0f, 1.0f); }

    float process(float input) {
        if (buffer_.empty()) return input;

        float readPos = writePos_ - delaySamples_;
        if (readPos < 0) readPos += bufferSize_;
        int idx = (int)readPos;
        float frac = readPos - idx;
        int next = (idx + 1) % bufferSize_;

        float delayed = buffer_[idx] * (1.0f - frac) + buffer_[next] * frac;
        buffer_[writePos_] = input + delayed * feedback_;
        writePos_ = (writePos_ + 1) % bufferSize_;

        return input * (1.0f - mix_) + delayed * mix_;
    }

    void reset() { std::fill(buffer_.begin(), buffer_.end(), 0.0f); }

  private:
    std::vector<float> buffer_;
    int bufferSize_ = 0;
    int writePos_ = 0;
    float delaySamples_ = 22050.0f;
    float feedback_ = 0.4f;
    float mix_ = 0.3f;
    float sampleRate_ = 44100.0f;
};

// Simple Schroeder reverb (4 comb + 2 allpass)
class Reverb {
  public:
    void init(float sampleRate) {
        for (int i = 0; i < 4; ++i) {
            int len = (int)(combLengths_[i] * sampleRate / 44100.0f);
            combs_[i].resize(len, 0.0f);
            combPos_[i] = 0;
        }
        for (int i = 0; i < 2; ++i) {
            int len = (int)(apLengths_[i] * sampleRate / 44100.0f);
            allpass_[i].resize(len, 0.0f);
            apPos_[i] = 0;
        }
    }

    void setRoomSize(float size) { roomSize_ = std::clamp(size, 0.0f, 1.0f); }
    void setDamping(float damp) { damping_ = std::clamp(damp, 0.0f, 1.0f); }
    void setMix(float mix) { mix_ = std::clamp(mix, 0.0f, 1.0f); }

    float process(float input) {
        float combOut = 0.0f;
        float fb = roomSize_ * 0.9f + 0.1f;

        for (int i = 0; i < 4; ++i) {
            if (combs_[i].empty()) continue;
            float delayed = combs_[i][combPos_[i]];
            combFilter_[i] = delayed * (1.0f - damping_) + combFilter_[i] * damping_;
            combs_[i][combPos_[i]] = input + combFilter_[i] * fb;
            combPos_[i] = (combPos_[i] + 1) % (int)combs_[i].size();
            combOut += delayed;
        }
        combOut *= 0.25f;

        // Allpass
        float out = combOut;
        for (int i = 0; i < 2; ++i) {
            if (allpass_[i].empty()) continue;
            float delayed = allpass_[i][apPos_[i]];
            float temp = out + delayed * 0.5f;
            allpass_[i][apPos_[i]] = temp;
            out = delayed - temp * 0.5f;
            apPos_[i] = (apPos_[i] + 1) % (int)allpass_[i].size();
        }

        return input * (1.0f - mix_) + out * mix_;
    }

    void reset() {
        for (auto& c : combs_) std::fill(c.begin(), c.end(), 0.0f);
        for (auto& a : allpass_) std::fill(a.begin(), a.end(), 0.0f);
        for (auto& f : combFilter_) f = 0.0f;
    }

  private:
    static constexpr int combLengths_[4] = {1116, 1188, 1277, 1356};
    static constexpr int apLengths_[2] = {556, 441};

    std::vector<float> combs_[4];
    std::vector<float> allpass_[2];
    int combPos_[4] = {};
    int apPos_[2] = {};
    float combFilter_[4] = {};
    float roomSize_ = 0.5f;
    float damping_ = 0.5f;
    float mix_ = 0.3f;
};

// Distortion with multiple algorithms
class Distortion {
  public:
    enum Algorithm { SoftClip, HardClip, Tanh, Fold, Bitcrush };

    void setAlgorithm(Algorithm a) { algo_ = a; }
    void setDrive(float d) { drive_ = std::max(0.1f, d); }
    void setMix(float m) { mix_ = std::clamp(m, 0.0f, 1.0f); }

    float process(float input) {
        float driven = input * drive_;
        float distorted;

        switch (algo_) {
            case SoftClip:
                distorted = driven / (1.0f + std::abs(driven));
                break;
            case HardClip:
                distorted = std::clamp(driven, -1.0f, 1.0f);
                break;
            case Tanh:
                distorted = std::tanh(driven);
                break;
            case Fold:
                distorted = std::sin(driven * 1.5707963f);
                break;
            case Bitcrush: {
                float bits = std::max(1.0f, 16.0f / drive_);
                float scale = std::pow(2.0f, bits);
                distorted = std::round(driven * scale) / scale;
                break;
            }
        }

        return input * (1.0f - mix_) + distorted * mix_;
    }

  private:
    Algorithm algo_ = Tanh;
    float drive_ = 2.0f;
    float mix_ = 1.0f;
};

}  // namespace magda::daw::audio::tidal
