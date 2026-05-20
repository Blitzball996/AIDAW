#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace magda {

/**
 * @brief Small rotary knob for send level on each channel strip.
 *
 * Features:
 * - Rotary knob showing send level (0.0 to 1.0)
 * - Pre/post fader toggle (click label to switch)
 * - Tooltip showing destination bus name
 * - Right-click to remove send
 */
class SendKnob : public juce::Component {
  public:
    SendKnob();
    ~SendKnob() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;

    // State
    void setLevel(float level);
    float getLevel() const noexcept { return level_; }

    void setPreFader(bool preFader);
    bool isPreFader() const noexcept { return preFader_; }

    void setSendName(const juce::String& name);
    juce::String getSendName() const { return sendName_; }

    void setDestinationName(const juce::String& name);

    // Callbacks
    std::function<void(float level)> onLevelChanged;
    std::function<void(bool preFader)> onPreFaderToggled;
    std::function<void()> onRemoveRequested;

  private:
    float level_ = 0.0f;
    bool preFader_ = false;
    juce::String sendName_ = "Send";
    juce::String destName_;

    // Drag state
    float dragStartLevel_ = 0.0f;
    int dragStartY_ = 0;
    bool isDragging_ = false;

    float levelToAngle(float level) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SendKnob)
};

}  // namespace magda
