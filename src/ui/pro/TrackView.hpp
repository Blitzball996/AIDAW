#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace aidaw {

class TrackView : public juce::Component {
public:
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff2d2d3d));
        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds());
    }
};

}  // namespace aidaw
