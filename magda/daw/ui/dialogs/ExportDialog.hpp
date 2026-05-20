#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace magda {

/**
 * @brief Enhanced export dialog with format selection, stems export, and progress.
 *
 * Provides:
 * - Format selector (WAV/FLAC/MP3)
 * - Sample rate and bit depth options
 * - Start/end time fields with "Entire Project" checkbox
 * - Export Stems checkbox
 * - Progress bar during export
 * - Calls ExportManager for actual rendering
 */
class ExportDialog : public juce::Component, private juce::Timer {
  public:
    struct ExportSettings {
        juce::String format = "WAV";       // WAV, FLAC, MP3
        int sampleRate = 48000;
        int bitDepth = 24;
        bool entireProject = true;
        double startTime = 0.0;            // Seconds
        double endTime = 0.0;              // Seconds
        bool exportStems = false;
        juce::File outputFile;
    };

    ExportDialog();
    ~ExportDialog() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    ExportSettings getSettings() const;

    // Set project duration for time range validation
    void setProjectDuration(double durationSeconds);

    // Progress control (called from export thread)
    void setProgress(float progress);
    void setExporting(bool exporting);

    // Callbacks
    std::function<void(const ExportSettings&)> onExport;
    std::function<void()> onCancel;

    static void showDialog(juce::Component* parent,
                           std::function<void(const ExportSettings&)> exportCallback,
                           double projectDuration = 0.0);

  private:
    void timerCallback() override;
    void onEntireProjectToggled();
    void updateBitDepthForFormat();

    // Format
    juce::Label formatLabel_;
    juce::ComboBox formatCombo_;

    // Sample rate
    juce::Label sampleRateLabel_;
    juce::ComboBox sampleRateCombo_;

    // Bit depth
    juce::Label bitDepthLabel_;
    juce::ComboBox bitDepthCombo_;

    // Time range
    juce::ToggleButton entireProjectCheck_;
    juce::Label startTimeLabel_;
    juce::TextEditor startTimeEditor_;
    juce::Label endTimeLabel_;
    juce::TextEditor endTimeEditor_;

    // Stems
    juce::ToggleButton exportStemsCheck_;

    // Progress
    juce::ProgressBar progressBar_;
    double progressValue_ = 0.0;
    bool isExporting_ = false;

    // Buttons
    juce::TextButton exportButton_;
    juce::TextButton cancelButton_;

    double projectDuration_ = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExportDialog)
};

}  // namespace magda
