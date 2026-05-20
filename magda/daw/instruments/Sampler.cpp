#include "Sampler.hpp"

namespace magda
{

Sampler::Sampler()
{
    formatManager_.registerBasicFormats();
}

Sampler::~Sampler() = default;

//==============================================================================
void Sampler::prepareToPlay (double sampleRate, int blockSize)
{
    sampleRate_ = sampleRate;
    blockSize_  = blockSize;
}

bool Sampler::loadSample (int slotIndex, const juce::File& filePath)
{
    if (slotIndex < 0 || slotIndex >= NumSlots)
        return false;

    if (! filePath.existsAsFile())
        return false;

    std::unique_ptr<juce::AudioFormatReader> reader (
        formatManager_.createReaderFor (filePath));

    if (reader == nullptr)
        return false;

    auto buffer = std::make_unique<juce::AudioBuffer<float>> (
        (int) reader->numChannels, (int) reader->lengthInSamples);

    reader->read (buffer.get(), 0, (int) reader->lengthInSamples, 0, true, true);

    sampleBuffers_[(size_t) slotIndex] = std::move (buffer);
    playbackStates_[(size_t) slotIndex] = {};

    auto& slot   = slots_[(size_t) slotIndex];
    slot.filePath = filePath.getFullPathName();
    slot.name     = filePath.getFileNameWithoutExtension();
    slot.loaded   = true;

    return true;
}

void Sampler::unloadSample (int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= NumSlots)
        return;

    sampleBuffers_[(size_t) slotIndex].reset();
    playbackStates_[(size_t) slotIndex] = {};
    slots_[(size_t) slotIndex] = {};
}

//==============================================================================
void Sampler::triggerNote (int slotIndex, int velocity)
{
    if (slotIndex < 0 || slotIndex >= NumSlots)
        return;

    if (sampleBuffers_[(size_t) slotIndex] == nullptr)
        return;

    auto& state   = playbackStates_[(size_t) slotIndex];
    state.playing  = true;
    state.position = 0;
    state.velocity = juce::jlimit (0.0f, 1.0f, velocity / 127.0f);
}

void Sampler::stopNote (int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= NumSlots)
        return;

    playbackStates_[(size_t) slotIndex].playing = false;
}

void Sampler::stopAll()
{
    for (auto& state : playbackStates_)
        state.playing = false;
}

//==============================================================================
const SampleSlot& Sampler::getSlot (int slotIndex) const
{
    static const SampleSlot empty;
    if (slotIndex < 0 || slotIndex >= NumSlots)
        return empty;
    return slots_[(size_t) slotIndex];
}

void Sampler::setSlotGain (int slotIndex, float gain)
{
    if (slotIndex >= 0 && slotIndex < NumSlots)
        slots_[(size_t) slotIndex].gain = juce::jlimit (0.0f, 2.0f, gain);
}

void Sampler::setSlotPan (int slotIndex, float pan)
{
    if (slotIndex >= 0 && slotIndex < NumSlots)
        slots_[(size_t) slotIndex].pan = juce::jlimit (-1.0f, 1.0f, pan);
}

void Sampler::setSlotRootNote (int slotIndex, int noteNumber)
{
    if (slotIndex >= 0 && slotIndex < NumSlots)
        slots_[(size_t) slotIndex].rootNote = juce::jlimit (0, 127, noteNumber);
}

void Sampler::setSlotName (int slotIndex, const juce::String& name)
{
    if (slotIndex >= 0 && slotIndex < NumSlots)
        slots_[(size_t) slotIndex].name = name;
}

bool Sampler::isSlotPlaying (int slotIndex) const
{
    if (slotIndex < 0 || slotIndex >= NumSlots)
        return false;
    return playbackStates_[(size_t) slotIndex].playing;
}

//==============================================================================
void Sampler::renderNextBlock (juce::AudioBuffer<float>& buffer,
                               int startSample, int numSamples)
{
    for (int slot = 0; slot < NumSlots; ++slot)
    {
        auto& state = playbackStates_[(size_t) slot];
        if (! state.playing)
            continue;

        auto* sampleBuf = sampleBuffers_[(size_t) slot].get();
        if (sampleBuf == nullptr)
            continue;

        const auto& slotInfo = slots_[(size_t) slot];
        const int sampleLen  = sampleBuf->getNumSamples();
        const int numCh      = juce::jmin (buffer.getNumChannels(), sampleBuf->getNumChannels());
        const float gain     = slotInfo.gain * state.velocity;

        // Simple pan law
        const float leftGain  = gain * juce::jlimit (0.0f, 1.0f, 1.0f - slotInfo.pan);
        const float rightGain = gain * juce::jlimit (0.0f, 1.0f, 1.0f + slotInfo.pan);

        int samplesRemaining = numSamples;
        int destPos = startSample;

        while (samplesRemaining > 0 && state.position < sampleLen)
        {
            int toCopy = juce::jmin (samplesRemaining, sampleLen - state.position);

            if (buffer.getNumChannels() >= 2 && numCh >= 1)
            {
                // Stereo output
                buffer.addFrom (0, destPos, *sampleBuf, 0, state.position, toCopy, leftGain);

                if (numCh >= 2)
                    buffer.addFrom (1, destPos, *sampleBuf, 1, state.position, toCopy, rightGain);
                else
                    buffer.addFrom (1, destPos, *sampleBuf, 0, state.position, toCopy, rightGain);
            }
            else if (buffer.getNumChannels() >= 1 && numCh >= 1)
            {
                buffer.addFrom (0, destPos, *sampleBuf, 0, state.position, toCopy, gain);
            }

            state.position += toCopy;
            destPos += toCopy;
            samplesRemaining -= toCopy;
        }

        if (state.position >= sampleLen)
            state.playing = false;
    }
}

} // namespace magda
