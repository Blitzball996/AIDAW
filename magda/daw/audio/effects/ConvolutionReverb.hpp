#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#include <memory>

namespace magda {

/**
 * @brief Metadata about a loaded impulse response.
 */
struct IRInfo {
    juce::String name;        ///< Display name of the IR
    juce::String path;        ///< File path to the IR file
    double duration = 0.0;    ///< Duration in seconds
    double sampleRate = 0.0;  ///< Sample rate of the IR file
    int channels = 0;         ///< Number of channels in the IR
};

/**
 * @brief IR-based convolution reverb processor.
 *
 * Uses JUCE's dsp::Convolution for efficient partitioned convolution.
 * Supports loading impulse responses from file, with dry/wet mix,
 * pre-delay, and decay factor controls.
 */
class ConvolutionReverb {
  public:
    ConvolutionReverb();
    ~ConvolutionReverb();

    // --- Impulse Response ---

    /**
     * @brief Load an impulse response from a file.
     * @param file Path to the IR audio file (WAV, AIFF, FLAC, etc.)
     * @return true if loaded successfully.
     */
    bool loadImpulseResponse(const juce::File& file);

    /**
     * @brief Load an impulse response from a buffer.
     * @param buffer The IR audio data.
     * @param sampleRate The sample rate of the IR data.
     * @param name Display name for the IR.
     */
    void loadImpulseResponse(const juce::AudioBuffer<float>& buffer,
                             double sampleRate,
                             const juce::String& name = "Custom IR");

    /** Get info about the currently loaded IR. */
    IRInfo getIRInfo() const { return irInfo_; }

    /** Check if an IR is currently loaded. */
    bool isIRLoaded() const { return irLoaded_; }

    // --- Parameters ---

    /** Set dry/wet mix (0.0 = fully dry, 1.0 = fully wet). */
    void setDryWet(float mix);

    /** Get the current dry/wet mix. */
    float getDryWet() const { return dryWet_; }

    /** Set pre-delay in milliseconds. */
    void setPreDelay(float ms);

    /** Get the current pre-delay in milliseconds. */
    float getPreDelay() const { return preDelayMs_; }

    /**
     * @brief Set the decay factor (trims the IR tail).
     * @param factor 0.0 to 1.0 — multiplied into the IR envelope.
     *              1.0 = full IR length, 0.5 = half decay time.
     */
    void setDecay(float factor);

    /** Get the current decay factor. */
    float getDecay() const { return decayFactor_; }

    // --- Processing ---

    /** Prepare the processor for playback. */
    void prepare(const juce::dsp::ProcessSpec& spec);

    /** Process an audio buffer in-place. */
    void process(juce::AudioBuffer<float>& buffer);

    /** Reset internal state (clear delay lines, etc.). */
    void reset();

  private:
    void updatePreDelay();

    juce::dsp::Convolution convolution_;
    juce::dsp::DelayLine<float> preDelay_{ 192000 };  // Max 4s at 48kHz

    IRInfo irInfo_;
    bool irLoaded_ = false;

    float dryWet_      = 1.0f;
    float preDelayMs_  = 0.0f;
    float decayFactor_ = 1.0f;

    double currentSampleRate_ = 44100.0;
    int currentBlockSize_     = 512;

    juce::AudioBuffer<float> dryBuffer_;
};

}  // namespace magda
