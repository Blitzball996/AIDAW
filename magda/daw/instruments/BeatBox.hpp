#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <vector>

namespace magda
{

//==============================================================================
/// A single MIDI note event produced by the pattern sequencer.
struct MidiNoteEvent
{
    int    noteNumber = 0;
    int    velocity   = 0;
    double startBeat  = 0.0;
    double lengthBeat = 0.0;
};

//==============================================================================
/// Drum instrument identifiers for the 8 default slots.
enum class DrumInstrument : int
{
    Kick      = 0,
    Snare     = 1,
    ClosedHH  = 2,
    OpenHH    = 3,
    LowTom    = 4,
    MidTom    = 5,
    HiTom     = 6,
    Clap      = 7
};

//==============================================================================
/// A drum pattern: 16 steps x 8 instruments.
struct Pattern
{
    juce::String name;
    std::array<std::array<int, 8>, 16> steps {}; ///< velocity per step/instrument (0 = off)
    float swing = 0.0f;   ///< Swing amount 0.0 - 1.0
    double tempo = 120.0;  ///< Pattern tempo (BPM)
};

//==============================================================================
/**
    Step sequencer for drum patterns.
    Provides 16 steps x 8 instruments with velocity control.
*/
class BeatBox
{
public:
    static constexpr int NumSteps       = 16;
    static constexpr int NumInstruments = 8;

    BeatBox();
    ~BeatBox();

    /// Set a step with a given velocity (1-127). 0 clears.
    void setStep (int step, int instrument, int velocity);

    /// Clear a specific step.
    void clearStep (int step, int instrument);

    /// Toggle a step on/off (uses default velocity 100 when toggling on).
    void toggleStep (int step, int instrument);

    /// Get the velocity at a step/instrument.
    int getStepVelocity (int step, int instrument) const;

    /// Convert the current pattern to a list of MIDI note events.
    std::vector<MidiNoteEvent> getPatternAsMidi() const;

    /// Get/set the active pattern.
    const Pattern& getPattern() const noexcept;
    void setPattern (const Pattern& pattern);

    /// Set swing amount (0.0 = straight, 1.0 = full swing).
    void setSwing (float swing);
    float getSwing() const noexcept;

    /// Set pattern tempo.
    void setTempo (double bpm);
    double getTempo() const noexcept;

    /// Set the pattern name.
    void setPatternName (const juce::String& name);

    /// Clear all steps.
    void clearAll();

    //==========================================================================
    // Built-in patterns
    //==========================================================================
    static Pattern createFourOnFloor();
    static Pattern createBreakbeat();
    static Pattern createBossaNova();
    static Pattern createShuffle();

    /// Get the default MIDI note number for a drum instrument.
    static int getDefaultNoteForInstrument (int instrument);

private:
    Pattern currentPattern_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeatBox)
};

} // namespace magda
