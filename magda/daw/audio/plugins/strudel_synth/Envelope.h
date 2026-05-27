#pragma once
#include <cmath>
#include <algorithm>

namespace magda::daw::audio::tidal {

// ADSR Envelope with linear/exponential curves
class Envelope {
  public:
    enum Stage { Idle, Attack, Decay, Sustain, Release };

    void setADSR(float a, float d, float s, float r) {
        attack_ = std::max(0.001f, a);
        decay_ = std::max(0.001f, d);
        sustain_ = std::clamp(s, 0.0f, 1.0f);
        release_ = std::max(0.001f, r);
    }

    void trigger() {
        stage_ = Attack;
        level_ = 0.0f;
    }

    void release() {
        if (stage_ != Idle)
            stage_ = Release;
    }

    bool isActive() const { return stage_ != Idle; }
    Stage getStage() const { return stage_; }

    float render(float sampleRate) {
        switch (stage_) {
            case Attack:
                level_ += 1.0f / (attack_ * sampleRate);
                if (level_ >= 1.0f) { level_ = 1.0f; stage_ = Decay; }
                break;
            case Decay:
                level_ -= (1.0f - sustain_) / (decay_ * sampleRate);
                if (level_ <= sustain_) { level_ = sustain_; stage_ = Sustain; }
                break;
            case Sustain:
                level_ = sustain_;
                break;
            case Release:
                level_ -= level_ / (release_ * sampleRate + 1.0f);
                if (level_ < 0.001f) { level_ = 0.0f; stage_ = Idle; }
                break;
            case Idle:
                level_ = 0.0f;
                break;
        }
        return level_;
    }

  private:
    Stage stage_ = Idle;
    float level_ = 0.0f;
    float attack_ = 0.005f;
    float decay_ = 0.1f;
    float sustain_ = 0.7f;
    float release_ = 0.2f;
};

}  // namespace magda::daw::audio::tidal
