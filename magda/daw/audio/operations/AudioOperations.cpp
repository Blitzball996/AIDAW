#include "AudioOperations.hpp"

#include <cmath>

namespace magda {

AudioOperations::AudioOperations() = default;
AudioOperations::~AudioOperations() = default;

void AudioOperations::ensureFormatsRegistered() {
    if (!formatManagerInitialized_) {
        formatManager_.registerBasicFormats();
        formatManagerInitialized_ = true;
    }
}

std::unique_ptr<juce::AudioFormatReader> AudioOperations::createReader(
    const juce::File& file) {
    ensureFormatsRegistered();
    return std::unique_ptr<juce::AudioFormatReader>(
        formatManager_.createReaderFor(file));
}

juce::File AudioOperations::generateOutputPath(const juce::File& source,
                                               const juce::String& suffix) {
    auto dir = source.getParentDirectory();
    auto baseName = source.getFileNameWithoutExtension();
    auto ext = source.getFileExtension();
    return dir.getChildFile(baseName + suffix + ext);
}

std::vector<AudioRegion> AudioOperations::stripSilence(
    const juce::File& file, float thresholdDb, double minDurationMs) {
    std::vector<AudioRegion> regions;

    auto reader = createReader(file);
    if (!reader)
        return regions;

    const double sampleRate = reader->sampleRate;
    const float threshold = std::pow(10.0f, thresholdDb / 20.0f);
    const int minSilenceSamples =
        static_cast<int>(sampleRate * minDurationMs / 1000.0);

    const int blockSize = 4096;
    juce::AudioBuffer<float> buffer(
        static_cast<int>(reader->numChannels), blockSize);

    bool inRegion = false;
    int silenceCount = 0;
    juce::int64 regionStart = 0;
    juce::int64 position = 0;
    juce::int64 samplesRemaining = reader->lengthInSamples;

    while (samplesRemaining > 0) {
        int samplesToRead = static_cast<int>(
            juce::jmin(static_cast<juce::int64>(blockSize), samplesRemaining));
        reader->read(&buffer, 0, samplesToRead, position, true, true);

        for (int i = 0; i < samplesToRead; ++i) {
            float maxSample = 0.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                maxSample = juce::jmax(
                    maxSample, std::abs(buffer.getSample(ch, i)));
            }

            bool isSilent = maxSample < threshold;

            if (!isSilent) {
                if (!inRegion) {
                    inRegion = true;
                    regionStart = position + i;
                }
                silenceCount = 0;
            } else {
                if (inRegion) {
                    ++silenceCount;
                    if (silenceCount >= minSilenceSamples) {
                        AudioRegion region;
                        region.startTime =
                            static_cast<double>(regionStart) / sampleRate;
                        region.endTime =
                            static_cast<double>(position + i - silenceCount) /
                            sampleRate;
                        if (region.endTime > region.startTime)
                            regions.push_back(region);
                        inRegion = false;
                        silenceCount = 0;
                    }
                }
            }
        }

        position += samplesToRead;
        samplesRemaining -= samplesToRead;
    }

    // Close final region if still open
    if (inRegion) {
        AudioRegion region;
        region.startTime = static_cast<double>(regionStart) / sampleRate;
        region.endTime =
            static_cast<double>(reader->lengthInSamples) / sampleRate;
        if (region.endTime > region.startTime)
            regions.push_back(region);
    }

    return regions;
}

juce::String AudioOperations::normalize(const juce::File& file,
                                        float targetDb) {
    auto reader = createReader(file);
    if (!reader)
        return {};

    // First pass: find peak
    const int blockSize = 8192;
    juce::AudioBuffer<float> buffer(
        static_cast<int>(reader->numChannels), blockSize);

    float peak = 0.0f;
    juce::int64 samplesRemaining = reader->lengthInSamples;
    juce::int64 position = 0;

    while (samplesRemaining > 0) {
        int samplesToRead = static_cast<int>(
            juce::jmin(static_cast<juce::int64>(blockSize), samplesRemaining));
        reader->read(&buffer, 0, samplesToRead, position, true, true);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            peak = juce::jmax(peak, buffer.getMagnitude(ch, 0, samplesToRead));
        }

        position += samplesToRead;
        samplesRemaining -= samplesToRead;
    }

    if (peak <= 0.0f)
        return {};

    float targetLinear = std::pow(10.0f, targetDb / 20.0f);
    float gain = targetLinear / peak;

    // Second pass: write normalized audio
    auto outputFile = generateOutputPath(file, "_normalized");
    juce::WavAudioFormat wavFormat;
    auto stream = outputFile.createOutputStream();
    if (!stream)
        return {};

    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(
            stream.release(), reader->sampleRate,
            static_cast<unsigned int>(reader->numChannels),
            static_cast<int>(reader->bitsPerSample), {}, 0));
    if (!writer)
        return {};

    // Re-read from start
    auto reader2 = createReader(file);
    if (!reader2)
        return {};

    samplesRemaining = reader2->lengthInSamples;
    position = 0;

    while (samplesRemaining > 0) {
        int samplesToRead = static_cast<int>(
            juce::jmin(static_cast<juce::int64>(blockSize), samplesRemaining));
        reader2->read(&buffer, 0, samplesToRead, position, true, true);

        buffer.applyGain(0, samplesToRead, gain);
        writer->writeFromAudioSampleBuffer(buffer, 0, samplesToRead);

        position += samplesToRead;
        samplesRemaining -= samplesToRead;
    }

    return outputFile.getFullPathName();
}

