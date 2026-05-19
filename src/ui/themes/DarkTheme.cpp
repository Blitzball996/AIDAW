#include "DarkTheme.hpp"

namespace aidaw {

void DarkTheme::applyToLookAndFeel(juce::LookAndFeel_V4& laf) {
    laf.setColour(juce::ResizableWindow::backgroundColourId, getColour(BACKGROUND));
    laf.setColour(juce::TextButton::buttonColourId, getColour(BUTTON_NORMAL));
    laf.setColour(juce::TextButton::buttonOnColourId, getColour(BUTTON_ACTIVE));
    laf.setColour(juce::TextButton::textColourOffId, getColour(TEXT_PRIMARY));
    laf.setColour(juce::TextButton::textColourOnId, getColour(TEXT_PRIMARY));
    laf.setColour(juce::Label::textColourId, getColour(TEXT_PRIMARY));
    laf.setColour(juce::Slider::thumbColourId, getColour(ACCENT_BLUE));
    laf.setColour(juce::Slider::trackColourId, getColour(SURFACE));
    laf.setColour(juce::ScrollBar::thumbColourId, getColour(BORDER));
}

}  // namespace aidaw
