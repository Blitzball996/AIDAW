#include "ConvolutionReverb.hpp"

#include <juce_audio_formats/juce_audio_formats.h>

#include <algorithm>
#include <cmath>

namespace magda {

ConvolutionReverb::ConvolutionReverb()
{
}

ConvolutionReverb::~ConvolutionReverb() = default;

bool ConvolutionReverb::loadImpulseResponse(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(file));

    if (reader == nullptr)
        return false;

    // Read the IR into a buffer
    juce::AudioBuffer<float> irBuffer(
        static_cast<int>(reader->numChannels),
        static_cast<int>(reader->lengthInSamples));

    reader->read(&irBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

    // Store IR info
    irInfo_.name       = file.getFileNameWithoutExtension();
    irInfo_.path       = file.getFullPathName();
    irInfo_.duration   = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
    irInfo_.sampleRate = reader->sampleRate;
    irInfo_.channels   = static_cast<int>(reader->numChannels);

    // Load into the convolution engine
    convolution_.loadImpulseResponse(std::move(irBuffer),
                                     reader->sampleRate,
                                     juce::dsp::Convolution::Stereo::yes,
                                     juce::dsp::Convolution::Trim::yes,
                                     juce::dsp::Convolution::Normalise::yes);

    irLoaded_ = true;
    return true;
}

void ConvolutionReverb::loadImpulseResponse(const juce::AudioBuffer<float>& buffer,
                                            double sampleRate,
                                            const juce::String& name)
{
    irInfo_.name       = name;
    irInfo_.path       = "";
    irInfo_.duration   = static_cast<double>(buffer.getNumSamples()) / sampleRate;
    irInfo_.sampleRate = sampleRate;
    irInfo_.channels   = buffer.getNumChannels();

    // Copy the buffer (Convolution takes ownership via move)
    juce::AudioBuffer<float> irCopy(buffer);

    convolution_.loadImpulseResponse(std::move(irCopy),
                                     sampleRate,
                                     juce::dsp::Convolution::Stereo::yes,
                                     juce::dsp::Convolution::Trim::yes,
                                     juce::dsp::Convolution::Normalise::yes);

    irLoaded_ = true;
}

void ConvolutionReverb::setDryWet(float mix)
{
    dryWet_ = std::clamp(mix, 0.0f, 1.0f);
}

void ConvolutionReverb::setPreDelay(float ms)
{
    preDelayMs_ = std::clamp(ms, 0.0f, 500.0f);
    updatePreDelay();
}

void ConvolutionReverb::setDecay(float factor)
{
    decayFactor_ = std::clamp(factor, 0.0f, 1.0f);
}

void ConvolutionReverb::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSampleRate_ = spec.sampleRate;
    currentBlockSize_  = static_cast<int>(spec.maximumBlockSize);

    convolution_.prepare(spec);

    // Prepare pre-delay
    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;
    preDelay_.prepare(monoSpec);
    updatePreDelay();

    // Allocate dry buffer
    dryBuffer_.setSize(static_cast<int>(spec.numChannels),
                       static_cast<int>(spec.maximumBlockSize));
}

void ConvolutionReverb::process(juce::AudioBuffer<float>& buffer)
{
    if (!irLoaded_)
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    // Save dry signal for mix
    dryBuffer_.setSize(numChannels, numSamples, false, false, true);
    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer_.copyFrom(ch, 0, buffer, ch, 0, numSamples);

    // Apply pre-delay to the input before convolution
    if (preDelayMs_ > 0.0f)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
            {
                float delayed = preDelay_.popSample(ch);
                preDelay_.pushSample(ch, data[i]);
                data[i] = delayed;
            }
        }
    }

    // Process through convolution engine
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    convolution_.process(context);

    // Apply decay factor (simple amplitude envelope on wet signal)
    if (decayFactor_ < 1.0f)
    {
        buffer.applyGain(decayFactor_);
    }

    // Mix dry/wet
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* wetData = buffer.getWritePointer(ch);
        const float* dryData = dryBuffer_.getReadPointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            wetData[i] = dryData[i] * (1.0f - dryWet_) + wetData[i] * dryWet_;
        }
    }
}

void ConvolutionReverb::reset()
{
    convolution_.reset();
    preDelay_.reset();
}

void ConvolutionReverb::updatePreDelay()
{
    float delaySamples = static_cast<float>(preDelayMs_ * 0.001 * currentSampleRate_);
    preDelay_.setDelay(delaySamples);
}

}  // namespace magda
