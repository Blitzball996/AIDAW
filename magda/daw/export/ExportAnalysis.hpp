#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

namespace magda {

/**
 * @brief Results of audio analysis measurements
 */
struct AnalysisResult {
    float peakDb = -100.0f;
    float rmsDb = -100.0f;
    float lufs = -100.0f;
    float truePeakDb = -100.0f;
    float dynamicRange = 0.0f;
    bool clipping = false;
};

/**
 * @brief Provides audio analysis for export validation
 *
 * Measures peak, RMS, and LUFS loudness of audio files or buffers.
 */
class ExportAnalysis {
  public:
    ExportAnalysis() = default;
    ~ExportAnalysis() = default;

    /**
     * @brief Measure peak level of an audio file
     * @return Peak level in dB
     */
    float measurePeak(const juce::File& audioFile);

    /**
     * @brief Measure RMS level of an audio file
     * @return RMS level in dB
     */
    float measureRMS(const juce::File& audioFile);

    /**
     * @brief Measure integrated LUFS loudness of an audio file
     * @return Integrated loudness in LUFS
     */
    float measureLUFS(const juce::File& audioFile);

    /**
     * @brief Run full analysis on an audio file
     * @return Complete analysis results
     */
    AnalysisResult analyzeFile(const juce::File& audioFile);

    /**
     * @brief Measure peak level of an audio buffer
     */
    static float measurePeak(const juce::AudioBuffer<float>& buffer);

    /**
     * @brief Measure RMS level of an audio buffer
     */
    static float measureRMS(const juce::AudioBuffer<float>& buffer);

    /**
     * @brief Measure integrated LUFS of an audio buffer at a given sample rate
     *
     * Implements a simplified ITU-R BS.1770 measurement.
     */
    static float measureLUFS(const juce::AudioBuffer<float>& buffer,
                             double sampleRate);

  private:
    juce::AudioFormatManager formatManager_;
    bool formatManagerInitialized_ = false;

    void ensureFormatsRegistered();
    std::unique_ptr<juce::AudioFormatReader> createReader(
        const juce::File& file);
};

}  // namespace magda
