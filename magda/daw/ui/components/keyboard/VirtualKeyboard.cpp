#include "VirtualKeyboard.hpp"

#include "../../themes/DarkTheme.hpp"
#include "../../themes/FontManager.hpp"

namespace magda {

//==============================================================================
// MidiRecorder
//==============================================================================

void MidiRecorder::startRecording(double transportPositionBeats) {
    recording_ = true;
    startPositionBeats_ = transportPositionBeats;
}

void MidiRecorder::stopRecording() {
    recording_ = false;
}

void MidiRecorder::addNoteOn(int noteNumber, int velocity, double currentPositionBeats) {
    if (!recording_) return;
    events_.push_back({noteNumber, velocity, currentPositionBeats - startPositionBeats_, true});
}

void MidiRecorder::addNoteOff(int noteNumber, double currentPositionBeats) {
    if (!recording_) return;
    events_.push_back({noteNumber, 0, currentPositionBeats - startPositionBeats_, false});
}

void MidiRecorder::clearEvents() {
    events_.clear();
}

//==============================================================================
// VirtualKeyboard - Key mapping
//==============================================================================

// Maps computer keys to note offsets within one octave
// A=C(0), W=C#(1), S=D(2), E=D#(3), D=E(4), F=F(5), T=F#(6),
// G=G(7), Y=G#(8), H=A(9), U=A#(10), J=B(11), K=C+1(12)
const std::array<VirtualKeyboard::KeyInfo, 18> VirtualKeyboard::keyMapping_ = {{
    {0,  false, 'A', "C"},
    {1,  true,  'W', "C#"},
    {2,  false, 'S', "D"},
    {3,  true,  'E', "D#"},
    {4,  false, 'D', "E"},
    {5,  false, 'F', "F"},
    {6,  true,  'T', "F#"},
    {7,  false, 'G', "G"},
    {8,  true,  'Y', "G#"},
    {9,  false, 'H', "A"},
    {10, true,  'U', "A#"},
    {11, false, 'J', "B"},
    {12, false, 'K', "C"},
    {13, true,  'O', "C#"},
    {14, false, 'L', "D"},
    {15, true,  'P', "D#"},
    {16, false, ';', "E"},
    {17, false, '\'', "F"},
}};

//==============================================================================
// VirtualKeyboard
//==============================================================================

VirtualKeyboard::VirtualKeyboard() {
    setWantsKeyboardFocus(true);
}

VirtualKeyboard::~VirtualKeyboard() = default;

void VirtualKeyboard::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    g.fillAll(DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND));

    if (bounds.getWidth() < 10 || bounds.getHeight() < 10) return;

    const int whiteKeyWidth = bounds.getWidth() / NUM_WHITE_KEYS;
    const int whiteKeyHeight = bounds.getHeight();
    const int blackKeyWidth = whiteKeyWidth * 2 / 3;
    const int blackKeyHeight = whiteKeyHeight * 3 / 5;

    // Only draw the mapped range: 18 semitones (C to F+1)
    // White keys in this range: C D E F G A B C D E F = 11
    static const int whiteNotes[] = {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17};
    static const int blackNotes[] = {1, 3, 6, 8, 10, 13, 15};

    // Draw white keys
    for (int i = 0; i < NUM_WHITE_KEYS; ++i) {
        int semitone = whiteNotes[i];
        int midiNote = baseOctave_ * 12 + semitone;
        auto keyRect = juce::Rectangle<int>(i * whiteKeyWidth, 0,
                                            whiteKeyWidth - 1, whiteKeyHeight);

        bool isPressed = pressedNotes_.count(midiNote) > 0;
        g.setColour(isPressed ? DarkTheme::getColour(DarkTheme::ACCENT_BLUE) : juce::Colours::white);
        g.fillRect(keyRect);

        g.setColour(juce::Colour(0xFF333333));
        g.drawRect(keyRect);

        // Draw computer key label at bottom
        for (auto& km : keyMapping_) {
            if (km.noteOffset == semitone && !km.isBlack) {
                g.setColour(juce::Colour(0xFF666666));
                g.setFont(FontManager::getInstance().getUIFont(9.0f));
                auto labelArea = juce::Rectangle<int>(
                    i * whiteKeyWidth, whiteKeyHeight - 20, whiteKeyWidth - 1, 14);
                g.drawText(juce::String::charToString(km.computerKey),
                           labelArea, juce::Justification::centred);
                break;
            }
        }
    }

    // Draw black keys on top
    // Black key positions relative to white key index
    static const int blackWhitePos[] = {1, 2, 4, 5, 6, 8, 9};  // after which white key
    for (int i = 0; i < 7; ++i) {
        int semitone = blackNotes[i];
        int midiNote = baseOctave_ * 12 + semitone;
        int xPos = blackWhitePos[i] * whiteKeyWidth - blackKeyWidth / 2;
        auto keyRect = juce::Rectangle<int>(xPos, 0, blackKeyWidth, blackKeyHeight);

        bool isPressed = pressedNotes_.count(midiNote) > 0;
        g.setColour(isPressed ? DarkTheme::getColour(DarkTheme::ACCENT_BLUE) : juce::Colour(0xFF222222));
        g.fillRect(keyRect);

        g.setColour(juce::Colour(0xFF111111));
        g.drawRect(keyRect);

        // Draw computer key label
        for (auto& km : keyMapping_) {
            if (km.noteOffset == semitone && km.isBlack) {
                g.setColour(juce::Colour(0xFFAAAAAA));
                g.setFont(FontManager::getInstance().getUIFont(8.0f));
                auto labelArea = juce::Rectangle<int>(xPos, blackKeyHeight - 16, blackKeyWidth, 12);
                g.drawText(juce::String::charToString(km.computerKey),
                           labelArea, juce::Justification::centred);
                break;
            }
        }
    }

    // Draw status indicators at top
    g.setFont(FontManager::getInstance().getUIFont(9.0f));
    g.setColour(DarkTheme::getSecondaryTextColour());
    juce::String status = "Oct:" + juce::String(baseOctave_) +
                          " Vel:" + juce::String(velocity_);
    if (sustainOn_) status += " [SUS]";
    g.drawText(status, bounds.removeFromTop(14).reduced(4, 0), juce::Justification::centredLeft);
}

