#include "QuantizeDialog.hpp"

#include "../../themes/DarkTheme.hpp"
#include "../../themes/FontManager.hpp"

namespace magda::daw::ui {

QuantizeDialog::QuantizeDialog() {
    // Grid size
    gridLabel_.setText("Grid Size:", juce::dontSendNotification);
    gridLabel_.setFont(FontManager::getInstance().getUIFontBold(13.0f));
    addAndMakeVisible(gridLabel_);

    gridCombo_.addItem("1/4 (Quarter)", 1);
    gridCombo_.addItem("1/8 (Eighth)", 2);
    gridCombo_.addItem("1/16 (Sixteenth)", 3);
    gridCombo_.addItem("1/32 (Thirty-second)", 4);
    gridCombo_.setSelectedId(3, juce::dontSendNotification);  // Default 1/16
    addAndMakeVisible(gridCombo_);

    // Strength
    strengthLabel_.setText("Strength:", juce::dontSendNotification);
    strengthLabel_.setFont(FontManager::getInstance().getUIFontBold(13.0f));
    addAndMakeVisible(strengthLabel_);

    strengthSlider_.setRange(0.0, 100.0, 1.0);
    strengthSlider_.setValue(100.0);
    strengthSlider_.setTextValueSuffix("%");
    strengthSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    strengthSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    addAndMakeVisible(strengthSlider_);

    // Swing
    swingLabel_.setText("Swing:", juce::dontSendNotification);
    swingLabel_.setFont(FontManager::getInstance().getUIFontBold(13.0f));
    addAndMakeVisible(swingLabel_);

    swingSlider_.setRange(0.0, 100.0, 1.0);
    swingSlider_.setValue(0.0);
    swingSlider_.setTextValueSuffix("%");
    swingSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    swingSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    addAndMakeVisible(swingSlider_);

    // Groove template
    grooveLabel_.setText("Groove:", juce::dontSendNotification);
    grooveLabel_.setFont(FontManager::getInstance().getUIFontBold(13.0f));
    addAndMakeVisible(grooveLabel_);

    grooveCombo_.addItem("None", 1);
    grooveCombo_.addItem("MPC Swing 54%", 2);
    grooveCombo_.addItem("MPC Swing 58%", 3);
    grooveCombo_.addItem("MPC Swing 62%", 4);
    grooveCombo_.addItem("MPC Swing 66%", 5);
    grooveCombo_.addItem("Laid Back", 6);
    grooveCombo_.addItem("Push", 7);
    grooveCombo_.addItem("Humanize Light", 8);
    grooveCombo_.addItem("Humanize Heavy", 9);
    grooveCombo_.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(grooveCombo_);

    // Preview button
    previewButton_.setButtonText("Preview");
    previewButton_.onClick = [this]() {
        if (onPreview) onPreview(getSettings());
    };
    addAndMakeVisible(previewButton_);

    // Apply button
    applyButton_.setButtonText("Apply");
    applyButton_.onClick = [this]() {
        if (onApply) onApply(getSettings());
        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->exitModalState(0);
    };
    addAndMakeVisible(applyButton_);

    // Cancel button
    cancelButton_.setButtonText("Cancel");
    cancelButton_.onClick = [this]() {
        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->exitModalState(0);
    };
    addAndMakeVisible(cancelButton_);

    setSize(400, 260);
}

QuantizeDialog::~QuantizeDialog() = default;

void QuantizeDialog::paint(juce::Graphics& g) {
    g.fillAll(DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND));
}

void QuantizeDialog::resized() {
    auto bounds = getLocalBounds().reduced(20);

    auto row = [&]() {
        auto r = bounds.removeFromTop(28);
        bounds.removeFromTop(8);
        return r;
    };

    // Grid size
    auto gridRow = row();
    gridLabel_.setBounds(gridRow.removeFromLeft(90));
    gridRow.removeFromLeft(8);
    gridCombo_.setBounds(gridRow);

    // Strength
    auto strRow = row();
    strengthLabel_.setBounds(strRow.removeFromLeft(90));
    strRow.removeFromLeft(8);
    strengthSlider_.setBounds(strRow);

    // Swing
    auto swRow = row();
    swingLabel_.setBounds(swRow.removeFromLeft(90));
    swRow.removeFromLeft(8);
    swingSlider_.setBounds(swRow);

    // Groove
    auto grRow = row();
    grooveLabel_.setBounds(grRow.removeFromLeft(90));
    grRow.removeFromLeft(8);
    grooveCombo_.setBounds(grRow);

    bounds.removeFromTop(8);

    // Buttons
    auto buttonRow = bounds.removeFromBottom(30);
    cancelButton_.setBounds(buttonRow.removeFromRight(80));
    buttonRow.removeFromRight(8);
    applyButton_.setBounds(buttonRow.removeFromRight(80));
    buttonRow.removeFromRight(8);
    previewButton_.setBounds(buttonRow.removeFromRight(80));
}

QuantizeDialog::Settings QuantizeDialog::getSettings() const {
    Settings s;
    s.gridSize = gridCombo_.getSelectedId() - 1;  // 0-based
    s.strength = static_cast<float>(strengthSlider_.getValue());
    s.swing = static_cast<float>(swingSlider_.getValue());
    s.grooveTemplate = grooveCombo_.getSelectedId() - 1;  // 0 = None
    return s;
}

void QuantizeDialog::showDialog(juce::Component* parent,
                                std::function<void(const Settings&)> applyCallback,
                                std::function<void(const Settings&)> previewCallback) {
    juce::ignoreUnused(parent);
    auto* dialog = new QuantizeDialog();
    dialog->onApply = applyCallback;
    dialog->onPreview = previewCallback;

    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle = "Quantize MIDI";
    options.dialogBackgroundColour = DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND);
    options.content.setOwned(dialog);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.launchAsync();
}

}  // namespace magda::daw::ui
