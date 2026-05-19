#include "ArrangementView.hpp"

namespace aidaw {

ArrangementView::ArrangementView() {}

void ArrangementView::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff12121e));
    drawGrid(g);
    drawTrackLanes(g);
    drawPlayhead(g);
}

void ArrangementView::resized() {}

void ArrangementView::drawGrid(juce::Graphics& g) {
    g.setColour(juce::Colour(0xff2a2a3e));
    int w = getWidth();
    int h = getHeight();

    // Vertical beat lines
    double beatWidth = ppb;
    double startBeat = scrollPos;
    for (double b = startBeat; (b - startBeat) * beatWidth < w; b += 1.0) {
        int x = (int)((b - startBeat) * beatWidth);
        bool isBar = ((int)b % 4 == 0);
        g.setColour(isBar ? juce::Colour(0xff3d3d5c) : juce::Colour(0xff222238));
        g.drawVerticalLine(x, 0.0f, (float)h);
    }

    // Horizontal track dividers
    for (int i = 0; i <= trackCount; i++) {
        int y = i * trackHeight;
        g.setColour(juce::Colour(0xff3d3d5c));
        g.drawHorizontalLine(y, 0.0f, (float)w);
    }
}

void ArrangementView::drawTrackLanes(juce::Graphics& g) {
    for (int i = 0; i < trackCount; i++) {
        int y = i * trackHeight;
        g.setColour(juce::Colour(0xff1a1a2e).withAlpha(i % 2 == 0 ? 1.0f : 0.8f));
        g.fillRect(0, y + 1, getWidth(), trackHeight - 1);
    }
}

void ArrangementView::drawPlayhead(juce::Graphics& g) {
    g.setColour(juce::Colours::white);
    int x = (int)(10.0 * ppb); // placeholder position
    g.drawVerticalLine(x, 0.0f, (float)getHeight());
}

}  // namespace aidaw