juce::String AudioOperations::reverse(const juce::File& file) {
    auto reader = createReader(file);
    if (!reader)
        return {};

    // Read entire file
    juce::AudioBuffer<float> buffer(
        static_cast<int>(reader->numChannels),
        static_cast<int>(reader->lengthInSamples));
    reader->read(&buffer, 0, static_cast<int>(reader->lengthInSamples), 0,
                 true, true);

    // Reverse in place
    buffer.reverse(0, buffer.getNumSamples());

    // Write output
    auto outputFile = generateOutputPath(file, "_reversed");
    juce::WavAudioFormat wavFormat;
    auto stream = outputFile.createOutputStream();
    if (!stream)
        return {};

    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(
            stream.release(), reader->sampleRate,
            static_cast<unsigned int>(reader->numChannels),
            static_cast<int>(reader->bitsPerSample), {}, 0));
    if (!writer)
        return {};

    writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    return outputFile.getFullPathName();
}

juce::String AudioOperations::fadeIn(const juce::File& file,
                                     double durationMs) {
    auto reader = createReader(file);
    if (!reader)
        return {};

    juce::AudioBuffer<float> buffer(
        static_cast<int>(reader->numChannels),
        static_cast<int>(reader->lengthInSamples));
    reader->read(&buffer, 0, static_cast<int>(reader->lengthInSamples), 0,
                 true, true);

    int fadeSamples = static_cast<int>(
        reader->sampleRate * durationMs / 1000.0);
    fadeSamples = juce::jmin(fadeSamples, buffer.getNumSamples());

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < fadeSamples; ++i) {
            float gain = static_cast<float>(i) / static_cast<float>(fadeSamples);
            data[i] *= gain;
        }
    }

    auto outputFile = generateOutputPath(file, "_fadein");
    juce::WavAudioFormat wavFormat;
    auto stream = outputFile.createOutputStream();
    if (!stream)
        return {};

    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(
            stream.release(), reader->sampleRate,
            static_cast<unsigned int>(reader->numChannels),
            static_cast<int>(reader->bitsPerSample), {}, 0));
    if (!writer)
        return {};

    writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    return outputFile.getFullPathName();
}

juce::String AudioOperations::fadeOut(const juce::File& file,
                                      double durationMs) {
    auto reader = createReader(file);
    if (!reader)
        return {};

    juce::AudioBuffer<float> buffer(
        static_cast<int>(reader->numChannels),
        static_cast<int>(reader->lengthInSamples));
    reader->read(&buffer, 0, static_cast<int>(reader->lengthInSamples), 0,
                 true, true);

    int fadeSamples = static_cast<int>(
        reader->sampleRate * durationMs / 1000.0);
    fadeSamples = juce::jmin(fadeSamples, buffer.getNumSamples());

    int fadeStart = buffer.getNumSamples() - fadeSamples;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < fadeSamples; ++i) {
            float gain = 1.0f - (static_cast<float>(i) /
                                 static_cast<float>(fadeSamples));
            data[fadeStart + i] *= gain;
        }
    }

    auto outputFile = generateOutputPath(file, "_fadeout");
    juce::WavAudioFormat wavFormat;
    auto stream = outputFile.createOutputStream();
    if (!stream)
        return {};

    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(
            stream.release(), reader->sampleRate,
            static_cast<unsigned int>(reader->numChannels),
            static_cast<int>(reader->bitsPerSample), {}, 0));
    if (!writer)
        return {};

    writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    return outputFile.getFullPathName();
}

}  // namespace magda
