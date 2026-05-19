#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace aidaw {

class LevelMeter : public juce::Component {
public:
    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds();
        g.fillAll(juce::Colour(0xff0a0a14));

        int meterH = (int)(area.getHeight() * peakL);
        auto colour = peakL > 0.9f ? juce::Colours::red
                    : peakL > 0.7f ? juce::Colours::yellow
                    : juce::Colour(0xff2d8a4e);

        g.setColour(colour);
        g.fillRect(area.getX(), area.getBottom() - meterH, area.getWidth() / 2 - 1, meterH);

        int meterHR = (int)(area.getHeight() * peakR);
        g.fillRect(area.getX() + area.getWidth() / 2 + 1, area.getBottom() - meterHR, area.getWidth() / 2 - 1, meterHR);
    }

    void setPeaks(float l, float r) { peakL = l; peakR = r; repaint(); }

private:
    float peakL = 0.0f, peakR = 0.0f;
};

}  // namespace aidaw
