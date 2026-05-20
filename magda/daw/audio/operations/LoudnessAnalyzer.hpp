#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

namespace magda {

/**
 * @brief Results of a loudness analysis
 */
struct LoudnessResult {
    float integratedLUFS = -100.0f;  ///< ITU-R BS.1770 integrated loudness
    float truePeakDb = -100.0f;      ///< True peak level in dBTP
    float rmsDb = -100.0f;           ///< RMS level in dB
};

/**
 * @brief Analyzes loudness of audio files per ITU-R BS.1770
 *
 * Provides integrated LUFS, true peak, and RMS measurements.
 */
class LoudnessAnalyzer {
  public:
    LoudnessAnalyzer();
    ~LoudnessAnalyzer();

    /**
     * @brief Analyze an audio file for loudness metrics
     * @param file Audio file to analyze
     * @return Loudness measurements
     */
    LoudnessResult analyze(const juce::File& file);

    /**
     * @brief Analyze an audio buffer for loudness metrics
     * @param buffer Audio buffer to analyze
     * @param sampleRate Sample rate of the buffer
     * @return Loudness measurements
     */
    LoudnessResult analyze(const juce::AudioBuffer<float>& buffer,
                           double sampleRate);

  private:
    juce::AudioFormatManager formatManager_;
    bool formatManagerInitialized_ = false;

    void ensureFormatsRegistered();

    float computeIntegratedLUFS(const juce::AudioBuffer<float>& buffer,
                                double sampleRate);
    float computeTruePeak(const juce::AudioBuffer<float>& buffer);
    float computeRMS(const juce::AudioBuffer<float>& buffer);
};

}  // namespace magda
