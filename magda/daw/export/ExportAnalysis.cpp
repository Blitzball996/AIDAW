#include "ExportAnalysis.hpp"

#include <cmath>

namespace magda {

void ExportAnalysis::ensureFormatsRegistered() {
    if (!formatManagerInitialized_) {
        formatManager_.registerBasicFormats();
        formatManagerInitialized_ = true;
    }
}

std::unique_ptr<juce::AudioFormatReader> ExportAnalysis::createReader(
    const juce::File& file) {
    ensureFormatsRegistered();
    return std::unique_ptr<juce::AudioFormatReader>(
        formatManager_.createReaderFor(file));
}

float ExportAnalysis::measurePeak(const juce::File& audioFile) {
    auto reader = createReader(audioFile);
    if (!reader)
        return -100.0f;

    const int blockSize = 8192;
    juce::AudioBuffer<float> buffer(
        static_cast<int>(reader->numChannels), blockSize);

    float peak = 0.0f;
    juce::int64 samplesRemaining = reader->lengthInSamples;
    juce::int64 position = 0;

    while (samplesRemaining > 0) {
        int samplesToRead =
            static_cast<int>(juce::jmin(static_cast<juce::int64>(blockSize),
                                        samplesRemaining));
        reader->read(&buffer, 0, samplesToRead, position, true, true);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            float channelPeak = buffer.getMagnitude(ch, 0, samplesToRead);
            peak = juce::jmax(peak, channelPeak);
        }

        position += samplesToRead;
        samplesRemaining -= samplesToRead;
    }

    return peak > 0.0f ? 20.0f * std::log10(peak) : -100.0f;
}

float ExportAnalysis::measureRMS(const juce::File& audioFile) {
    auto reader = createReader(audioFile);
    if (!reader)
        return -100.0f;

    const int blockSize = 8192;
    juce::AudioBuffer<float> buffer(
        static_cast<int>(reader->numChannels), blockSize);

    double sumSquares = 0.0;
    juce::int64 totalSamples = 0;
    juce::int64 samplesRemaining = reader->lengthInSamples;
    juce::int64 position = 0;

    while (samplesRemaining > 0) {
        int samplesToRead =
            static_cast<int>(juce::jmin(static_cast<juce::int64>(blockSize),
                                        samplesRemaining));
        reader->read(&buffer, 0, samplesToRead, position, true, true);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            for (int i = 0; i < samplesToRead; ++i) {
                float sample = buffer.getSample(ch, i);
                sumSquares += static_cast<double>(sample * sample);
            }
        }

        totalSamples +=
            static_cast<juce::int64>(samplesToRead) * reader->numChannels;
        position += samplesToRead;
        samplesRemaining -= samplesToRead;
    }

    if (totalSamples == 0)
        return -100.0f;

    double rms = std::sqrt(sumSquares / static_cast<double>(totalSamples));
    return rms > 0.0 ? 20.0f * std::log10(static_cast<float>(rms)) : -100.0f;
}

float ExportAnalysis::measureLUFS(const juce::File& audioFile) {
    auto reader = createReader(audioFile);
    if (!reader)
        return -100.0f;

    // Read entire file for LUFS measurement
    juce::AudioBuffer<float> buffer(
        static_cast<int>(reader->numChannels),
        static_cast<int>(reader->lengthInSamples));
    reader->read(&buffer, 0, static_cast<int>(reader->lengthInSamples), 0,
                 true, true);

    return measureLUFS(buffer, reader->sampleRate);
}