void VirtualKeyboard::resized() {
    // No child components to layout
}

bool VirtualKeyboard::keyPressed(const juce::KeyPress& key, juce::Component*) {
    auto keyChar = static_cast<juce::juce_wchar>(juce::CharacterFunctions::toUpperCase(
        static_cast<juce::juce_wchar>(key.getTextCharacter())));

    // Octave shift
    if (keyChar == 'Z') {
        setBaseOctave(juce::jmax(0, baseOctave_ - 1));
        return true;
    }
    if (keyChar == 'X') {
        setBaseOctave(juce::jmin(8, baseOctave_ + 1));
        return true;
    }

    // Velocity control: C = decrease, V = increase
    if (keyChar == 'C') {
        setVelocity(juce::jmax(1, velocity_ - 14));
        if (onVelocityChanged) onVelocityChanged(velocity_);
        return true;
    }
    if (keyChar == 'V') {
        setVelocity(juce::jmin(127, velocity_ + 14));
        if (onVelocityChanged) onVelocityChanged(velocity_);
        return true;
    }

    // Pitch bend: 1 = down, 2 = up
    if (keyChar == '1') {
        if (onPitchBend) onPitchBend(-8192);
        return true;
    }
    if (keyChar == '2') {
        if (onPitchBend) onPitchBend(8191);
        return true;
    }

    // Sustain pedal: Tab
    if (key.getKeyCode() == juce::KeyPress::tabKey) {
        sustainOn_ = !sustainOn_;
        if (onSustain) onSustain(sustainOn_);
        repaint();
        return true;
    }

    // Check if already held
    if (heldComputerKeys_.count(keyChar) > 0) return true;

    // Modulation wheel: 3=off, 4-8=increasing values
    if (keyChar >= '3' && keyChar <= '8') {
        int modValue = (keyChar == '3') ? 0 : static_cast<int>((keyChar - '3') * 25.5f);
        if (onModWheel) onModWheel(modValue);
        repaint();
        return true;
    }

    int note = computerKeyToNote(keyChar);
    if (note >= 0) {
        heldComputerKeys_.insert(keyChar);
        triggerNoteOn(note);
        return true;
    }

    return false;
}

bool VirtualKeyboard::keyStateChanged(bool /*isKeyDown*/, juce::Component*) {
    // Check for released keys
    std::vector<juce::juce_wchar> toRelease;

    for (auto keyChar : heldComputerKeys_) {
        if (!juce::KeyPress::isKeyCurrentlyDown(keyChar) &&
            !juce::KeyPress::isKeyCurrentlyDown(
                juce::CharacterFunctions::toLowerCase(keyChar))) {
            toRelease.push_back(keyChar);
        }
    }

    for (auto keyChar : toRelease) {
        heldComputerKeys_.erase(keyChar);
        int note = computerKeyToNote(keyChar);
        if (note >= 0) {
            triggerNoteOff(note);
        }
    }

    // Pitch bend release: when 1 or 2 is released, return to center
    if (!juce::KeyPress::isKeyCurrentlyDown('1') && !juce::KeyPress::isKeyCurrentlyDown('2')) {
        if (onPitchBend) onPitchBend(0);
    }

    return !toRelease.empty();
}

