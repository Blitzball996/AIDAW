#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace aidaw {

class ClipComponent : public juce::Component {
public:
    ClipComponent(const juce::String& name, juce::Colour colour)
        : clipName(name), clipColour(colour) {}

    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds().reduced(1);
        g.setColour(clipColour);
        g.fillRoundedRectangle(area.toFloat(), 3.0f);
        g.setColour(clipColour.brighter(0.3f));
        g.drawRoundedRectangle(area.toFloat(), 3.0f, 1.0f);
        g.setColour(juce::Colours::white);
        g.setFont(11.0f);
        g.drawText(clipName, area.reduced(4, 2), juce::Justification::topLeft);
    }

private:
    juce::String clipName;
    juce::Colour clipColour;
};

}  // namespace aidaw
