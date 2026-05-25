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
    setMouseClickGrabsKeyboardFocus(true);
    setInterceptsMouseClicks(true, false);
}

VirtualKeyboard::~VirtualKeyboard() = default;

void VirtualKeyboard::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    g.fillAll(juce::Colour(0xFF2D2D2D));  // Dark background like Logic

    if (bounds.getWidth() < 100 || bounds.getHeight() < 60) return;

    // Layout like Logic: [PitchBend|ModWheel] [Piano Keys] / [Bottom controls]
    constexpr int leftPanelW = 50;
    constexpr int bottomBarH = 24;

    auto leftPanel = bounds.removeFromLeft(leftPanelW);
    auto bottomBar = bounds.removeFromBottom(bottomBarH);
    auto keyArea = bounds;

    // === LEFT PANEL: Pitch Bend slider + Mod Wheel slider ===
    auto pitchSlider = leftPanel.removeFromLeft(24);
    auto modSlider = leftPanel;

    // Pitch bend vertical bar
    {
        auto barRect = pitchSlider.reduced(4, 8).toFloat();
        g.setColour(juce::Colour(0xFF444444));
        g.fillRoundedRectangle(barRect, 3.0f);
        // Center line
        float centerY = barRect.getCentreY();
        g.setColour(juce::Colour(0xFF666666));
        g.drawHorizontalLine(static_cast<int>(centerY), barRect.getX(), barRect.getRight());
        // Label
        g.setColour(DarkTheme::getSecondaryTextColour());
        g.setFont(FontManager::getInstance().getUIFont(7.0f));
        g.drawText("1", pitchSlider.removeFromTop(12), juce::Justification::centred);
        g.drawText("2", pitchSlider.removeFromBottom(12), juce::Justification::centred);
    }

    // Mod wheel vertical bar
    {
        auto barRect = modSlider.reduced(4, 8).toFloat();
        g.setColour(juce::Colour(0xFF444444));
        g.fillRoundedRectangle(barRect, 3.0f);
        // Label
        g.setColour(DarkTheme::getSecondaryTextColour());
        g.setFont(FontManager::getInstance().getUIFont(7.0f));
        g.drawText("Mod", modSlider.removeFromTop(12), juce::Justification::centred);
        g.drawText("3-8", modSlider.removeFromBottom(12), juce::Justification::centred);
    }

    // === KEYBOARD AREA ===
    const int whiteKeyWidth = keyArea.getWidth() / NUM_WHITE_KEYS;
    const int whiteKeyHeight = keyArea.getHeight();
    const int blackKeyWidth = whiteKeyWidth * 2 / 3;
    const int blackKeyHeight = whiteKeyHeight * 3 / 5;
    const int keyX = keyArea.getX();
    const int keyY = keyArea.getY();

    static const int whiteNotes[] = {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17};
    static const int blackNotes[] = {1, 3, 6, 8, 10, 13, 15};

    // White keys
    for (int i = 0; i < NUM_WHITE_KEYS; ++i) {
        int semitone = whiteNotes[i];
        int midiNote = baseOctave_ * 12 + semitone;
        auto keyRect = juce::Rectangle<int>(keyX + i * whiteKeyWidth, keyY,
                                            whiteKeyWidth - 1, whiteKeyHeight);

        bool isPressed = pressedNotes_.count(midiNote) > 0;
        g.setColour(isPressed ? DarkTheme::getColour(DarkTheme::ACCENT_BLUE) : juce::Colour(0xFFF0F0F0));
        g.fillRect(keyRect);
        g.setColour(juce::Colour(0xFF888888));
        g.drawRect(keyRect);

        // Key label at bottom
        for (auto& km : keyMapping_) {
            if (km.noteOffset == semitone && !km.isBlack) {
                g.setColour(isPressed ? juce::Colours::white : juce::Colour(0xFF444444));
                g.setFont(FontManager::getInstance().getUIFont(10.0f));
                g.drawText(juce::String::charToString(km.computerKey),
                           keyRect.withTop(keyRect.getBottom() - 18),
                           juce::Justification::centred);
                break;
            }
        }
    }

    // Black keys
    static const int blackWhitePos[] = {1, 2, 4, 5, 6, 8, 9};
    for (int i = 0; i < 7; ++i) {
        int semitone = blackNotes[i];
        int midiNote = baseOctave_ * 12 + semitone;
        int xPos = keyX + blackWhitePos[i] * whiteKeyWidth - blackKeyWidth / 2;
        auto keyRect = juce::Rectangle<int>(xPos, keyY, blackKeyWidth, blackKeyHeight);

        bool isPressed = pressedNotes_.count(midiNote) > 0;
        g.setColour(isPressed ? DarkTheme::getColour(DarkTheme::ACCENT_BLUE) : juce::Colour(0xFF1A1A1A));
        g.fillRect(keyRect);
        g.setColour(juce::Colour(0xFF000000));
        g.drawRect(keyRect);

        for (auto& km : keyMapping_) {
            if (km.noteOffset == semitone && km.isBlack) {
                g.setColour(isPressed ? juce::Colours::white : juce::Colour(0xFFAAAAAA));
                g.setFont(FontManager::getInstance().getUIFont(8.0f));
                g.drawText(juce::String::charToString(km.computerKey),
                           keyRect.withTop(keyRect.getBottom() - 14),
                           juce::Justification::centred);
                break;
            }
        }
    }

    // === BOTTOM BAR: [Z Oct X]  [C Vel V]  [Tab:Sus] ===
    g.setFont(FontManager::getInstance().getUIFont(10.0f));
    g.setColour(juce::Colour(0xFF3A3A3A));
    g.fillRect(bottomBar);

    const auto btnNormal = juce::Colour(0xFF555555);
    const auto btnPressed = juce::Colour(0xFF888888);

    // Octave down button
    auto zBtn = bottomBar.removeFromLeft(24);
    g.setColour(pressedButton_ == 0 ? btnPressed : btnNormal);
    g.fillRoundedRectangle(zBtn.reduced(2).toFloat(), 3.0f);
    g.setColour(juce::Colours::white);
    g.drawText("Z", zBtn, juce::Justification::centred);

    // Octave label
    auto octLabel = bottomBar.removeFromLeft(50);
    g.setColour(juce::Colours::white);
    g.drawText("Oct:" + juce::String(baseOctave_), octLabel, juce::Justification::centred);

    // Octave up button
    auto xBtn = bottomBar.removeFromLeft(24);
    g.setColour(pressedButton_ == 1 ? btnPressed : btnNormal);
    g.fillRoundedRectangle(xBtn.reduced(2).toFloat(), 3.0f);
    g.setColour(juce::Colours::white);
    g.drawText("X", xBtn, juce::Justification::centred);

    bottomBar.removeFromLeft(12);

    // Velocity down button
    auto cBtn = bottomBar.removeFromLeft(24);
    g.setColour(pressedButton_ == 2 ? btnPressed : btnNormal);
    g.fillRoundedRectangle(cBtn.reduced(2).toFloat(), 3.0f);
    g.setColour(juce::Colours::white);
    g.drawText("C", cBtn, juce::Justification::centred);

    // Velocity label
    auto velLabel = bottomBar.removeFromLeft(50);
    g.setColour(juce::Colours::white);
    g.drawText("Vel:" + juce::String(velocity_), velLabel, juce::Justification::centred);

    // Velocity up button
    auto vBtn = bottomBar.removeFromLeft(24);
    g.setColour(pressedButton_ == 3 ? btnPressed : btnNormal);
    g.fillRoundedRectangle(vBtn.reduced(2).toFloat(), 3.0f);
    g.setColour(juce::Colours::white);
    g.drawText("V", vBtn, juce::Justification::centred);

    bottomBar.removeFromLeft(12);

    // Sustain button
    auto susBtn = bottomBar.removeFromLeft(70);
    auto susColor = sustainOn_ ? DarkTheme::getColour(DarkTheme::ACCENT_GREEN) : btnNormal;
    g.setColour(pressedButton_ == 4 ? susColor.brighter(0.3f) : susColor);
    g.fillRoundedRectangle(susBtn.reduced(2).toFloat(), 3.0f);
    g.setColour(juce::Colours::white);
    g.drawText("Tab:Sus", susBtn, juce::Justification::centred);
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
    auto bounds = getLocalBounds();
    constexpr int leftPanelW = 50;
    constexpr int bottomBarH = 24;
    auto bottomBar = bounds.removeFromBottom(bottomBarH);

    // Check if click is in bottom bar (control buttons)
    if (e.getPosition().getY() >= bottomBar.getY()) {
        // Bottom bar is drawn after left panel, so offset x by leftPanelW
        int x = e.getPosition().getX() - leftPanelW;
        if (x < 0) return;
        // Match the layout from paint(): Z(24) Oct(50) X(24) gap(12) C(24) Vel(50) V(24) gap(12) Sus(70)
        int pos = 0;
        // Z button
        if (x >= pos && x < pos + 24) { pressedButton_ = 0; setBaseOctave(juce::jmax(0, baseOctave_ - 1)); repaint(); return; }
        pos += 24 + 50;  // skip Z + Oct label
        // X button
        if (x >= pos && x < pos + 24) { pressedButton_ = 1; setBaseOctave(juce::jmin(8, baseOctave_ + 1)); repaint(); return; }
        pos += 24 + 12;  // skip X + gap
        // C button
        if (x >= pos && x < pos + 24) { pressedButton_ = 2; setVelocity(juce::jmax(1, velocity_ - 14)); if (onVelocityChanged) onVelocityChanged(velocity_); repaint(); return; }
        pos += 24 + 50;  // skip C + Vel label
        // V button
        if (x >= pos && x < pos + 24) { pressedButton_ = 3; setVelocity(juce::jmin(127, velocity_ + 14)); if (onVelocityChanged) onVelocityChanged(velocity_); repaint(); return; }
        pos += 24 + 12;  // skip V + gap
        // Sustain button
        if (x >= pos && x < pos + 70) { pressedButton_ = 4; sustainOn_ = !sustainOn_; if (onSustain) onSustain(sustainOn_); repaint(); return; }
        return;
    }

    // Check if click is in left panel (pitch bend / mod wheel)
    if (e.getPosition().getX() < leftPanelW) {
        // Use the key area (full bounds minus left panel and bottom bar)
        auto keyArea = getLocalBounds();
        keyArea.removeFromBottom(bottomBarH);
        auto leftArea = keyArea.removeFromLeft(leftPanelW);

        if (e.getPosition().getX() < 24) {
            // Pitch bend area — map Y to -8192..8191
            float ratio = 1.0f - static_cast<float>(e.getPosition().getY() - leftArea.getY()) / static_cast<float>(leftArea.getHeight());
            int pbVal = static_cast<int>((ratio - 0.5f) * 2.0f * 8191.0f);
            pbVal = juce::jlimit(-8192, 8191, pbVal);
            if (onPitchBend) onPitchBend(pbVal);
        } else {
            // Mod wheel area — map Y position to 0-127
            float ratio = 1.0f - static_cast<float>(e.getPosition().getY() - leftArea.getY()) / static_cast<float>(leftArea.getHeight());
            int modVal = juce::jlimit(0, 127, static_cast<int>(ratio * 127));
            if (onModWheel) onModWheel(modVal);
        }
        repaint();
        return;
    }

    // Piano key click
    int note = getNoteAtPosition(e.getPosition());
    if (note >= 0) {
        mouseNote_ = note;
        triggerNoteOn(note);
    }
    grabKeyboardFocus();
}

