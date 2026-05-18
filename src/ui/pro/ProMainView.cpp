#include "ProMainView.hpp"

namespace aidaw {

ProMainView::ProMainView() {
    titleLabel.setText("Professional Mode", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(recordButton);
}

void ProMainView::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff1e1e2e));

    // Track area placeholder
    g.setColour(juce::Colour(0xff2d2d3d));
    g.fillRect(getLocalBounds().reduced(10).withTrimmedTop(80));
}

void ProMainView::resized() {
    auto area = getLocalBounds().reduced(10);
    auto top = area.removeFromTop(40);
    titleLabel.setBounds(top.removeFromLeft(300));

    auto transport = top.removeFromRight(300);
    playButton.setBounds(transport.removeFromLeft(80).reduced(2));
    stopButton.setBounds(transport.removeFromLeft(80).reduced(2));
    recordButton.setBounds(transport.removeFromLeft(80).reduced(2));
}

}  // namespace aidaw
