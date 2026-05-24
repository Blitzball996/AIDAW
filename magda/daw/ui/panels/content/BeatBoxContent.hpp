#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <functional>
#include <memory>

#include "PanelContent.hpp"

namespace magda::daw::ui {

/**
 * @brief Beat box / drum machine step sequencer panel.
 *
 * Displays a 16x8 grid of toggle buttons (steps x instruments).
 * - Instrument labels on left (Kick, Snare, HiHat, etc.)
 * - Step numbers on top
 * - Play/Stop button, pattern selector dropdown
 * - Swing knob
 *
 * Connects to the BeatBox backend for pattern storage and playback.
 */
class BeatBoxContent : public PanelContent {
  public:
    static constexpr int NUM_STEPS = 16;
    static constexpr int NUM_INSTRUMENTS = 8;

    BeatBoxContent();
    ~BeatBoxContent() override;

    PanelContentType getContentType() const override { return PanelContentType::BeatBox; }
    PanelContentInfo getContentInfo() const override {
        return {PanelContentType::BeatBox, "Beat Box", "Drum pattern sequencer", "DrumGrid"};
    }

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

    // Pattern data access
    void setStep(int step, int instrument, int velocity);
    int getStepVelocity(int step, int instrument) const;
    void clearAll();

    // Transport
    void setPlaying(bool playing);
    void setCurrentStep(int step);

    // Swing
    void setSwing(float swing);
    float getSwing() const noexcept { return swing_; }

    // Callbacks
    std::function<void(int step, int instrument, int velocity)> onStepToggled;
    std::function<void(bool playing)> onPlayStateChanged;
    std::function<void(int patternIndex)> onPatternSelected;
    std::function<void(float swing)> onSwingChanged;

  private:
    // Grid data: velocity per step/instrument (0 = off)
    std::array<std::array<int, NUM_INSTRUMENTS>, NUM_STEPS> grid_{};

    // Instrument names
    static const std::array<juce::String, NUM_INSTRUMENTS> instrumentNames_;

    // UI state
    bool playing_ = false;
    int currentStep_ = -1;
    float swing_ = 0.0f;

    // UI components
    juce::TextButton playButton_;
    juce::TextButton clearButton_;
    juce::ComboBox patternSelector_;
    juce::Slider swingSlider_;
    juce::Label swingLabel_;

    // Layout
    static constexpr int LABEL_WIDTH = 80;
    static constexpr int HEADER_HEIGHT = 36;
    static constexpr int CELL_SIZE = 28;

    juce::Rectangle<int> getCellRect(int step, int instrument) const;
    void toggleCell(int step, int instrument);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BeatBoxContent)
};

}  // namespace magda::daw::ui
