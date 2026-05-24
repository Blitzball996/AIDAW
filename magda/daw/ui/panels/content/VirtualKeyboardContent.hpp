#pragma once

#include <memory>

#include "PanelContent.hpp"
#include "ui/components/keyboard/VirtualKeyboard.hpp"

namespace magda::daw::ui {

/**
 * @brief PanelContent wrapper for VirtualKeyboard component
 *
 * Wraps the VirtualKeyboard (computer-key → MIDI) as a dockable panel.
 * Includes octave selector, velocity slider, and record toggle.
 */
class VirtualKeyboardContent : public PanelContent {
  public:
    VirtualKeyboardContent();
    ~VirtualKeyboardContent() override = default;

    PanelContentType getContentType() const override {
        return PanelContentType::VirtualKeyboard;
    }

    PanelContentInfo getContentInfo() const override {
        return {PanelContentType::VirtualKeyboard, "Keyboard", "Virtual MIDI keyboard",
                "Keyboard"};
    }

    void paint(juce::Graphics& g) override;
    void resized() override;

    void onActivated() override;
    void onDeactivated() override;

    magda::VirtualKeyboard& getKeyboard() { return keyboard_; }

  private:
    magda::VirtualKeyboard keyboard_;

    juce::Label octaveLabel_;
    juce::TextButton octaveDownBtn_{"-"};
    juce::TextButton octaveUpBtn_{"+"};
    juce::Label octaveValueLabel_;

    juce::Label velocityLabel_;
    juce::Slider velocitySlider_;

    juce::TextButton recordBtn_{"Rec"};
    juce::Label helpLabel_;

    static constexpr int TOOLBAR_HEIGHT = 28;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VirtualKeyboardContent)
};

}  // namespace magda::daw::ui
