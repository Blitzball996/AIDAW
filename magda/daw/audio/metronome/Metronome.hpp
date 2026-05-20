#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <memory>

namespace magda
{

//==============================================================================
/**
    Generates click audio synchronised to the transport.
    Supports custom click sounds, count-in, subdivisions, and accent on beat 1.
*/
class Metronome
{
public:
    Metronome();
    ~Metronome();

    //==========================================================================
    // Enable / disable
    //==========================================================================

    void setEnabled (bool enabled);
    bool isEnabled() const noexcept;

    //==========================================================================
    // Volume
    //==========================================================================

    /// Set click volume (0.0 - 1.0).
    void setVolume (float volume);
    float getVolume() const noexcept;

    //==========================================================================
    // Accent
    //==========================================================================

    /// Accent the first beat of each bar.
    void setAccentFirst (bool accent);
    bool getAccentFirst() const noexcept;

    //==========================================================================
    // Count-in
    //==========================================================================

    /// Set the number of bars to count in before recording starts.
    void setCountIn (int bars);
    int getCountIn() const noexcept;

    /// Returns true if currently in count-in phase.
    bool isCountingIn() const noexcept;

    /// Start the count-in. Call before transport starts recording.
    void startCountIn();

    /// Reset count-in state.
    void resetCountIn();

    //==========================================================================
    // Subdivision
    //==========================================================================

    /// Set subdivision: 1 = quarter notes, 2 = 8th notes, 4 = 16th notes.
    void setSubdivision (int subdivision);
    int getSubdivision() const noexcept;

    //==========================================================================
    // Custom sounds
    //==========================================================================

    /// Load a custom click sound (for normal beats).
    bool loadClickSound (const juce::File& audioFile);

    /// Load a custom accent sound (for beat 1).
    bool loadAccentSound (const juce::File& audioFile);

    /// Reset to built-in synthesised click sounds.
    void resetToDefaultSounds();

    //==========================================================================
    // Audio generation
    //==========================================================================

    /// Prepare for playback.
    void prepareToPlay (double sampleRate, int blockSize);

    /// Generate click audio for the current block based on tempo and position.
    /// @param buffer       Output buffer to mix clicks into.
    /// @param numSamples   Number of samples in this block.
    /// @param tempo        Current tempo in BPM.
    /// @param timeSignatureNumerator   Beats per bar.
    /// @param positionInBeats  Current playback position in beats.
    void generateClickBuffer (juce::AudioBuffer<float>& buffer,
                              int numSamples,
                              double tempo,
                              int timeSignatureNumerator,
                              double positionInBeats);

private:
    bool   enabled_       = false;
    float  volume_        = 0.8f;
    bool   accentFirst_   = true;
    int    countInBars_   = 0;
    int    subdivision_   = 1;
    bool   countingIn_    = false;
    int    countInBeatsRemaining_ = 0;

    double sampleRate_    = 44100.0;
    int    blockSize_     = 512;

    // Custom click sounds
    std::unique_ptr<juce::AudioBuffer<float>> clickSound_;
    std::unique_ptr<juce::AudioBuffer<float>> accentSound_;

    // Default synthesised click
    std::unique_ptr<juce::AudioBuffer<float>> defaultClick_;
    std::unique_ptr<juce::AudioBuffer<float>> defaultAccent_;

    // Playback state for click samples
    int clickPlaybackPos_  = -1;
    int accentPlaybackPos_ = -1;
    bool playingAccent_    = false;

    // Track which beats have already triggered in the current block
    double lastTriggeredBeat_ = -1.0;

    /// Generate the default synthesised click sounds.
    void generateDefaultSounds();

    /// Get the appropriate click buffer (custom or default).
    const juce::AudioBuffer<float>* getClickBuffer() const;
    const juce::AudioBuffer<float>* getAccentBuffer() const;

    /// Load an audio file into a buffer.
    std::unique_ptr<juce::AudioBuffer<float>> loadAudioFile (const juce::File& file);

    juce::AudioFormatManager formatManager_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Metronome)
};

} // namespace magda
