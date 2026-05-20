#include "ExportDialog.hpp"

#include "../themes/DarkTheme.hpp"
#include "../themes/FontManager.hpp"

namespace magda {

ExportDialog::ExportDialog() : progressBar_(progressValue_) {
    // Format selector
    formatLabel_.setText("Format:", juce::dontSendNotification);
    formatLabel_.setFont(FontManager::getInstance().getUIFontBold(13.0f));
    addAndMakeVisible(formatLabel_);

    formatCombo_.addItem("WAV", 1);
    formatCombo_.addItem("FLAC", 2);
    formatCombo_.addItem("MP3", 3);
    formatCombo_.setSelectedId(1, juce::dontSendNotification);
    formatCombo_.onChange = [this]() { updateBitDepthForFormat(); };
    addAndMakeVisible(formatCombo_);

    // Sample rate
    sampleRateLabel_.setText("Sample Rate:", juce::dontSendNotification);
    sampleRateLabel_.setFont(FontManager::getInstance().getUIFontBold(13.0f));
    addAndMakeVisible(sampleRateLabel_);

    sampleRateCombo_.addItem("44100 Hz", 1);
    sampleRateCombo_.addItem("48000 Hz", 2);
    sampleRateCombo_.addItem("96000 Hz", 3);
    sampleRateCombo_.addItem("192000 Hz", 4);
    sampleRateCombo_.setSelectedId(2, juce::dontSendNotification);
    addAndMakeVisible(sampleRateCombo_);

    // Bit depth
    bitDepthLabel_.setText("Bit Depth:", juce::dontSendNotification);
    bitDepthLabel_.setFont(FontManager::getInstance().getUIFontBold(13.0f));
    addAndMakeVisible(bitDepthLabel_);

    bitDepthCombo_.addItem("16-bit", 1);
    bitDepthCombo_.addItem("24-bit", 2);
    bitDepthCombo_.addItem("32-bit Float", 3);
    bitDepthCombo_.setSelectedId(2, juce::dontSendNotification);
    addAndMakeVisible(bitDepthCombo_);

    // Entire project checkbox
    entireProjectCheck_.setButtonText("Entire Project");
    entireProjectCheck_.setToggleState(true, juce::dontSendNotification);
    entireProjectCheck_.onClick = [this]() { onEntireProjectToggled(); };
    addAndMakeVisible(entireProjectCheck_);

    // Start/end time
    startTimeLabel_.setText("Start:", juce::dontSendNotification);
    startTimeLabel_.setFont(FontManager::getInstance().getUIFont(13.0f));
    addAndMakeVisible(startTimeLabel_);

    startTimeEditor_.setText("0:00.000");
    startTimeEditor_.setEnabled(false);
    addAndMakeVisible(startTimeEditor_);

    endTimeLabel_.setText("End:", juce::dontSendNotification);
    endTimeLabel_.setFont(FontManager::getInstance().getUIFont(13.0f));
    addAndMakeVisible(endTimeLabel_);

    endTimeEditor_.setText("0:00.000");
    endTimeEditor_.setEnabled(false);
    addAndMakeVisible(endTimeEditor_);

    // Export stems
    exportStemsCheck_.setButtonText("Export Stems (separate file per track)");
    exportStemsCheck_.setToggleState(false, juce::dontSendNotification);
    addAndMakeVisible(exportStemsCheck_);

    // Progress bar (hidden initially)
    progressBar_.setVisible(false);
    addAndMakeVisible(progressBar_);

    // Buttons
    exportButton_.setButtonText("Export");
    exportButton_.onClick = [this]() {
        if (onExport) onExport(getSettings());
    };
    addAndMakeVisible(exportButton_);

    cancelButton_.setButtonText("Cancel");
    cancelButton_.onClick = [this]() {
        if (onCancel) onCancel();
        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->exitModalState(0);
    };
    addAndMakeVisible(cancelButton_);

    setSize(460, 380);
}

ExportDialog::~ExportDialog() {
    stopTimer();
}

void ExportDialog::paint(juce::Graphics& g) {
    g.fillAll(DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND));
}

