#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace aidaw {

class ArrangementView : public juce::Component {
public:
    ArrangementView();
    void paint(juce::Graphics& g) override;
    void resized() override;

    void setZoom(double pixelsPerBeat) { ppb = pixelsPerBeat; repaint(); }
    void setScrollPosition(double beats) { scrollPos = beats; repaint(); }
    void setTrackCount(int count) { trackCount = count; repaint(); }

private:
    double ppb = 20.0;
    double scrollPos = 0.0;
    int trackCount = 4;
    int trackHeight = 60;

    void drawGrid(juce::Graphics& g);
    void drawTrackLanes(juce::Graphics& g);
    void drawPlayhead(juce::Graphics& g);
};

}  // namespace aidaw
