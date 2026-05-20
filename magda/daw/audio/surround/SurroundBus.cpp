#include "SurroundBus.hpp"

#include <algorithm>

namespace magda {

SurroundBus::SurroundBus(SurroundFormat format)
    : format_(format)
{
}

void SurroundBus::setFormat(SurroundFormat format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    format_ = format;

    // Update all panners to the new format
    for (auto& input : inputs_)
    {
        if (input.panner)
            input.panner->setFormat(format);
    }
}

SpeakerLayoutInfo SurroundBus::getSpeakerLayout() const
{
    return SpeakerLayoutInfo::create(format_);
}

int SurroundBus::addInput(const juce::String& name)
{
    auto panner = std::make_unique<SurroundPanner>(format_);
    return addInput(name, std::move(panner));
}

int SurroundBus::addInput(const juce::String& name, std::unique_ptr<SurroundPanner> panner)
{
    std::lock_guard<std::mutex> lock(mutex_);

    SurroundBusInput input;
    input.id = nextInputId_++;
    input.name = name;
    input.panner = std::move(panner);

    int id = input.id;
    inputs_.push_back(std::move(input));
    return id;
}

void SurroundBus::removeInput(int inputId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    inputs_.erase(
        std::remove_if(inputs_.begin(), inputs_.end(),
                       [inputId](const SurroundBusInput& input) {
                           return input.id == inputId;
                       }),
        inputs_.end());
}

int SurroundBus::getNumInputs() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int>(inputs_.size());
}

SurroundPanner* SurroundBus::getPannerForInput(int inputId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& input : inputs_)
    {
        if (input.id == inputId)
            return input.panner.get();
    }
    return nullptr;
}

void SurroundBus::processInput(int inputId,
                               const juce::AudioBuffer<float>& monoInput,
                               juce::AudioBuffer<float>& output)
{
    std::lock_guard<std::mutex> lock(mutex_);

    SurroundPanner* panner = nullptr;
    for (auto& input : inputs_)
    {
        if (input.id == inputId)
        {
            panner = input.panner.get();
            break;
        }
    }

    if (panner == nullptr)
        return;

    const auto gains = panner->getGains();
    const int numSamples = monoInput.getNumSamples();
    const int numOutChannels = std::min(output.getNumChannels(),
                                        static_cast<int>(gains.size()));

    const float* monoData = monoInput.getReadPointer(0);

    for (int ch = 0; ch < numOutChannels; ++ch)
    {
        if (gains[static_cast<size_t>(ch)] < 0.0001f)
            continue;

        float* outData = output.getWritePointer(ch);
        float gain = gains[static_cast<size_t>(ch)];

        for (int i = 0; i < numSamples; ++i)
            outData[i] += monoData[i] * gain;
    }
}

void SurroundBus::clearOutput(juce::AudioBuffer<float>& output)
{
    output.clear();
}

}  // namespace magda
