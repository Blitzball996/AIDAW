#pragma once
#include <cmath>
#include <algorithm>

namespace magda::daw::audio::tidal {

// Biquad filter (LP, HP, BP, Notch)
class Filter {
  public:
    enum Type { Lowpass, Highpass, Bandpass, Notch };

    void setType(Type t) { type_ = t; dirty_ = true; }
    void setCutoff(float freq) { cutoff_ = std::clamp(freq, 20.0f, 20000.0f); dirty_ = true; }
    void setResonance(float q) { resonance_ = std::clamp(q, 0.1f, 20.0f); dirty_ = true; }

    float process(float input, float sampleRate) {
        if (dirty_) {
            recalculate(sampleRate);
            dirty_ = false;
        }

        float output = a0_ * input + a1_ * x1_ + a2_ * x2_ - b1_ * y1_ - b2_ * y2_;
        x2_ = x1_; x1_ = input;
        y2_ = y1_; y1_ = output;
        return output;
    }

    void reset() { x1_ = x2_ = y1_ = y2_ = 0.0f; }

  private:
    void recalculate(float sampleRate) {
        float w0 = 6.283185307f * cutoff_ / sampleRate;
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.0f * resonance_);

        float a0, a1, a2, b0, b1, b2;
        switch (type_) {
            case Lowpass:
                b0 = (1.0f - cosw0) / 2.0f;
                b1 = 1.0f - cosw0;
                b2 = (1.0f - cosw0) / 2.0f;
                a0 = 1.0f + alpha;
                a1 = -2.0f * cosw0;
                a2 = 1.0f - alpha;
                break;
            case Highpass:
                b0 = (1.0f + cosw0) / 2.0f;
                b1 = -(1.0f + cosw0);
                b2 = (1.0f + cosw0) / 2.0f;
                a0 = 1.0f + alpha;
                a1 = -2.0f * cosw0;
                a2 = 1.0f - alpha;
                break;
            case Bandpass:
                b0 = alpha;
                b1 = 0.0f;
                b2 = -alpha;
                a0 = 1.0f + alpha;
                a1 = -2.0f * cosw0;
                a2 = 1.0f - alpha;
                break;
            case Notch:
                b0 = 1.0f;
                b1 = -2.0f * cosw0;
                b2 = 1.0f;
                a0 = 1.0f + alpha;
                a1 = -2.0f * cosw0;
                a2 = 1.0f - alpha;
                break;
        }

        a0_ = b0 / a0; a1_ = b1 / a0; a2_ = b2 / a0;
        b1_ = a1 / a0; b2_ = a2 / a0;
    }

    Type type_ = Lowpass;
    float cutoff_ = 10000.0f;
    float resonance_ = 0.707f;
    bool dirty_ = true;

    float a0_ = 1, a1_ = 0, a2_ = 0, b1_ = 0, b2_ = 0;
    float x1_ = 0, x2_ = 0, y1_ = 0, y2_ = 0;
};

}  // namespace magda::daw::audio::tidal
