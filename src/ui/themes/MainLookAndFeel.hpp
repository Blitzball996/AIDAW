#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "DarkTheme.hpp"
#include "FontManager.hpp"

namespace aidaw {

/**
 * Custom LookAndFeel for AIDAW's main window chrome.
 * Handles title bar drawing and window button styling.
 */
class MainLookAndFeel : public juce::LookAndFeel_V4 {
public:
    static constexpr int kTitleBarHeight = 22;

    MainLookAndFeel() = default;
    ~MainLookAndFeel() override = default;

    void drawDocumentWindowTitleBar(juce::DocumentWindow& window,
                                    juce::Graphics& g, int w, int h,
                                    int titleSpaceX, int titleSpaceW,
                                    const juce::Image* icon,
                                    bool drawTitleTextOnLeft) override;

    juce::Button* createDocumentWindowButton(int buttonType) override;

private:
    class GlyphButton : public juce::Button {
    public:
        GlyphButton(const juce::String& name, juce::Colour c,
                    const juce::Path& normal, const juce::Path& toggled);
        void paintButton(juce::Graphics& g, bool isHighlighted, bool isDown) override;

    private:
        juce::Colour colour_;
        juce::Path normalShape_, toggledShape_;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlyphButton)
    };
};

}  // namespace aidaw
