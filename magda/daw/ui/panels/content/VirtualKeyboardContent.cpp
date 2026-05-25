#include "VirtualKeyboardContent.hpp"

#include <tracktion_engine/tracktion_engine.h>

#include "../../themes/DarkTheme.hpp"
#include "../../themes/FontManager.hpp"
#include "../../../audio/AudioBridge.hpp"
#include "../../../audio/MidiBridge.hpp"
#include "../../../core/TrackManager.hpp"
#include "../../../engine/AudioEngine.hpp"

namespace magda::daw::ui {

VirtualKeyboardContent::VirtualKeyboardContent() {
    setName("Virtual Keyboard");

    addAndMakeVisible(keyboard_);

    // Octave controls
    octaveLabel_.setText("Oct", juce::dontSendNotification);
    octaveLabel_.setFont(FontManager::getInstance().getUIFont(11.0f));
    octaveLabel_.setColour(juce::Label::textColourId, DarkTheme::getSecondaryTextColour());
    addAndMakeVisible(octaveLabel_);

    octaveValueLabel_.setText(juce::String(keyboard_.getBaseOctave()), juce::dontSendNotification);
    octaveValueLabel_.setFont(FontManager::getInstance().getUIFont(12.0f));
    octaveValueLabel_.setColour(juce::Label::textColourId, DarkTheme::getTextColour());
    octaveValueLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(octaveValueLabel_);

    octaveDownBtn_.onClick = [this]() {
        keyboard_.setBaseOctave(keyboard_.getBaseOctave() - 1);
        octaveValueLabel_.setText(juce::String(keyboard_.getBaseOctave()),
                                  juce::dontSendNotification);
    };
    octaveDownBtn_.setTooltip("Z key");
    addAndMakeVisible(octaveDownBtn_);

    octaveUpBtn_.onClick = [this]() {
        keyboard_.setBaseOctave(keyboard_.getBaseOctave() + 1);
        octaveValueLabel_.setText(juce::String(keyboard_.getBaseOctave()),
                                  juce::dontSendNotification);
    };
    octaveUpBtn_.setTooltip("X key");
    addAndMakeVisible(octaveUpBtn_);

    // Velocity slider
    velocityLabel_.setText("Vel", juce::dontSendNotification);
    velocityLabel_.setFont(FontManager::getInstance().getUIFont(11.0f));
    velocityLabel_.setColour(juce::Label::textColourId, DarkTheme::getSecondaryTextColour());
    addAndMakeVisible(velocityLabel_);

    velocitySlider_.setRange(1, 127, 1);
    velocitySlider_.setValue(keyboard_.getVelocity(), juce::dontSendNotification);
    velocitySlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    velocitySlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 32, 20);
    velocitySlider_.setColour(juce::Slider::backgroundColourId,
                              DarkTheme::getColour(DarkTheme::SURFACE));
    velocitySlider_.setColour(juce::Slider::trackColourId,
                              DarkTheme::getColour(DarkTheme::ACCENT_BLUE));
    velocitySlider_.onValueChange = [this]() {
        keyboard_.setVelocity(static_cast<int>(velocitySlider_.getValue()));
    };
    addAndMakeVisible(velocitySlider_);

