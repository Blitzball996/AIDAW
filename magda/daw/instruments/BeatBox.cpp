#include "BeatBox.hpp"

namespace magda
{

BeatBox::BeatBox()
{
    currentPattern_.name = "Empty";
    currentPattern_.tempo = 120.0;
    currentPattern_.swing = 0.0f;
}

BeatBox::~BeatBox() = default;

//==============================================================================
void BeatBox::setStep (int step, int instrument, int velocity)
{
    if (step < 0 || step >= NumSteps || instrument < 0 || instrument >= NumInstruments)
        return;

    currentPattern_.steps[(size_t) step][(size_t) instrument] =
        juce::jlimit (0, 127, velocity);
}

void BeatBox::clearStep (int step, int instrument)
{
    setStep (step, instrument, 0);
}

void BeatBox::toggleStep (int step, int instrument)
{
    if (step < 0 || step >= NumSteps || instrument < 0 || instrument >= NumInstruments)
        return;

    auto& vel = currentPattern_.steps[(size_t) step][(size_t) instrument];
    vel = (vel > 0) ? 0 : 100;
}

int BeatBox::getStepVelocity (int step, int instrument) const
{
    if (step < 0 || step >= NumSteps || instrument < 0 || instrument >= NumInstruments)
        return 0;

    return currentPattern_.steps[(size_t) step][(size_t) instrument];
}

//==============================================================================
std::vector<MidiNoteEvent> BeatBox::getPatternAsMidi() const
{
    std::vector<MidiNoteEvent> events;
    const double beatsPerStep = 0.25; // 16th notes

    for (int step = 0; step < NumSteps; ++step)
    {
        for (int inst = 0; inst < NumInstruments; ++inst)
        {
            int vel = currentPattern_.steps[(size_t) step][(size_t) inst];
            if (vel > 0)
            {
                MidiNoteEvent event;
                event.noteNumber = getDefaultNoteForInstrument (inst);
                event.velocity   = vel;

                // Apply swing to even-numbered steps (off-beats)
                double beatPos = step * beatsPerStep;
                if (step % 2 == 1)
                    beatPos += currentPattern_.swing * beatsPerStep * 0.5;

                event.startBeat  = beatPos;
                event.lengthBeat = beatsPerStep * 0.9; // Slightly shorter than full step
                events.push_back (event);
            }
        }
    }

    return events;
}

//==============================================================================
const Pattern& BeatBox::getPattern() const noexcept
{
    return currentPattern_;
}

void BeatBox::setPattern (const Pattern& pattern)
{
    currentPattern_ = pattern;
}

void BeatBox::setSwing (float swing)
{
    currentPattern_.swing = juce::jlimit (0.0f, 1.0f, swing);
}

float BeatBox::getSwing() const noexcept
{
    return currentPattern_.swing;
}

void BeatBox::setTempo (double bpm)
{
    currentPattern_.tempo = juce::jlimit (20.0, 999.0, bpm);
}

double BeatBox::getTempo() const noexcept
{
    return currentPattern_.tempo;
}

void BeatBox::setPatternName (const juce::String& name)
{
    currentPattern_.name = name;
}

void BeatBox::clearAll()
{
    for (auto& row : currentPattern_.steps)
        row.fill (0);
}

//==============================================================================
int BeatBox::getDefaultNoteForInstrument (int instrument)
{
    // General MIDI drum map
    static constexpr int noteMap[NumInstruments] = {
        36, // Kick (C1)
        38, // Snare (D1)
        42, // Closed HH (F#1)
        46, // Open HH (A#1)
        41, // Low Tom (F1)
        45, // Mid Tom (A1)
        48, // Hi Tom (C2)
        39  // Clap (D#1)
    };

    if (instrument >= 0 && instrument < NumInstruments)
        return noteMap[instrument];

    return 60; // Middle C fallback
}

//==============================================================================
// Built-in patterns
//==============================================================================

Pattern BeatBox::createFourOnFloor()
{
    Pattern p;
    p.name  = "Four on Floor";
    p.tempo = 120.0;
    p.swing = 0.0f;

    // Kick on every beat (steps 0, 4, 8, 12)
    p.steps[0][0] = 127;  p.steps[4][0] = 127;
    p.steps[8][0] = 127;  p.steps[12][0] = 127;

    // Snare on 2 and 4 (steps 4, 12)
    p.steps[4][1] = 110;  p.steps[12][1] = 110;

    // Closed HH on every 8th (steps 0,2,4,6,8,10,12,14)
    for (int i = 0; i < 16; i += 2)
        p.steps[i][2] = 80;

    // Open HH on off-beats
    p.steps[2][3] = 70;  p.steps[6][3] = 70;
    p.steps[10][3] = 70; p.steps[14][3] = 70;

    return p;
}

Pattern BeatBox::createBreakbeat()
{
    Pattern p;
    p.name  = "Breakbeat";
    p.tempo = 130.0;
    p.swing = 0.0f;

    // Kick
    p.steps[0][0] = 127;  p.steps[6][0] = 100;
    p.steps[10][0] = 110;

    // Snare
    p.steps[4][1] = 120;  p.steps[12][1] = 120;
    p.steps[14][1] = 80;

    // Closed HH on all 16ths
    for (int i = 0; i < 16; ++i)
        p.steps[i][2] = 70;

    // Open HH
    p.steps[2][3] = 90;  p.steps[8][3] = 90;

    return p;
}

Pattern BeatBox::createBossaNova()
{
    Pattern p;
    p.name  = "Bossa Nova";
    p.tempo = 140.0;
    p.swing = 0.0f;

    // Kick
    p.steps[0][0] = 100;  p.steps[6][0] = 90;
    p.steps[10][0] = 90;

    // Snare (cross-stick)
    p.steps[4][1] = 80;  p.steps[12][1] = 80;

    // Closed HH (shaker pattern)
    p.steps[0][2] = 60;  p.steps[2][2] = 50;
    p.steps[4][2] = 60;  p.steps[6][2] = 50;
    p.steps[8][2] = 60;  p.steps[10][2] = 50;
    p.steps[12][2] = 60; p.steps[14][2] = 50;

    // Clap (rimshot)
    p.steps[3][7] = 70;  p.steps[7][7] = 70;
    p.steps[11][7] = 70; p.steps[15][7] = 70;

    return p;
}

Pattern BeatBox::createShuffle()
{
    Pattern p;
    p.name  = "Shuffle";
    p.tempo = 110.0;
    p.swing = 0.6f;

    // Kick
    p.steps[0][0] = 127;  p.steps[8][0] = 110;
    p.steps[6][0] = 80;

    // Snare
    p.steps[4][1] = 120;  p.steps[12][1] = 120;

    // Closed HH (shuffled via swing)
    for (int i = 0; i < 16; ++i)
        p.steps[i][2] = (i % 2 == 0) ? 90 : 50;

    return p;
}

} // namespace magda
