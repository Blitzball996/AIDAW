#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace magda::daw::ui {

/**
 * @brief Dialog for MIDI quantization settings.
 *
 * Provides:
 * - Grid size selector (1/4, 1/8, 1/16, 1/32)
 * - Strength slider (0-100%)
 * - Swing slider (0-100%)
 * - Groove template dropdown
 * - Preview button (applies non-destructively for audition)
 */
class QuantizeDialog : public juce::Component {
  public:
    struct Settings {
        int gridSize = 2;          // 0=1/4, 1=1/8, 2=1/16, 3=1/32
        float strength = 100.0f;   // 0-100%
        float swing = 0.0f;        // 0-100%
        int grooveTemplate = 0;    // 0=None, 1+=template index
    };

    QuantizeDialog();
    ~QuantizeDialog() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    Settings getSettings() const;

    // Callbacks
    std::function<void(const Settings&)> onApply;
    std::function<void(const Settings&)> onPreview;

    static void showDialog(juce::Component* parent,
                           std::function<void(const Settings&)> applyCallback,
                           std::function<void(const Settings&)> previewCallback = nullptr);

  private:
    // Grid size
    juce::Label gridLabel_;
    juce::ComboBox gridCombo_;

    // Strength
    juce::Label strengthLabel_;
    juce::Slider strengthSlider_;

    // Swing
    juce::Label swingLabel_;
    juce::Slider swingSlider_;

    // Groove template
    juce::Label grooveLabel_;
    juce::ComboBox grooveCombo_;

    // Buttons
    juce::TextButton previewButton_;
    juce::TextButton applyButton_;
    juce::TextButton cancelButton_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuantizeDialog)
};

}  // namespace magda::daw::ui
