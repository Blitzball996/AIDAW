#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace aidaw {

class MixerView : public juce::Component {
public:
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff252535));
    }
};

}  // namespace aidaw
