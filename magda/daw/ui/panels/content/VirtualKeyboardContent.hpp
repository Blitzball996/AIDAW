#pragma once

#include <memory>

#include "PanelContent.hpp"
#include "ui/components/keyboard/VirtualKeyboard.hpp"

namespace magda::daw::ui {

class VirtualKeyboardWindow;

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

    void popOutToWindow();

  private:
    magda::VirtualKeyboard keyboard_;

    juce::Label octaveLabel_;
    juce::TextButton octaveDownBtn_{"-"};
    juce::TextButton octaveUpBtn_{"+"};
    juce::Label octaveValueLabel_;

    juce::Label velocityLabel_;
    juce::Slider velocitySlider_;

    juce::TextButton recordBtn_{"Rec"};
    juce::TextButton popOutBtn_{"^"};
    juce::Label helpLabel_;

    static constexpr int TOOLBAR_HEIGHT = 28;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VirtualKeyboardContent)
};

class VirtualKeyboardWindow : public juce::DocumentWindow {
  public:
    VirtualKeyboardWindow();
    void closeButtonPressed() override { setVisible(false); }

    magda::VirtualKeyboard& getKeyboard() { return content_.getKeyboard(); }

  private:
    VirtualKeyboardContent content_;
};

}  // namespace magda::daw::ui
