#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <functional>
#include <vector>

namespace magda {

/**
 * @brief Records MIDI note events with timestamps relative to transport position.
 */
class MidiRecorder {
  public:
    struct NoteEvent {
        int noteNumber = 0;
        int velocity = 0;
        double timestampBeats = 0.0;
        bool isNoteOn = true;
    };

    void startRecording(double transportPositionBeats);
    void stopRecording();
    bool isRecording() const noexcept { return recording_; }

    void addNoteOn(int noteNumber, int velocity, double currentPositionBeats);
    void addNoteOff(int noteNumber, double currentPositionBeats);

    const std::vector<NoteEvent>& getRecordedEvents() const noexcept { return events_; }
    void clearEvents();

  private:
    bool recording_ = false;
    double startPositionBeats_ = 0.0;
    std::vector<NoteEvent> events_;
};

/**
 * @brief Virtual piano keyboard that maps computer keys to MIDI notes.
 *
 * Key mapping (default octave C4):
 *   A=C, W=C#, S=D, E=D#, D=E, F=F, T=F#, G=G, Y=G#, H=A, U=A#, J=B, K=C+1
 *   Z = octave down, X = octave up
 *
 * Displays a visual piano with highlighted keys on press.
 * Outputs MIDI noteOn/noteOff that can be recorded to the current track.
 */
class VirtualKeyboard : public juce::Component, public juce::KeyListener {
  public:
    VirtualKeyboard();
    ~VirtualKeyboard() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // KeyListener
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) override;

    // Mouse interaction on piano keys
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    // Recording control
    void setRecording(bool shouldRecord, double transportPositionBeats = 0.0);
    bool isRecording() const noexcept { return recorder_.isRecording(); }
    MidiRecorder& getRecorder() { return recorder_; }

    // Octave control
    void setBaseOctave(int octave);
    int getBaseOctave() const noexcept { return baseOctave_; }

    // Velocity
    void setVelocity(int velocity);
    int getVelocity() const noexcept { return velocity_; }
    bool isSustainOn() const noexcept { return sustainOn_; }

    // Callbacks
    std::function<void(int noteNumber, int velocity)> onNoteOn;
    std::function<void(int noteNumber)> onNoteOff;
    std::function<void(int pitchBendValue)> onPitchBend;  // -8192 to 8191, 0=center
    std::function<void(bool on)> onSustain;               // sustain pedal toggle
    std::function<void(int velocity)> onVelocityChanged;  // velocity changed via C/V
    std::function<double()> getTransportPosition;

  private:
    static constexpr int NUM_WHITE_KEYS = 21;
    static constexpr int NUM_TOTAL_KEYS = 36;

    struct KeyInfo {
        int noteOffset;       // Semitone offset from C
        bool isBlack;
        char computerKey;     // Mapped computer key (0 if none)
        juce::String label;   // Display label
    };

    // Piano key layout for one octave (C to B)
    static const std::array<KeyInfo, 18> keyMapping_;

    int baseOctave_ = 4;
    int velocity_ = 100;
    bool sustainOn_ = false;

    // Currently pressed keys (by MIDI note number)
    std::set<int> pressedNotes_;
    int mouseNote_ = -1;

    // Computer key state tracking
    std::set<juce::juce_wchar> heldComputerKeys_;

    MidiRecorder recorder_;

    // Layout helpers
    juce::Rectangle<int> getWhiteKeyRect(int whiteKeyIndex) const;
    juce::Rectangle<int> getBlackKeyRect(int blackKeyPosition) const;
    int getNoteAtPosition(juce::Point<int> pos) const;
    int computerKeyToNote(juce::juce_wchar key) const;
    bool isBlackKey(int noteInOctave) const;
    int getWhiteKeyIndex(int midiNote) const;

    void triggerNoteOn(int noteNumber);
    void triggerNoteOff(int noteNumber);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VirtualKeyboard)
};

}  // namespace magda
