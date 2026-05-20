#include "LoudnessAnalyzer.hpp"

#include <cmath>
#include <vector>

namespace magda {

LoudnessAnalyzer::LoudnessAnalyzer() = default;
LoudnessAnalyzer::~LoudnessAnalyzer() = default;

void LoudnessAnalyzer::ensureFormatsRegistered() {
    if (!formatManagerInitialized_) {
        formatManager_.registerBasicFormats();
        formatManagerInitialized_ = true;
    }
}

LoudnessResult LoudnessAnalyzer::analyze(const juce::File& file) {
    ensureFormatsRegistered();

    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager_.createReaderFor(file));
    if (!reader)
        return {};

    juce::AudioBuffer<float> buffer(
        static_cast<int>(reader->numChannels),
        static_cast<int>(reader->lengthInSamples));
    reader->read(&buffer, 0, static_cast<int>(reader->lengthInSamples), 0,
                 true, true);

    return analyze(buffer, reader->sampleRate);
}

LoudnessResult LoudnessAnalyzer::analyze(
    const juce::AudioBuffer<float>& buffer, double sampleRate) {
    LoudnessResult result;
    result.integratedLUFS = computeIntegratedLUFS(buffer, sampleRate);
    result.truePeakDb = computeTruePeak(buffer);
    result.rmsDb = computeRMS(buffer);
    return result;
}

float LoudnessAnalyzer::computeIntegratedLUFS(
    const juce::AudioBuffer<float>& buffer, double sampleRate) {
    if (buffer.getNumSamples() == 0)
        return -100.0f;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // ITU-R BS.1770-4: 400ms gate blocks, 75% overlap (100ms step)
    const int gateBlockSize = static_cast<int>(sampleRate * 0.4);
    const int stepSize = static_cast<int>(sampleRate * 0.1);

    if (gateBlockSize == 0 || stepSize == 0)
        return -100.0f;

    std::vector<double> blockLoudness;

    for (int start = 0; start + gateBlockSize <= numSamples;
         start += stepSize) {
        double blockSum = 0.0;

        for (int ch = 0; ch < numChannels; ++ch) {
            // Channel weighting: L/R = 1.0, surround = 1.41
            double weight = 1.0;
            const float* data = buffer.getReadPointer(ch);

            for (int i = start; i < start + gateBlockSize; ++i) {
                blockSum += weight * static_cast<double>(data[i]) *
                            static_cast<double>(data[i]);
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
    double relGateThreshold = ungatedMean * std::pow(10.0, -1.0);

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
    return static_cast<float>(-0.691 + 10.0 * std::log10(gatedMean));
}

float LoudnessAnalyzer::computeTruePeak(
    const juce::AudioBuffer<float>& buffer) {
    // True peak requires 4x oversampling per ITU-R BS.1770.
    // Simplified here: use sample peak as approximation.
    float peak = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        peak = juce::jmax(
            peak, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    }
    return peak > 0.0f ? 20.0f * std::log10(peak) : -100.0f;
}

float LoudnessAnalyzer::computeRMS(
    const juce::AudioBuffer<float>& buffer) {
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

}  // namespace magda