    // Record button
    recordBtn_.setClickingTogglesState(true);
    recordBtn_.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red.darker(0.2f));
    recordBtn_.onClick = [this]() {
        bool rec = recordBtn_.getToggleState();
        double pos = 0.0;
        if (keyboard_.getTransportPosition)
            pos = keyboard_.getTransportPosition();
        keyboard_.setRecording(rec, pos);
    };
    addAndMakeVisible(recordBtn_);

    // Pop-out button
    popOutBtn_.setTooltip("Open as floating window");
    popOutBtn_.onClick = [this]() { popOutToWindow(); };
    addAndMakeVisible(popOutBtn_);

    // Help label with key mapping hints
    helpLabel_.setText("A-; ': C-F (2 oct) | Black: W E T Y U O P | Z/X: Oct-/+",
                       juce::dontSendNotification);
    helpLabel_.setFont(FontManager::getInstance().getUIFont(9.0f));
    helpLabel_.setColour(juce::Label::textColourId,
                         DarkTheme::getSecondaryTextColour().withAlpha(0.6f));
    addAndMakeVisible(helpLabel_);

    // Connect MIDI output — send notes via AudioBridge's QWERTY MIDI device
    keyboard_.onNoteOn = [](int noteNumber, int velocity) {
        auto* engine = magda::TrackManager::getInstance().getAudioEngine();
        if (!engine) return;
        auto* bridge = engine->getAudioBridge();
        if (!bridge) return;
        auto* vmd = bridge->getQwertyMidiDevice();
        if (!vmd) return;
        vmd->keyboardState.noteOn(1, noteNumber, static_cast<float>(velocity) / 127.0f);
        auto* midiBridge = engine->getMidiBridge();
        if (midiBridge)
            midiBridge->broadcastSynthesizedNote(vmd->getDeviceID(), noteNumber, velocity, true);
    };
    keyboard_.onNoteOff = [](int noteNumber) {
        auto* engine = magda::TrackManager::getInstance().getAudioEngine();
        if (!engine) return;
        auto* bridge = engine->getAudioBridge();
        if (!bridge) return;
        auto* vmd = bridge->getQwertyMidiDevice();
        if (!vmd) return;
        vmd->keyboardState.noteOff(1, noteNumber, 0.0f);
        auto* midiBridge = engine->getMidiBridge();
        if (midiBridge)
            midiBridge->broadcastSynthesizedNote(vmd->getDeviceID(), noteNumber, 0, false);
    };
    keyboard_.onPitchBend = [](int value) {
        auto* engine = magda::TrackManager::getInstance().getAudioEngine();
        if (!engine) return;
        auto* bridge = engine->getAudioBridge();
        if (!bridge) return;
        auto* vmd = bridge->getQwertyMidiDevice();
        if (!vmd) return;
        int midiValue = value + 8192;
        auto msg = juce::MidiMessage::pitchWheel(1, midiValue);
        vmd->handleIncomingMidiMessage(msg, tracktion::MPESourceID());
    };
    keyboard_.onSustain = [](bool on) {
        auto* engine = magda::TrackManager::getInstance().getAudioEngine();
        if (!engine) return;
        auto* bridge = engine->getAudioBridge();
        if (!bridge) return;
        auto* vmd = bridge->getQwertyMidiDevice();
        if (!vmd) return;
        auto msg = juce::MidiMessage::controllerEvent(1, 64, on ? 127 : 0);
        vmd->handleIncomingMidiMessage(msg, tracktion::MPESourceID());
    };
    keyboard_.onModWheel = [](int modValue) {
        auto* engine = magda::TrackManager::getInstance().getAudioEngine();
        if (!engine) return;
        auto* bridge = engine->getAudioBridge();
        if (!bridge) return;
        auto* vmd = bridge->getQwertyMidiDevice();
        if (!vmd) return;
        auto msg = juce::MidiMessage::controllerEvent(1, 1, modValue);
        vmd->handleIncomingMidiMessage(msg, tracktion::MPESourceID());
    };
    keyboard_.onVelocityChanged = [this](int vel) {
        velocitySlider_.setValue(vel, juce::dontSendNotification);
    };
}

void VirtualKeyboardContent::paint(juce::Graphics& g) {
    g.fillAll(DarkTheme::getPanelBackgroundColour());
}

void VirtualKeyboardContent::resized() {
    auto bounds = getLocalBounds();

    // Toolbar at top
    auto toolbar = bounds.removeFromTop(TOOLBAR_HEIGHT);
    toolbar = toolbar.reduced(4, 2);

    octaveLabel_.setBounds(toolbar.removeFromLeft(24));
    octaveDownBtn_.setBounds(toolbar.removeFromLeft(22).reduced(1));
    octaveValueLabel_.setBounds(toolbar.removeFromLeft(20));
    octaveUpBtn_.setBounds(toolbar.removeFromLeft(22).reduced(1));

    toolbar.removeFromLeft(12);
    velocityLabel_.setBounds(toolbar.removeFromLeft(24));
    velocitySlider_.setBounds(toolbar.removeFromLeft(120).reduced(0, 2));

    toolbar.removeFromLeft(12);
    recordBtn_.setBounds(toolbar.removeFromLeft(36).reduced(1));

    toolbar.removeFromLeft(4);
    popOutBtn_.setBounds(toolbar.removeFromLeft(22).reduced(1));

    toolbar.removeFromLeft(8);
    helpLabel_.setBounds(toolbar);

    // Keyboard fills the rest
    keyboard_.setBounds(bounds);
}

void VirtualKeyboardContent::onActivated() {
    if (auto* topLevel = getTopLevelComponent())
        topLevel->addKeyListener(&keyboard_);
    keyboard_.setWantsKeyboardFocus(true);
    keyboard_.grabKeyboardFocus();
}

void VirtualKeyboardContent::onDeactivated() {
    if (auto* topLevel = getTopLevelComponent())
        topLevel->removeKeyListener(&keyboard_);
}

// Static instance of floating window
static std::unique_ptr<VirtualKeyboardWindow> s_floatingWindow;

void VirtualKeyboardContent::popOutToWindow() {
    if (!s_floatingWindow)
        s_floatingWindow = std::make_unique<VirtualKeyboardWindow>();
    s_floatingWindow->setVisible(true);
    s_floatingWindow->toFront(true);
}

VirtualKeyboardWindow::VirtualKeyboardWindow()
    : juce::DocumentWindow("Virtual Keyboard",
                           DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND),
                           juce::DocumentWindow::closeButton) {
    setContentNonOwned(&content_, false);
    setSize(700, 160);
    setResizable(true, false);
    setUsingNativeTitleBar(true);
    setAlwaysOnTop(true);
    centreWithSize(700, 160);
    content_.onActivated();
}

}  // namespace magda::daw::ui
