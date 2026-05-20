#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

#include <vector>

namespace magda {

/**
 * @brief A region of audio defined by start and end times in seconds
 */
struct AudioRegion {
    double startTime = 0.0;
    double endTime = 0.0;
};

/**
 * @brief Offline audio processing operations
 *
 * All operations are non-destructive: they produce new files rather than
 * modifying the source. The caller is responsible for managing the output
 * files (e.g., placing them in the project pool).
 */
class AudioOperations {
  public:
    AudioOperations();
    ~AudioOperations();

    /**
     * @brief Detect non-silent regions in an audio file
     * @param file Source audio file
     * @param thresholdDb Silence threshold in dB (e.g., -60.0)
     * @param minDurationMs Minimum silence duration to count as a gap (ms)
     * @return Vector of non-silent regions (start/end in seconds)
     */
    std::vector<AudioRegion> stripSilence(const juce::File& file,
                                          float thresholdDb,
                                          double minDurationMs);

    /**
     * @brief Normalize an audio file to a target peak level
     * @param file Source audio file
     * @param targetDb Target peak level in dB (e.g., -0.3)
     * @return Path to the normalized output file, or empty on failure
     */
    juce::String normalize(const juce::File& file, float targetDb);

    /**
     * @brief Reverse an audio file
     * @param file Source audio file
     * @return Path to the reversed output file, or empty on failure
     */
    juce::String reverse(const juce::File& file);

    /**
     * @brief Apply a fade-in to the beginning of an audio file
     * @param file Source audio file
     * @param durationMs Fade duration in milliseconds
     * @return Path to the output file, or empty on failure
     */
    juce::String fadeIn(const juce::File& file, double durationMs);

    /**
     * @brief Apply a fade-out to the end of an audio file
     * @param file Source audio file
     * @param durationMs Fade duration in milliseconds
     * @return Path to the output file, or empty on failure
     */
    juce::String fadeOut(const juce::File& file, double durationMs);

  private:
    juce::AudioFormatManager formatManager_;
    bool formatManagerInitialized_ = false;

    void ensureFormatsRegistered();
    std::unique_ptr<juce::AudioFormatReader> createReader(
        const juce::File& file);
    juce::File generateOutputPath(const juce::File& source,
                                  const juce::String& suffix);
};

}  // namespace magda
