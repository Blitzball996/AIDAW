#include "VCAStrip.hpp"

#include "../../themes/DarkTheme.hpp"
#include "../../themes/FontManager.hpp"

namespace magda {

VCAStrip::VCAStrip() {
    // Name label
    nameLabel_.setText("VCA 1", juce::dontSendNotification);
    nameLabel_.setFont(FontManager::getInstance().getUIFontBold(11.0f));
    nameLabel_.setJustificationType(juce::Justification::centred);
    nameLabel_.setEditable(false, true);  // Double-click to edit
    nameLabel_.onTextChange = [this]() {
        if (onNameChanged) onNameChanged(nameLabel_.getText());
    };
    addAndMakeVisible(nameLabel_);

    // Fader
    faderSlider_ = std::make_unique<daw::ui::TextSlider>(daw::ui::TextSlider::Format::Decibels);
    faderSlider_->setRange(-60.0, 6.0, 0.1);
    faderSlider_->setValue(0.0, juce::dontSendNotification);
    faderSlider_->setOrientation(daw::ui::TextSlider::Orientation::Vertical);
    faderSlider_->onValueChanged = [this](double value) {
        // Convert dB to linear gain
        float gain = juce::Decibels::decibelsToGain(static_cast<float>(value), -60.0f);
        if (onLevelChanged) onLevelChanged(gain);
    };
    addAndMakeVisible(*faderSlider_);

    // Mute button
    muteButton_.setButtonText("M");
    muteButton_.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    muteButton_.setClickingTogglesState(true);
    muteButton_.onClick = [this]() {
        muted_ = muteButton_.getToggleState();
        if (onMuteToggled) onMuteToggled(muted_);
    };
    addAndMakeVisible(muteButton_);

    // Solo button
    soloButton_.setButtonText("S");
    soloButton_.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow);
    soloButton_.setClickingTogglesState(true);
    soloButton_.onClick = [this]() {
        soloed_ = soloButton_.getToggleState();
        if (onSoloToggled) onSoloToggled(soloed_);
    };
    addAndMakeVisible(soloButton_);

    // Track count indicator
    trackCountLabel_.setText("0", juce::dontSendNotification);
    trackCountLabel_.setFont(FontManager::getInstance().getUIFont(10.0f));
    trackCountLabel_.setJustificationType(juce::Justification::centred);
    trackCountLabel_.setColour(juce::Label::textColourId,
                               DarkTheme::getColour(DarkTheme::TEXT_SECONDARY));
    addAndMakeVisible(trackCountLabel_);
}

VCAStrip::~VCAStrip() = default;

void VCAStrip::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();

    // Background
    g.setColour(DarkTheme::getColour(DarkTheme::SURFACE));
    g.fillRoundedRectangle(bounds.toFloat(), 2.0f);

    // Border
    g.setColour(DarkTheme::getColour(DarkTheme::BORDER));
    g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), 2.0f, 1.0f);

    // VCA indicator stripe at top
    g.setColour(juce::Colour(0xFF9C27B0));  // Purple for VCA
    g.fillRect(bounds.getX() + 2, bounds.getY() + 2, bounds.getWidth() - 4, 3);
}

void VCAStrip::resized() {
    auto bounds = getLocalBounds().reduced(4);

    // Name at top
    nameLabel_.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(4);

    // Track count badge
    trackCountLabel_.setBounds(bounds.removeFromTop(14));
    bounds.removeFromTop(4);

    // Mute/Solo buttons at bottom
    auto buttonArea = bounds.removeFromBottom(22);
    int halfW = buttonArea.getWidth() / 2;
    muteButton_.setBounds(buttonArea.removeFromLeft(halfW).reduced(1));
    soloButton_.setBounds(buttonArea.reduced(1));
    bounds.removeFromBottom(4);

    // Fader takes remaining space
    faderSlider_->setBounds(bounds);
}

void VCAStrip::setVCAName(const juce::String& name) {
    nameLabel_.setText(name, juce::dontSendNotification);
}

juce::String VCAStrip::getVCAName() const {
    return nameLabel_.getText();
}

void VCAStrip::setLevel(float level) {
    float db = juce::Decibels::gainToDecibels(level, -60.0f);
    faderSlider_->setValue(static_cast<double>(db), juce::dontSendNotification);
}

float VCAStrip::getLevel() const {
    return juce::Decibels::decibelsToGain(
        static_cast<float>(faderSlider_->getValue()), -60.0f);
}

void VCAStrip::setMute(bool mute) {
    muted_ = mute;
    muteButton_.setToggleState(mute, juce::dontSendNotification);
}

void VCAStrip::setSolo(bool solo) {
    soloed_ = solo;
    soloButton_.setToggleState(solo, juce::dontSendNotification);
}

void VCAStrip::setAssignedTrackCount(int count) {
    assignedTrackCount_ = count;
    trackCountLabel_.setText(juce::String(count) + " tracks", juce::dontSendNotification);
}

}  // namespace magda
