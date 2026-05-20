#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

#include <functional>
#include <memory>

namespace magda {

/**
 * @brief Available time-stretching algorithms.
 */
enum class StretchAlgorithm {
    Basic,       ///< Simple OLA (Overlap-Add) — fast, lower quality
    Elastique,   ///< zplane Elastique — high quality, licensed
    RubberBand   ///< Rubber Band Library — open source, good quality
};

/**
 * @brief Configuration for a time-stretch or pitch-shift operation.
 */
struct StretchSettings {
    double ratio            = 1.0;    ///< Time stretch ratio (2.0 = double length)
    double pitchSemitones   = 0.0;    ///< Pitch shift in semitones
    bool preserveFormants   = false;  ///< Preserve vocal formants during pitch shift
    StretchAlgorithm algorithm = StretchAlgorithm::RubberBand;
};

/**
 * @brief Result of a stretch/pitch operation.
 */
struct StretchResult {
    bool success = false;
    juce::String errorMessage;
    int64_t outputLengthSamples = 0;
    double outputDuration = 0.0;  ///< Duration in seconds
};

/**
 * @brief Progress callback for offline stretch operations.
 * @param progress 0.0 to 1.0
 * @return false to cancel the operation.
 */
using StretchProgressCallback = std::function<bool(float progress)>;

/**
 * @brief Offline and real-time time stretching and pitch shifting engine.
 *
 * Provides file-based offline processing and buffer-based real-time
 * processing for time stretching and pitch shifting audio.
 */
class TimeStretchEngine {
  public:
    TimeStretchEngine();
    ~TimeStretchEngine();

    // --- Offline file processing ---

    /**
     * @brief Time-stretch an audio file.
     * @param inputFile Source audio file.
     * @param outputFile Destination file (will be created/overwritten).
     * @param ratio Time stretch ratio (>1 = longer, <1 = shorter).
     * @param algorithm The stretching algorithm to use.
     * @param progress Optional progress callback.
     * @return Result with success status and output info.
     */
    StretchResult stretchFile(const juce::File& inputFile,
                              const juce::File& outputFile,
                              double ratio,
                              StretchAlgorithm algorithm = StretchAlgorithm::RubberBand,
                              StretchProgressCallback progress = nullptr);

    /**
     * @brief Pitch-shift an audio file without changing duration.
     * @param inputFile Source audio file.
     * @param outputFile Destination file.
     * @param semitones Pitch shift amount in semitones.
     * @param progress Optional progress callback.
     * @return Result with success status and output info.
     */
    StretchResult pitchShiftFile(const juce::File& inputFile,
                                 const juce::File& outputFile,
                                 double semitones,
                                 StretchProgressCallback progress = nullptr);

    /**
     * @brief Apply combined time-stretch and pitch-shift.
     * @param inputFile Source audio file.
     * @param outputFile Destination file.
     * @param settings Full stretch/pitch configuration.
     * @param progress Optional progress callback.
     * @return Result with success status and output info.
     */
    StretchResult processFile(const juce::File& inputFile,
                              const juce::File& outputFile,
                              const StretchSettings& settings,
                              StretchProgressCallback progress = nullptr);

    // --- Real-time processing ---

    /**
     * @brief Prepare for real-time processing.
     * @param sampleRate The audio sample rate.
     * @param blockSize Maximum expected block size.
     * @param numChannels Number of audio channels.
     */
    void prepareRealtime(double sampleRate, int blockSize, int numChannels);

    /**
     * @brief Set real-time stretch ratio (can be changed per-block).
     */
    void setRealtimeRatio(double ratio);

    /**
     * @brief Set real-time pitch shift in semitones.
     */
    void setRealtimePitch(double semitones);

    /**
     * @brief Process a block of audio in real-time.
     * @param buffer Audio buffer to process in-place.
     */
    void processBlock(juce::AudioBuffer<float>& buffer);

    /** Reset internal state. */
    void reset();

    // --- Utilities ---

    /** Get the name of a stretch algorithm. */
    static juce::String getAlgorithmName(StretchAlgorithm algorithm);

    /** Check if an algorithm is available on this system. */
    static bool isAlgorithmAvailable(StretchAlgorithm algorithm);

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace magda