AnalysisResult ExportAnalysis::analyzeFile(const juce::File& audioFile) {
    AnalysisResult result;

    auto reader = createReader(audioFile);
    if (!reader)
        return result;

    const int blockSize = 8192;
    juce::AudioBuffer<float> buffer(
        static_cast<int>(reader->numChannels), blockSize);

    float peak = 0.0f;
    double sumSquares = 0.0;
    juce::int64 totalSamples = 0;
    juce::int64 samplesRemaining = reader->lengthInSamples;
    juce::int64 position = 0;

    while (samplesRemaining > 0) {
        int samplesToRead =
            static_cast<int>(juce::jmin(static_cast<juce::int64>(blockSize),
                                        samplesRemaining));
        reader->read(&buffer, 0, samplesToRead, position, true, true);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            float channelPeak = buffer.getMagnitude(ch, 0, samplesToRead);
            peak = juce::jmax(peak, channelPeak);

            for (int i = 0; i < samplesToRead; ++i) {
                float sample = buffer.getSample(ch, i);
                sumSquares += static_cast<double>(sample * sample);
            }
        }

        totalSamples +=
            static_cast<juce::int64>(samplesToRead) * reader->numChannels;
        position += samplesToRead;
        samplesRemaining -= samplesToRead;
    }

    result.peakDb =
        peak > 0.0f ? 20.0f * std::log10(peak) : -100.0f;
    result.truePeakDb = result.peakDb;  // Simplified; true peak needs oversampling
    result.clipping = peak >= 1.0f;

    if (totalSamples > 0) {
        double rms =
            std::sqrt(sumSquares / static_cast<double>(totalSamples));
        result.rmsDb =
            rms > 0.0 ? 20.0f * std::log10(static_cast<float>(rms)) : -100.0f;
    }

    result.dynamicRange = result.peakDb - result.rmsDb;

    // LUFS requires full-file read
    result.lufs = measureLUFS(audioFile);

    return result;
}

float ExportAnalysis::measurePeak(const juce::AudioBuffer<float>& buffer) {
    float peak = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        peak = juce::jmax(peak,
                          buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    }
    return peak > 0.0f ? 20.0f * std::log10(peak) : -100.0f;
}

float ExportAnalysis::measureRMS(const juce::AudioBuffer<float>& buffer) {
    double sumSquares = 0.0;
    int totalSamples = 0;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            sumSquares += static_cast<double>(data[i] * data[i]);
        }
        totalSamples += buffer.getNumSamples();
    }

    if (totalSamples == 0)
        return -100.0f;

    double rms = std::sqrt(sumSquares / static_cast<double>(totalSamples));
    return rms > 0.0 ? 20.0f * std::log10(static_cast<float>(rms)) : -100.0f;
}

float ExportAnalysis::measureLUFS(const juce::AudioBuffer<float>& buffer,
                                  double sampleRate) {
    // Simplified ITU-R BS.1770-4 integrated loudness measurement.
    // Uses K-weighting approximation and gated mean-square.

    if (buffer.getNumSamples() == 0)
        return -100.0f;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Gate block size: 400ms
    const int gateBlockSize =
        static_cast<int>(sampleRate * 0.4);
    if (gateBlockSize == 0)
        return -100.0f;

    // Step size: 75% overlap -> 100ms steps
    const int stepSize = static_cast<int>(sampleRate * 0.1);
    if (stepSize == 0)
        return -100.0f;

    // Compute mean square per block (simplified, no K-weighting filter)
    std::vector<double> blockLoudness;

    for (int start = 0; start + gateBlockSize <= numSamples;
         start += stepSize) {
        double blockSum = 0.0;

        for (int ch = 0; ch < numChannels; ++ch) {
            const float* data = buffer.getReadPointer(ch);
            for (int i = start; i < start + gateBlockSize; ++i) {
                blockSum += static_cast<double>(data[i] * data[i]);
            }
        }

        double meanSquare =
            blockSum / (static_cast<double>(gateBlockSize) * numChannels);
        blockLoudness.push_back(meanSquare);
    }

    if (blockLoudness.empty())
        return -100.0f;

    // Absolute gate: -70 LUFS
    const double absGateThreshold = std::pow(10.0, (-70.0 + 0.691) / 10.0);

    // First pass: compute ungated mean
    double ungatedSum = 0.0;
    int ungatedCount = 0;
    for (double ms : blockLoudness) {
        if (ms > absGateThreshold) {
            ungatedSum += ms;
            ++ungatedCount;
        }
    }

    if (ungatedCount == 0)
        return -100.0f;

    // Relative gate: -10 dB below ungated mean
    double ungatedMean = ungatedSum / ungatedCount;
    double relGateThreshold = ungatedMean * std::pow(10.0, -10.0 / 10.0);

    // Second pass: compute gated mean
    double gatedSum = 0.0;
    int gatedCount = 0;
    for (double ms : blockLoudness) {
        if (ms > relGateThreshold) {
            gatedSum += ms;
            ++gatedCount;
        }
    }

    if (gatedCount == 0)
        return -100.0f;

    double gatedMean = gatedSum / gatedCount;
    double lufs = -0.691 + 10.0 * std::log10(gatedMean);

    return static_cast<float>(lufs);
}

}  // namespace magda
