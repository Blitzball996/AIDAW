#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <array>

namespace magda
{

//==============================================================================
/// A single sample slot in the sampler.
struct SampleSlot
{
    juce::String name;
    juce::String filePath;
    int          rootNote = 60;   ///< MIDI note that plays at original pitch
    float        gain     = 1.0f; ///< Linear gain (0.0 - 2.0)
    float        pan      = 0.0f; ///< -1.0 (left) to 1.0 (right)
    bool         loaded   = false;
};

//==============================================================================
/**
    Simple sample playback engine with 16 slots.
    Each slot can hold one audio sample triggered by MIDI note or direct call.
*/
class Sampler
{
public:
    static constexpr int NumSlots = 16;

    Sampler();
    ~Sampler();

    /// Prepare the sampler for playback.
    void prepareToPlay (double sampleRate, int blockSize);

    /// Load a sample file into a slot.
    bool loadSample (int slotIndex, const juce::File& filePath);

    /// Unload a sample from a slot.
    void unloadSample (int slotIndex);

    /// Trigger playback of a sample slot with given velocity (0-127).
    void triggerNote (int slotIndex, int velocity);

    /// Stop playback of a sample slot.
    void stopNote (int slotIndex);

    /// Stop all playing samples.
    void stopAll();

    /// Get slot info.
    const SampleSlot& getSlot (int slotIndex) const;

    /// Set slot gain (linear, 0.0 - 2.0).
    void setSlotGain (int slotIndex, float gain);

    /// Set slot pan (-1.0 to 1.0).
    void setSlotPan (int slotIndex, float pan);

    /// Set the root note for a slot.
    void setSlotRootNote (int slotIndex, int noteNumber);

    /// Set the slot name.
    void setSlotName (int slotIndex, const juce::String& name);

    /// Check if a slot is currently playing.
    bool isSlotPlaying (int slotIndex) const;

    /// Render audio into the given buffer.
    void renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

private:
    struct PlaybackState
    {
        bool   playing     = false;
        int    position    = 0;
        float  velocity    = 1.0f;
    };

    std::array<SampleSlot, NumSlots>                        slots_;
    std::array<std::unique_ptr<juce::AudioBuffer<float>>, NumSlots> sampleBuffers_;
    std::array<PlaybackState, NumSlots>                     playbackStates_;

    double sampleRate_ = 44100.0;
    int    blockSize_  = 512;

    juce::AudioFormatManager formatManager_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sampler)
};

} // namespace magda
