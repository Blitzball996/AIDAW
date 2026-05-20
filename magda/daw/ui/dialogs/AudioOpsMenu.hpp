#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace magda {

/**
 * @brief Right-click context menu for audio clip operations.
 *
 * Provides quick access to:
 * - Strip Silence (with threshold/duration settings)
 * - Normalize (with target dB)
 * - Reverse
 * - Fade In/Out (with duration)
 *
 * Each operation shows a small settings dialog before executing.
 * Calls AudioOperations backend for processing.
 */
class AudioOpsMenu {
  public:
    struct OperationResult {
        bool success = false;
        juce::String outputPath;
        juce::String errorMessage;
    };

    // Callbacks for each operation
    std::function<void(float thresholdDb, double minDurationMs)> onStripSilence;
    std::function<void(float targetDb)> onNormalize;
    std::function<void()> onReverse;
    std::function<void(double durationMs)> onFadeIn;
    std::function<void(double durationMs)> onFadeOut;

    /**
     * @brief Show the audio operations context menu at the given position.
     * @param parent Component to attach the menu to
     * @param screenPosition Position in screen coordinates
     */
    void show(juce::Component* parent, juce::Point<int> screenPosition);

  private:
    void showStripSilenceDialog(juce::Component* parent);
    void showNormalizeDialog(juce::Component* parent);
    void showFadeInDialog(juce::Component* parent);
    void showFadeOutDialog(juce::Component* parent);
};

/**
 * @brief Small settings dialog for Strip Silence parameters.
 */
class StripSilenceDialog : public juce::Component {
  public:
    StripSilenceDialog();
    void resized() override;
    void paint(juce::Graphics& g) override;

    std::function<void(float thresholdDb, double minDurationMs)> onApply;

  private:
    juce::Label thresholdLabel_;
    juce::Slider thresholdSlider_;
    juce::Label durationLabel_;
    juce::Slider durationSlider_;
    juce::TextButton applyButton_;
    juce::TextButton cancelButton_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StripSilenceDialog)
};

/**
 * @brief Small settings dialog for Normalize parameters.
 */
class NormalizeDialog : public juce::Component {
  public:
    NormalizeDialog();
    void resized() override;
    void paint(juce::Graphics& g) override;

    std::function<void(float targetDb)> onApply;

  private:
    juce::Label targetLabel_;
    juce::Slider targetSlider_;
    juce::TextButton applyButton_;
    juce::TextButton cancelButton_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NormalizeDialog)
};

/**
 * @brief Small settings dialog for Fade duration.
 */
class FadeDialog : public juce::Component {
  public:
    FadeDialog(const juce::String& title);
    void resized() override;
    void paint(juce::Graphics& g) override;

    std::function<void(double durationMs)> onApply;

  private:
    juce::String title_;
    juce::Label durationLabel_;
    juce::Slider durationSlider_;
    juce::TextButton applyButton_;
    juce::TextButton cancelButton_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FadeDialog)
};

}  // namespace magda
