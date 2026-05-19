#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace aidaw {

class PianoRollGridComponent : public juce::Component {
public:
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff12121e));
        drawGrid(g);
    }

    void setZoom(double ppb, int noteH) { pixelsPerBeat = ppb; noteHeight = noteH; repaint(); }

private:
    double pixelsPerBeat = 30.0;
    int noteHeight = 12;
    int lowestNote = 24;
    int highestNote = 96;

    void drawGrid(juce::Graphics& g) {
        int h = getHeight();
        int w = getWidth();

        // Horizontal note lines
        for (int note = lowestNote; note <= highestNote; note++) {
            int y = (highestNote - note) * noteHeight;
            bool isBlack = (note % 12 == 1 || note % 12 == 3 || note % 12 == 6 || note % 12 == 8 || note % 12 == 10);
            g.setColour(isBlack ? juce::Colour(0xff0e0e1a) : juce::Colour(0xff161626));
            g.fillRect(0, y, w, noteHeight);
            g.setColour(juce::Colour(0xff2a2a3e));
            g.drawHorizontalLine(y, 0.0f, (float)w);
        }

        // Vertical beat lines
        for (double b = 0; b * pixelsPerBeat < w; b += 1.0) {
            int x = (int)(b * pixelsPerBeat);
            bool isBar = ((int)b % 4 == 0);
            g.setColour(isBar ? juce::Colour(0xff3d3d5c) : juce::Colour(0xff222238));
            g.drawVerticalLine(x, 0.0f, (float)h);
        }
    }
};

class NoteComponent : public juce::Component {
public:
    NoteComponent(int pitch, double startBeat, double lengthBeats, int velocity)
        : notePitch(pitch), start(startBeat), length(lengthBeats), vel(velocity) {}

    void paint(juce::Graphics& g) override {
        float alpha = vel / 127.0f;
        g.setColour(juce::Colour(0xff6699ff).withAlpha(0.5f + alpha * 0.5f));
        g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 2.0f);
        g.setColour(juce::Colour(0xff99bbff));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 2.0f, 0.5f);
    }

private:
    int notePitch;
    double start, length;
    int vel;
};

}  // namespace aidaw
