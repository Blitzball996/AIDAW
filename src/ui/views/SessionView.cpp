#include "SessionView.hpp"

namespace aidaw {

void SessionView::ClipSlot::paint(juce::Graphics& g) {
    auto area = getLocalBounds().reduced(2);
    if (hasClip) {
        g.setColour(isPlaying ? clipColour.brighter(0.3f) : clipColour);
        g.fillRoundedRectangle(area.toFloat(), 4.0f);
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.drawRoundedRectangle(area.toFloat(), 4.0f, 1.0f);
    } else {
        g.setColour(juce::Colour(0xff2a2a3e));
        g.fillRoundedRectangle(area.toFloat(), 4.0f);
        g.setColour(juce::Colour(0xff3d3d5c));
        g.drawRoundedRectangle(area.toFloat(), 4.0f, 0.5f);
    }
}

void SessionView::ClipSlot::mouseDown(const juce::MouseEvent&) {
    if (onClick) onClick();
}

SessionView::SessionView() {
    viewport.setViewedComponent(&grid, false);
    addAndMakeVisible(viewport);
    rebuildGrid();
}

void SessionView::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff0f0f1a));
}

void SessionView::resized() {
    viewport.setBounds(getLocalBounds());
    grid.setSize(trackCount * cellWidth, sceneCount * cellHeight);
    rebuildGrid();
}

void SessionView::rebuildGrid() {
    grid.removeAllChildren();
    slots.clear();

    for (int scene = 0; scene < sceneCount; scene++) {
        for (int track = 0; track < trackCount; track++) {
            auto* slot = new ClipSlot();
            slot->trackIdx = track;
            slot->sceneIdx = scene;
            slot->hasClip = (track + scene) % 3 == 0; // placeholder
            slot->setBounds(track * cellWidth, scene * cellHeight, cellWidth, cellHeight);
            slot->onClick = [this, track, scene] {
                if (onClipLaunch) onClipLaunch(track, scene);
            };
            slots.add(slot);
            grid.addAndMakeVisible(slot);
        }
    }
}

}  // namespace aidaw
