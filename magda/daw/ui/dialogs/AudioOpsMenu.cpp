#include "AudioOpsMenu.hpp"

#include "../themes/DarkTheme.hpp"
#include "../themes/FontManager.hpp"

namespace magda {

//==============================================================================
// AudioOpsMenu
//==============================================================================

void AudioOpsMenu::show(juce::Component* parent, juce::Point<int> screenPosition) {
    juce::PopupMenu menu;

    menu.addItem(1, "Strip Silence...");
    menu.addItem(2, "Normalize...");
    menu.addItem(3, "Reverse");
    menu.addSeparator();
    menu.addItem(4, "Fade In...");
    menu.addItem(5, "Fade Out...");

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetScreenArea(
        juce::Rectangle<int>(screenPosition.x, screenPosition.y, 1, 1)),
        [this, parent](int result) {
            switch (result) {
                case 1: showStripSilenceDialog(parent); break;
                case 2: showNormalizeDialog(parent); break;
                case 3: if (onReverse) onReverse(); break;
                case 4: showFadeInDialog(parent); break;
                case 5: showFadeOutDialog(parent); break;
                default: break;
            }
        });
}

void AudioOpsMenu::showStripSilenceDialog(juce::Component* parent) {
    auto* dialog = new StripSilenceDialog();
    dialog->onApply = onStripSilence;

    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle = "Strip Silence";
    options.dialogBackgroundColour = DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND);
    options.content.setOwned(dialog);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.componentToCentreAround = parent;
    options.launchAsync();
}

void AudioOpsMenu::showNormalizeDialog(juce::Component* parent) {
    auto* dialog = new NormalizeDialog();
    dialog->onApply = onNormalize;

    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle = "Normalize";
    options.dialogBackgroundColour = DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND);
    options.content.setOwned(dialog);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.componentToCentreAround = parent;
    options.launchAsync();
}

void AudioOpsMenu::showFadeInDialog(juce::Component* parent) {
    auto* dialog = new FadeDialog("Fade In");
    dialog->onApply = onFadeIn;

    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle = "Fade In";
    options.dialogBackgroundColour = DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND);
    options.content.setOwned(dialog);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.componentToCentreAround = parent;
    options.launchAsync();
}

void AudioOpsMenu::showFadeOutDialog(juce::Component* parent) {
    auto* dialog = new FadeDialog("Fade Out");
    dialog->onApply = onFadeOut;

    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle = "Fade Out";
    options.dialogBackgroundColour = DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND);
    options.content.setOwned(dialog);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.componentToCentreAround = parent;
    options.launchAsync();
}

//==============================================================================
// StripSilenceDialog
//==============================================================================

StripSilenceDialog::StripSilenceDialog() {
    thresholdLabel_.setText("Threshold (dB):", juce::dontSendNotification);
    thresholdLabel_.setFont(FontManager::getInstance().getUIFont(13.0f));
    addAndMakeVisible(thresholdLabel_);

    thresholdSlider_.setRange(-80.0, -20.0, 1.0);
    thresholdSlider_.setValue(-60.0);
    thresholdSlider_.setTextValueSuffix(" dB");
    thresholdSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    thresholdSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible(thresholdSlider_);

    durationLabel_.setText("Min Duration (ms):", juce::dontSendNotification);
    durationLabel_.setFont(FontManager::getInstance().getUIFont(13.0f));
    addAndMakeVisible(durationLabel_);

    durationSlider_.setRange(10.0, 2000.0, 10.0);
    durationSlider_.setValue(200.0);
    durationSlider_.setTextValueSuffix(" ms");
    durationSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    durationSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible(durationSlider_);

    applyButton_.setButtonText("Apply");
    applyButton_.onClick = [this]() {
        if (onApply)
            onApply(static_cast<float>(thresholdSlider_.getValue()),
                    durationSlider_.getValue());
        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->exitModalState(0);
    };
    addAndMakeVisible(applyButton_);

    cancelButton_.setButtonText("Cancel");
    cancelButton_.onClick = [this]() {
        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->exitModalState(0);
    };
    addAndMakeVisible(cancelButton_);

    setSize(350, 160);
}

