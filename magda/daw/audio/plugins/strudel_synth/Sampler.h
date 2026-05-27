#pragma once
#include "Envelope.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <cmath>
#include <algorithm>
#include <filesystem>

namespace magda::daw::audio::tidal {

// Holds loaded audio data (mono or stereo float arrays)
struct SampleBuffer {
    std::vector<std::vector<float>> channels;  // channels[ch][sample]
    float sampleRate = 44100.0f;
    int numChannels = 0;
    int numSamples = 0;

    bool isValid() const { return numSamples > 0 && numChannels > 0; }

    // Load from WAV file using JUCE AudioFormatManager
    bool loadFromFile(const std::string& path) {
        juce::File file(path);
        if (!file.existsAsFile()) return false;

        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        std::unique_ptr<juce::AudioFormatReader> reader(
            formatManager.createReaderFor(file));
        if (!reader) return false;

        numChannels = static_cast<int>(reader->numChannels);
        numSamples = static_cast<int>(reader->lengthInSamples);
        sampleRate = static_cast<float>(reader->sampleRate);

        juce::AudioBuffer<float> buffer(numChannels, numSamples);
        reader->read(&buffer, 0, numSamples, 0, true, true);

        channels.resize(numChannels);
        for (int ch = 0; ch < numChannels; ++ch) {
            channels[ch].assign(
                buffer.getReadPointer(ch),
                buffer.getReadPointer(ch) + numSamples);
        }
        return true;
    }

    // Get interpolated sample value at fractional position
    float getSample(int channel, float position) const {
        if (channel >= numChannels || numSamples == 0) return 0.0f;

        const auto& data = channels[channel];
        int idx0 = static_cast<int>(position);
        int idx1 = idx0 + 1;
        float frac = position - static_cast<float>(idx0);

        // Clamp indices
        idx0 = std::clamp(idx0, 0, numSamples - 1);
        idx1 = std::clamp(idx1, 0, numSamples - 1);

        return data[idx0] + frac * (data[idx1] - data[idx0]);
    }
};

// Plays back a SampleBuffer with variable speed, slicing, looping, and envelope
class SamplePlayer {
  public:
    enum LoopMode { OneShot, Looping };

    void setSample(const SampleBuffer* buf) { sample_ = buf; }
    void setSpeed(float speed) { speed_ = speed; }
    void setBegin(float b) { begin_ = std::clamp(b, 0.0f, 1.0f); }
    void setEnd(float e) { end_ = std::clamp(e, 0.0f, 1.0f); }
    void setLoopMode(LoopMode mode) { loopMode_ = mode; }
    void setADSR(float a, float d, float s, float r) { env_.setADSR(a, d, s, r); }

    void trigger(float velocity = 1.0f) {
        if (!sample_ || !sample_->isValid()) return;
        active_ = true;
        velocity_ = velocity;
        env_.trigger();

        // Compute start/end positions in samples
        float startNorm = (speed_ >= 0.0f) ? begin_ : end_;
        position_ = startNorm * static_cast<float>(sample_->numSamples);
    }

    void release() { env_.release(); }

    bool isActive() const { return active_; }

    // Render one sample frame (mono output, mixes channels)
    float render(float hostSampleRate) {
        if (!active_ || !sample_ || !sample_->isValid()) return 0.0f;

        float amp = env_.render(hostSampleRate);
        if (!env_.isActive()) { active_ = false; return 0.0f; }

        int numSamples = sample_->numSamples;
        float startPos = begin_ * static_cast<float>(numSamples);
        float endPos = end_ * static_cast<float>(numSamples);

        // Ensure begin < end for range calculation
        if (startPos > endPos) std::swap(startPos, endPos);
        float rangeLen = endPos - startPos;
        if (rangeLen < 1.0f) { active_ = false; return 0.0f; }

        // Read interpolated sample (mix all channels to mono)
        float out = 0.0f;
        for (int ch = 0; ch < sample_->numChannels; ++ch) {
            out += sample_->getSample(ch, position_);
        }
        out /= static_cast<float>(sample_->numChannels);

        // Advance position (adjust for sample rate difference)
        float rateRatio = sample_->sampleRate / hostSampleRate;
        position_ += speed_ * rateRatio;

        // Handle boundaries
        if (speed_ >= 0.0f) {
            if (position_ >= endPos) {
                if (loopMode_ == Looping)
                    position_ = startPos + std::fmod(position_ - startPos, rangeLen);
                else
                    { active_ = false; return out * amp * velocity_; }
            }
        } else {
            if (position_ < startPos) {
                if (loopMode_ == Looping)
                    position_ = endPos - std::fmod(endPos - position_, rangeLen);
                else
                    { active_ = false; return out * amp * velocity_; }
            }
        }

        return out * amp * velocity_;
    }

    void reset() {
        active_ = false;
        position_ = 0.0f;
    }

  private:
    const SampleBuffer* sample_ = nullptr;
    Envelope env_;
    float position_ = 0.0f;
    float speed_ = 1.0f;
    float begin_ = 0.0f;
    float end_ = 1.0f;
    float velocity_ = 1.0f;
    LoopMode loopMode_ = OneShot;
    bool active_ = false;
};

// Manages a collection of named samples with lazy loading
class SampleBank {
  public:
    // Register a directory of WAV files (does not load them yet)
    void loadDirectory(const std::string& dirPath) {
        namespace fs = std::filesystem;
        if (!fs::is_directory(dirPath)) return;

        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            // Case-insensitive WAV check
            if (ext == ".wav" || ext == ".WAV" || ext == ".Wav") {
                std::string name = entry.path().stem().string();
                registry_[name] = entry.path().string();
            }
        }
    }

    // Register a single sample path by name
    void registerSample(const std::string& name, const std::string& path) {
        registry_[name] = path;
    }

    // Lookup by name — loads on first access (lazy loading)
    const SampleBuffer* getSample(const std::string& name) {
        // Already loaded?
        auto it = loaded_.find(name);
        if (it != loaded_.end()) return it->second.get();

        // Registered but not loaded?
        auto reg = registry_.find(name);
        if (reg == registry_.end()) return nullptr;

        auto buf = std::make_unique<SampleBuffer>();
        if (!buf->loadFromFile(reg->second)) return nullptr;

        const SampleBuffer* ptr = buf.get();
        loaded_[name] = std::move(buf);
        return ptr;
    }

    bool hasSample(const std::string& name) const {
        return registry_.count(name) > 0 || loaded_.count(name) > 0;
    }

    std::vector<std::string> getNames() const {
        std::vector<std::string> names;
        names.reserve(registry_.size());
        for (const auto& [name, _] : registry_) names.push_back(name);
        return names;
    }

    void clear() {
        loaded_.clear();
        registry_.clear();
    }

  private:
    std::unordered_map<std::string, std::string> registry_;  // name -> file path
    std::unordered_map<std::string, std::unique_ptr<SampleBuffer>> loaded_;
};

}  // namespace magda::daw::audio::tidal