void ExportDialog::resized() {
    auto bounds = getLocalBounds().reduced(20);

    auto row = [&]() {
        auto r = bounds.removeFromTop(28);
        bounds.removeFromTop(8);
        return r;
    };

    // Format
    auto formatRow = row();
    formatLabel_.setBounds(formatRow.removeFromLeft(100));
    formatCombo_.setBounds(formatRow);

    // Sample rate
    auto srRow = row();
    sampleRateLabel_.setBounds(srRow.removeFromLeft(100));
    sampleRateCombo_.setBounds(srRow);

    // Bit depth
    auto bdRow = row();
    bitDepthLabel_.setBounds(bdRow.removeFromLeft(100));
    bitDepthCombo_.setBounds(bdRow);

    bounds.removeFromTop(4);

    // Entire project
    entireProjectCheck_.setBounds(row());

    // Start/end time
    auto timeRow = row();
    startTimeLabel_.setBounds(timeRow.removeFromLeft(40));
    startTimeEditor_.setBounds(timeRow.removeFromLeft(90));
    timeRow.removeFromLeft(20);
    endTimeLabel_.setBounds(timeRow.removeFromLeft(35));
    endTimeEditor_.setBounds(timeRow.removeFromLeft(90));

    bounds.removeFromTop(4);

    // Stems
    exportStemsCheck_.setBounds(row());

    bounds.removeFromTop(8);

    // Progress bar
    progressBar_.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(12);

    // Buttons
    auto buttonRow = bounds.removeFromBottom(32);
    cancelButton_.setBounds(buttonRow.removeFromRight(90));
    buttonRow.removeFromRight(10);
    exportButton_.setBounds(buttonRow.removeFromRight(90));
}

ExportDialog::ExportSettings ExportDialog::getSettings() const {
    ExportSettings s;

    switch (formatCombo_.getSelectedId()) {
        case 1: s.format = "WAV"; break;
        case 2: s.format = "FLAC"; break;
        case 3: s.format = "MP3"; break;
        default: s.format = "WAV"; break;
    }

    switch (sampleRateCombo_.getSelectedId()) {
        case 1: s.sampleRate = 44100; break;
        case 2: s.sampleRate = 48000; break;
        case 3: s.sampleRate = 96000; break;
        case 4: s.sampleRate = 192000; break;
        default: s.sampleRate = 48000; break;
    }

    switch (bitDepthCombo_.getSelectedId()) {
        case 1: s.bitDepth = 16; break;
        case 2: s.bitDepth = 24; break;
        case 3: s.bitDepth = 32; break;
        default: s.bitDepth = 24; break;
    }

    s.entireProject = entireProjectCheck_.getToggleState();
    s.exportStems = exportStemsCheck_.getToggleState();

    // Parse time fields if not entire project
    if (!s.entireProject) {
        // Simple mm:ss.ms parsing
        auto parseTime = [](const juce::String& text) -> double {
            auto parts = juce::StringArray::fromTokens(text, ":", "");
            if (parts.size() == 2)
                return parts[0].getDoubleValue() * 60.0 + parts[1].getDoubleValue();
            return text.getDoubleValue();
        };
        s.startTime = parseTime(startTimeEditor_.getText());
        s.endTime = parseTime(endTimeEditor_.getText());
    }

    return s;
}

void ExportDialog::setProjectDuration(double durationSeconds) {
    projectDuration_ = durationSeconds;
    int mins = static_cast<int>(durationSeconds) / 60;
    double secs = durationSeconds - mins * 60.0;
    endTimeEditor_.setText(juce::String(mins) + ":" + juce::String(secs, 3));
}

void ExportDialog::setProgress(float progress) {
    progressValue_ = static_cast<double>(progress);
    if (!isTimerRunning()) startTimerHz(30);
}

void ExportDialog::setExporting(bool exporting) {
    isExporting_ = exporting;
    progressBar_.setVisible(exporting);
    exportButton_.setEnabled(!exporting);
    if (!exporting) stopTimer();
}

void ExportDialog::timerCallback() {
    progressBar_.repaint();
}

void ExportDialog::onEntireProjectToggled() {
    bool entire = entireProjectCheck_.getToggleState();
    startTimeEditor_.setEnabled(!entire);
    endTimeEditor_.setEnabled(!entire);
}

void ExportDialog::updateBitDepthForFormat() {
    int formatId = formatCombo_.getSelectedId();
    if (formatId == 3) {
        // MP3 - only 16-bit makes sense
        bitDepthCombo_.setSelectedId(1, juce::dontSendNotification);
        bitDepthCombo_.setEnabled(false);
    } else {
        bitDepthCombo_.setEnabled(true);
    }
}

void ExportDialog::showDialog(juce::Component* parent,
                              std::function<void(const ExportSettings&)> exportCallback,
                              double projectDuration) {
    juce::ignoreUnused(parent);
    auto* dialog = new ExportDialog();
    dialog->onExport = exportCallback;
    if (projectDuration > 0.0)
        dialog->setProjectDuration(projectDuration);

    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle = "Export Audio";
    options.dialogBackgroundColour = DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND);
    options.content.setOwned(dialog);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.launchAsync();
}

}  // namespace magda
