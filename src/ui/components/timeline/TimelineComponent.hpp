#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace aidaw {

class TimelineComponent : public juce::Component {
public:
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff1a1a2e));
        drawRuler(g);
    }

    void setPixelsPerBeat(double ppb) { pixelsPerBeat = ppb; repaint(); }
    void setScrollOffset(double beats) { scrollOffset = beats; repaint(); }
    void setTempo(double bpm) { tempo = bpm; repaint(); }

private:
    double pixelsPerBeat = 20.0;
    double scrollOffset = 0.0;
    double tempo = 120.0;

    void drawRuler(juce::Graphics& g) {
        int h = getHeight();
        double beatW = pixelsPerBeat;
        g.setFont(10.0f);

        for (double b = scrollOffset; (b - scrollOffset) * beatW < getWidth(); b += 1.0) {
            int x = (int)((b - scrollOffset) * beatW);
            bool isBar = ((int)b % 4 == 0);

            if (isBar) {
                g.setColour(juce::Colours::white.withAlpha(0.7f));
                g.drawVerticalLine(x, 0.0f, (float)h);
                int barNum = (int)(b / 4.0) + 1;
                g.drawText(juce::String(barNum), x + 2, 0, 30, 12, juce::Justification::left);
            } else {
                g.setColour(juce::Colour(0xff3d3d5c));
                g.drawVerticalLine(x, (float)(h / 2), (float)h);
            }
        }
    }
};

class TimeRuler : public juce::Component {
public:
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff252540));
        g.setColour(juce::Colour(0xff3d3d5c));
        g.drawLine(0, (float)getHeight(), (float)getWidth(), (float)getHeight());
    }
};

}  // namespace aidaw
