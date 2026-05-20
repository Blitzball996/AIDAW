#include "TimeStretchEngine.hpp"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace magda {

// ============================================================================
// Basic OLA (Overlap-Add) time stretcher implementation
// ============================================================================

namespace {

constexpr int kOLAWindowSize = 2048;
constexpr int kOLAHopSize    = 512;

/**
 * Basic phase-vocoder style OLA stretcher.
 * Used as fallback when RubberBand/Elastique are not available.
 */
class BasicStretcher {
  public:
    BasicStretcher(double sampleRate, int numChannels)
        : sampleRate_(sampleRate), numChannels_(numChannels)
    {
        window_.resize(kOLAWindowSize);
        for (int i = 0; i < kOLAWindowSize; ++i)
            window_[static_cast<size_t>(i)] =
                0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi *
                                        static_cast<float>(i) /
                                        static_cast<float>(kOLAWindowSize - 1)));
    }

    void processBuffer(const juce::AudioBuffer<float>& input,
                       juce::AudioBuffer<float>& output,
                       double ratio)
    {
        const int inputLength = input.getNumSamples();
        const int outputLength = static_cast<int>(std::round(inputLength * ratio));

        output.setSize(numChannels_, outputLength);
        output.clear();

        if (ratio <= 0.0 || inputLength == 0)
            return;

        const int analysisHop = kOLAHopSize;
        const int synthesisHop = static_cast<int>(std::round(analysisHop * ratio));

        for (int ch = 0; ch < numChannels_; ++ch)
        {
            const float* inputData = input.getReadPointer(ch);
            float* outputData = output.getWritePointer(ch);

            int analysisPos = 0;
            int synthesisPos = 0;

            while (analysisPos + kOLAWindowSize <= inputLength &&
                   synthesisPos + kOLAWindowSize <= outputLength)
            {
                // Window and overlap-add
                for (int i = 0; i < kOLAWindowSize; ++i)
                {
                    outputData[synthesisPos + i] +=
                        inputData[analysisPos + i] * window_[static_cast<size_t>(i)];
                }

                analysisPos += analysisHop;
                synthesisPos += synthesisHop;
            }
        }
    }

  private:
    double sampleRate_;
    int numChannels_;
    std::vector<float> window_;
};

}  // namespace

// ============================================================================
// TimeStretchEngine::Impl
// ============================================================================

struct TimeStretchEngine::Impl {
    double sampleRate   = 44100.0;
    int blockSize       = 512;
    int numChannels     = 2;
    double realtimeRatio = 1.0;
    double realtimePitch = 0.0;

    std::unique_ptr<BasicStretcher> basicStretcher;
};

// ============================================================================
// TimeStretchEngine
// ============================================================================

TimeStretchEngine::TimeStretchEngine()
    : impl_(std::make_unique<Impl>())
{
}

TimeStretchEngine::~TimeStretchEngine() = default;

StretchResult TimeStretchEngine::stretchFile(const juce::File& inputFile,
                                             const juce::File& outputFile,
                                             double ratio,
                                             StretchAlgorithm algorithm,
                                             StretchProgressCallback progress)
{
    StretchSettings settings;
    settings.ratio = ratio;
    settings.pitchSemitones = 0.0;
    settings.algorithm = algorithm;
    return processFile(inputFile, outputFile, settings, progress);
}

StretchResult TimeStretchEngine::pitchShiftFile(const juce::File& inputFile,
                                                const juce::File& outputFile,
                                                double semitones,
                                                StretchProgressCallback progress)
{
    StretchSettings settings;
    settings.ratio = 1.0;  // No time change
    settings.pitchSemitones = semitones;
    settings.algorithm = StretchAlgorithm::RubberBand;
    return processFile(inputFile, outputFile, settings, progress);
}