void VirtualKeyboard::mouseDown(const juce::MouseEvent& e) {
    int note = getNoteAtPosition(e.getPosition());
    if (note >= 0) {
        mouseNote_ = note;
        triggerNoteOn(note);
    }
}

void VirtualKeyboard::mouseDrag(const juce::MouseEvent& e) {
    int note = getNoteAtPosition(e.getPosition());
    if (note != mouseNote_) {
        if (mouseNote_ >= 0) triggerNoteOff(mouseNote_);
        if (note >= 0) triggerNoteOn(note);
        mouseNote_ = note;
    }
}

void VirtualKeyboard::mouseUp(const juce::MouseEvent&) {
    if (mouseNote_ >= 0) {
        triggerNoteOff(mouseNote_);
        mouseNote_ = -1;
    }
}

void VirtualKeyboard::setRecording(bool shouldRecord, double transportPositionBeats) {
    if (shouldRecord)
        recorder_.startRecording(transportPositionBeats);
    else
        recorder_.stopRecording();
    repaint();
}

void VirtualKeyboard::setBaseOctave(int octave) {
    if (octave != baseOctave_) {
        // Release all currently held notes before changing octave
        for (int note : pressedNotes_) {
            if (onNoteOff) onNoteOff(note);
        }
        pressedNotes_.clear();
        heldComputerKeys_.clear();

        baseOctave_ = juce::jlimit(0, 8, octave);
        repaint();
    }
}

void VirtualKeyboard::setVelocity(int velocity) {
    velocity_ = juce::jlimit(1, 127, velocity);
}

int VirtualKeyboard::computerKeyToNote(juce::juce_wchar key) const {
    for (auto& km : keyMapping_) {
        if (static_cast<juce::juce_wchar>(km.computerKey) == key) {
            return baseOctave_ * 12 + km.noteOffset;
        }
    }
    return -1;
}

bool VirtualKeyboard::isBlackKey(int noteInOctave) const {
    // C#, D#, F#, G#, A# are black keys
    return noteInOctave == 1 || noteInOctave == 3 || noteInOctave == 6 ||
           noteInOctave == 8 || noteInOctave == 10;
}

int VirtualKeyboard::getNoteAtPosition(juce::Point<int> pos) const {
    auto bounds = getLocalBounds();
    if (!bounds.contains(pos)) return -1;

    const int whiteKeyWidth = bounds.getWidth() / NUM_WHITE_KEYS;
    const int blackKeyWidth = whiteKeyWidth * 2 / 3;
    const int blackKeyHeight = bounds.getHeight() * 3 / 5;

    // Check black keys first (they're on top)
    if (pos.getY() < blackKeyHeight) {
        int whiteIdx = 0;
        for (int octave = 0; octave < 3; ++octave) {
            int localWhite = 0;
            for (int note = 0; note < 12; ++note) {
                if (isBlackKey(note)) {
                    int xPos = (whiteIdx + localWhite) * whiteKeyWidth - blackKeyWidth / 2;
                    if (pos.getX() >= xPos && pos.getX() < xPos + blackKeyWidth) {
                        return (baseOctave_ + octave) * 12 + note;
                    }
                } else {
                    localWhite++;
                }
            }
            whiteIdx += 7;
        }
    }

    // Check white keys
    int whiteKeyIndex = pos.getX() / whiteKeyWidth;
    if (whiteKeyIndex >= 0 && whiteKeyIndex < NUM_WHITE_KEYS) {
        int octave = whiteKeyIndex / 7;
        int whiteInOctave = whiteKeyIndex % 7;
        // Map white key index to note: 0=C, 1=D, 2=E, 3=F, 4=G, 5=A, 6=B
        static const int whiteToNote[] = {0, 2, 4, 5, 7, 9, 11};
        if (whiteInOctave < 7) {
            return (baseOctave_ + octave) * 12 + whiteToNote[whiteInOctave];
        }
    }

    return -1;
}

void VirtualKeyboard::triggerNoteOn(int noteNumber) {
    if (noteNumber < 0 || noteNumber > 127) return;
    pressedNotes_.insert(noteNumber);

    if (onNoteOn) onNoteOn(noteNumber, velocity_);

    if (recorder_.isRecording() && getTransportPosition) {
        recorder_.addNoteOn(noteNumber, velocity_, getTransportPosition());
    }

    repaint();
}

void VirtualKeyboard::triggerNoteOff(int noteNumber) {
    if (noteNumber < 0 || noteNumber > 127) return;
    pressedNotes_.erase(noteNumber);

    if (onNoteOff) onNoteOff(noteNumber);

    if (recorder_.isRecording() && getTransportPosition) {
        recorder_.addNoteOff(noteNumber, getTransportPosition());
    }

    repaint();
}

}  // namespace magda