void VirtualKeyboard::mouseDrag(const juce::MouseEvent& e) {
    constexpr int leftPanelW = 50;
    constexpr int bottomBarH = 24;

    // Handle drag in left panel (continuous pitch bend / mod wheel control)
    if (e.getPosition().getX() < leftPanelW && e.getPosition().getY() < getHeight() - bottomBarH) {
        auto keyArea = getLocalBounds();
        keyArea.removeFromBottom(bottomBarH);
        auto leftArea = keyArea.removeFromLeft(leftPanelW);

        if (e.getPosition().getX() < 24) {
            float ratio = 1.0f - static_cast<float>(e.getPosition().getY() - leftArea.getY()) / static_cast<float>(leftArea.getHeight());
            int pbVal = static_cast<int>((ratio - 0.5f) * 2.0f * 8191.0f);
            pbVal = juce::jlimit(-8192, 8191, pbVal);
            if (onPitchBend) onPitchBend(pbVal);
        } else {
            float ratio = 1.0f - static_cast<float>(e.getPosition().getY() - leftArea.getY()) / static_cast<float>(leftArea.getHeight());
            int modVal = juce::jlimit(0, 127, static_cast<int>(ratio * 127));
            if (onModWheel) onModWheel(modVal);
        }
        repaint();
        return;
    }

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
    // Release pitch bend on mouse up (return to center)
    if (onPitchBend) onPitchBend(0);
    // Clear button press highlight
    if (pressedButton_ >= 0) {
        pressedButton_ = -1;
        repaint();
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

    constexpr int leftPanelW = 50;
    constexpr int bottomBarH = 24;

    // Exclude left panel and bottom bar
    auto keyArea = bounds;
    keyArea.removeFromLeft(leftPanelW);
    keyArea.removeFromBottom(bottomBarH);

    if (!keyArea.contains(pos)) return -1;

    int localX = pos.getX() - keyArea.getX();
    int localY = pos.getY() - keyArea.getY();

    const int whiteKeyWidth = keyArea.getWidth() / NUM_WHITE_KEYS;
    const int blackKeyWidth = whiteKeyWidth * 2 / 3;
    const int blackKeyHeight = keyArea.getHeight() * 3 / 5;

    static const int whiteNotes[] = {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17};
    static const int blackNotes[] = {1, 3, 6, 8, 10, 13, 15};
    static const int blackWhitePos[] = {1, 2, 4, 5, 6, 8, 9};

    // Check black keys first (they're on top)
    if (localY < blackKeyHeight) {
        for (int i = 0; i < 7; ++i) {
            int xPos = blackWhitePos[i] * whiteKeyWidth - blackKeyWidth / 2;
            if (localX >= xPos && localX < xPos + blackKeyWidth) {
                return baseOctave_ * 12 + blackNotes[i];
            }
        }
    }

    // Check white keys
    int whiteIdx = localX / whiteKeyWidth;
    if (whiteIdx >= 0 && whiteIdx < NUM_WHITE_KEYS) {
        return baseOctave_ * 12 + whiteNotes[whiteIdx];
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