void StripSilenceDialog::paint(juce::Graphics& g) {
    g.fillAll(DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND));
}

void StripSilenceDialog::resized() {
    auto bounds = getLocalBounds().reduced(16);

    auto row1 = bounds.removeFromTop(28);
    thresholdLabel_.setBounds(row1.removeFromLeft(130));
    thresholdSlider_.setBounds(row1);
    bounds.removeFromTop(8);

    auto row2 = bounds.removeFromTop(28);
    durationLabel_.setBounds(row2.removeFromLeft(130));
    durationSlider_.setBounds(row2);
    bounds.removeFromTop(16);

    auto buttonRow = bounds.removeFromBottom(28);
    cancelButton_.setBounds(buttonRow.removeFromRight(80));
    buttonRow.removeFromRight(8);
    applyButton_.setBounds(buttonRow.removeFromRight(80));
}

//==============================================================================
// NormalizeDialog
//==============================================================================

NormalizeDialog::NormalizeDialog() {
    targetLabel_.setText("Target Peak (dB):", juce::dontSendNotification);
    targetLabel_.setFont(FontManager::getInstance().getUIFont(13.0f));
    addAndMakeVisible(targetLabel_);

    targetSlider_.setRange(-6.0, 0.0, 0.1);
    targetSlider_.setValue(-0.3);
    targetSlider_.setTextValueSuffix(" dB");
    targetSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    targetSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible(targetSlider_);

    applyButton_.setButtonText("Apply");
    applyButton_.onClick = [this]() {
        if (onApply)
            onApply(static_cast<float>(targetSlider_.getValue()));
        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->exitModalState(0);
    };
    addAndMakeVisible(applyButton_);

    cancelButton_.setButtonText("Cancel");
    cancelButton_.onClick = [this]() {
        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->exitModalState(0);
    };
    addAndMakeVisible(cancelButton_);

    setSize(320, 120);
}

void NormalizeDialog::paint(juce::Graphics& g) {
    g.fillAll(DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND));
}

void NormalizeDialog::resized() {
    auto bounds = getLocalBounds().reduced(16);

    auto row1 = bounds.removeFromTop(28);
    targetLabel_.setBounds(row1.removeFromLeft(130));
    targetSlider_.setBounds(row1);
    bounds.removeFromTop(16);

    auto buttonRow = bounds.removeFromBottom(28);
    cancelButton_.setBounds(buttonRow.removeFromRight(80));
    buttonRow.removeFromRight(8);
    applyButton_.setBounds(buttonRow.removeFromRight(80));
}

//==============================================================================
// FadeDialog
//==============================================================================

FadeDialog::FadeDialog(const juce::String& title) : title_(title) {
    durationLabel_.setText("Duration (ms):", juce::dontSendNotification);
    durationLabel_.setFont(FontManager::getInstance().getUIFont(13.0f));
    addAndMakeVisible(durationLabel_);

    durationSlider_.setRange(10.0, 5000.0, 10.0);
    durationSlider_.setValue(500.0);
    durationSlider_.setTextValueSuffix(" ms");
    durationSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    durationSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible(durationSlider_);

    applyButton_.setButtonText("Apply");
    applyButton_.onClick = [this]() {
        if (onApply)
            onApply(durationSlider_.getValue());
        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->exitModalState(0);
    };
    addAndMakeVisible(applyButton_);

    cancelButton_.setButtonText("Cancel");
    cancelButton_.onClick = [this]() {
        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->exitModalState(0);
    };
    addAndMakeVisible(cancelButton_);

    setSize(320, 120);
}

void FadeDialog::paint(juce::Graphics& g) {
    g.fillAll(DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND));
}

void FadeDialog::resized() {
    auto bounds = getLocalBounds().reduced(16);

    auto row1 = bounds.removeFromTop(28);
    durationLabel_.setBounds(row1.removeFromLeft(110));
    durationSlider_.setBounds(row1);
    bounds.removeFromTop(16);

    auto buttonRow = bounds.removeFromBottom(28);
    cancelButton_.setBounds(buttonRow.removeFromRight(80));
    buttonRow.removeFromRight(8);
    applyButton_.setBounds(buttonRow.removeFromRight(80));
}

}  // namespace magda