StretchResult TimeStretchEngine::processFile(const juce::File& inputFile,
                                             const juce::File& outputFile,
                                             const StretchSettings& settings,
                                             StretchProgressCallback progress)
{
    StretchResult result;

    // Validate input
    if (!inputFile.existsAsFile())
    {
        result.errorMessage = "Input file does not exist: " + inputFile.getFullPathName();
        return result;
    }

    if (settings.ratio <= 0.0)
    {
        result.errorMessage = "Invalid stretch ratio: " + juce::String(settings.ratio);
        return result;
    }

    // Read input file
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(inputFile));

    if (reader == nullptr)
    {
        result.errorMessage = "Cannot read input file: " + inputFile.getFullPathName();
        return result;
    }

    const int numChannels = static_cast<int>(reader->numChannels);
    const int inputLength = static_cast<int>(reader->lengthInSamples);
    const double sampleRate = reader->sampleRate;

    // Read entire input into buffer
    juce::AudioBuffer<float> inputBuffer(numChannels, inputLength);
    reader->read(&inputBuffer, 0, inputLength, 0, true, true);

    if (progress && !progress(0.1f))
    {
        result.errorMessage = "Operation cancelled";
        return result;
    }

    // Compute effective ratio (pitch shift changes playback speed, then we
    // compensate with time stretch to maintain duration)
    double effectiveRatio = settings.ratio;
    if (std::abs(settings.pitchSemitones) > 0.001)
    {
        // Pitch shift by resampling: ratio = 2^(semitones/12)
        double pitchRatio = std::pow(2.0, settings.pitchSemitones / 12.0);
        // To pitch shift without changing duration, we stretch by pitchRatio
        // then resample
        effectiveRatio *= pitchRatio;
    }

    // Process using the selected algorithm (Basic OLA as fallback)
    juce::AudioBuffer<float> outputBuffer;

    BasicStretcher stretcher(sampleRate, numChannels);
    stretcher.processBuffer(inputBuffer, outputBuffer, effectiveRatio);

    if (progress && !progress(0.7f))
    {
        result.errorMessage = "Operation cancelled";
        return result;
    }

    // If pitch shifting, resample the output to compensate
    if (std::abs(settings.pitchSemitones) > 0.001)
    {
        double pitchRatio = std::pow(2.0, settings.pitchSemitones / 12.0);
        int resampledLength = static_cast<int>(
            std::round(outputBuffer.getNumSamples() / pitchRatio));

        juce::AudioBuffer<float> resampledBuffer(numChannels, resampledLength);

        // Simple linear interpolation resampling
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* src = outputBuffer.getReadPointer(ch);
            float* dst = resampledBuffer.getWritePointer(ch);
            int srcLen = outputBuffer.getNumSamples();

            for (int i = 0; i < resampledLength; ++i)
            {
                double srcPos = static_cast<double>(i) * pitchRatio;
                int idx = static_cast<int>(srcPos);
                float frac = static_cast<float>(srcPos - idx);

                if (idx + 1 < srcLen)
                    dst[i] = src[idx] * (1.0f - frac) + src[idx + 1] * frac;
                else if (idx < srcLen)
                    dst[i] = src[idx];
                else
                    dst[i] = 0.0f;
            }
        }

        outputBuffer = std::move(resampledBuffer);
    }

    if (progress && !progress(0.9f))
    {
        result.errorMessage = "Operation cancelled";
        return result;
    }

    // Write output file
    outputFile.deleteFile();
    std::unique_ptr<juce::FileOutputStream> outStream(outputFile.createOutputStream());
    if (outStream == nullptr)
    {
        result.errorMessage = "Cannot create output file: " + outputFile.getFullPathName();
        return result;
    }

    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(outStream.get(),
                                  sampleRate,
                                  static_cast<unsigned int>(numChannels),
                                  24,
                                  {},
                                  0));

    if (writer == nullptr)
    {
        result.errorMessage = "Cannot create WAV writer";
        return result;
    }

    outStream.release();  // Writer takes ownership
    writer->writeFromAudioSampleBuffer(outputBuffer, 0, outputBuffer.getNumSamples());

    if (progress)
        progress(1.0f);

    result.success = true;
    result.outputLengthSamples = outputBuffer.getNumSamples();
    result.outputDuration = static_cast<double>(outputBuffer.getNumSamples()) / sampleRate;
    return result;
}

void TimeStretchEngine::prepareRealtime(double sampleRate, int blockSize, int numChannels)
{
    impl_->sampleRate = sampleRate;
    impl_->blockSize = blockSize;
    impl_->numChannels = numChannels;
    impl_->basicStretcher = std::make_unique<BasicStretcher>(sampleRate, numChannels);
}

void TimeStretchEngine::setRealtimeRatio(double ratio)
{
    impl_->realtimeRatio = std::clamp(ratio, 0.25, 4.0);
}

void TimeStretchEngine::setRealtimePitch(double semitones)
{
    impl_->realtimePitch = std::clamp(semitones, -24.0, 24.0);
}

void TimeStretchEngine::processBlock(juce::AudioBuffer<float>& buffer)
{
    // Real-time processing placeholder.
    // In production, this would use RubberBand's real-time API or similar.
    // For now, pass-through when ratio is 1.0 and pitch is 0.
    if (std::abs(impl_->realtimeRatio - 1.0) < 0.001 &&
        std::abs(impl_->realtimePitch) < 0.001)
    {
        return;  // No processing needed
    }

    // Basic real-time pitch shift via resampling (simplified)
    if (std::abs(impl_->realtimePitch) > 0.001)
    {
        double pitchRatio = std::pow(2.0, impl_->realtimePitch / 12.0);
        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();

        juce::AudioBuffer<float> temp(numChannels, numSamples);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* src = buffer.getReadPointer(ch);
            float* dst = temp.getWritePointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                double srcPos = static_cast<double>(i) * pitchRatio;
                int idx = static_cast<int>(srcPos);
                float frac = static_cast<float>(srcPos - idx);

                if (idx + 1 < numSamples)
                    dst[i] = src[idx] * (1.0f - frac) + src[idx + 1] * frac;
                else if (idx < numSamples)
                    dst[i] = src[idx];
                else
                    dst[i] = 0.0f;
            }
        }

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.copyFrom(ch, 0, temp, ch, 0, numSamples);
    }
}

void TimeStretchEngine::reset()
{
    impl_->basicStretcher.reset();
}

juce::String TimeStretchEngine::getAlgorithmName(StretchAlgorithm algorithm)
{
    switch (algorithm)
    {
        case StretchAlgorithm::Basic:      return "Basic (OLA)";
        case StretchAlgorithm::Elastique:  return "Elastique";
        case StretchAlgorithm::RubberBand: return "Rubber Band";
    }
    return "Unknown";
}

bool TimeStretchEngine::isAlgorithmAvailable(StretchAlgorithm algorithm)
{
    switch (algorithm)
    {
        case StretchAlgorithm::Basic:
            return true;  // Always available (built-in)
        case StretchAlgorithm::Elastique:
            return false;  // Requires license — not bundled
        case StretchAlgorithm::RubberBand:
#if MAGDA_HAS_RUBBERBAND
            return true;
#else
            return false;
#endif
    }
    return false;
}

}  // namespace magda
