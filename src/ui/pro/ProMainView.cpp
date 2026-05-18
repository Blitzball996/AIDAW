#include "ProMainView.hpp"

namespace aidaw {

// ============================================================================
// TrackLaneComponent
// ============================================================================

TrackLaneComponent::TrackLaneComponent(int trackIndex, const juce::String& name)
    : index(trackIndex), trackName(name) {
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.8);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(volumeSlider);

    muteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3d3d5c));
    soloButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3d3d5c));
    addAndMakeVisible(muteButton);
    addAndMakeVisible(soloButton);
}

void TrackLaneComponent::paint(juce::Graphics& g) {
    auto area = getLocalBounds();

    // Track header background
    g.setColour(juce::Colour(0xff252540));
    g.fillRect(area.removeFromLeft(180));

    // Track lane background
    g.setColour(juce::Colour(0xff1a1a2e));
    g.fillRect(area);

    // Track name
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText(trackName, 10, 8, 160, 20, juce::Justification::centredLeft);

    // Grid lines
    g.setColour(juce::Colour(0xff2d2d4a));
    for (int x = 180; x < getWidth(); x += 100)
        g.drawVerticalLine(x, 0.0f, (float)getHeight());

    // Bottom border
    g.setColour(juce::Colour(0xff3d3d5c));
    g.drawHorizontalLine(getHeight() - 1, 0.0f, (float)getWidth());
}

void TrackLaneComponent::resized() {
    auto header = getLocalBounds().removeFromLeft(180).reduced(4);
    header.removeFromTop(24);
    auto controls = header.removeFromTop(24);
    muteButton.setBounds(controls.removeFromLeft(28).reduced(1));
    soloButton.setBounds(controls.removeFromLeft(28).reduced(1));
    volumeSlider.setBounds(controls.reduced(2));
}

// ============================================================================
// ProMainView
// ============================================================================

ProMainView::ProMainView() {
    // Transport buttons
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d8a4e));
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a4a6a));
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffcc3333));
    loopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a4a6a));

    playButton.onClick = [this] { onPlay(); };
    stopButton.onClick = [this] { onStop(); };
    recordButton.onClick = [this] { onRecord(); };

    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(recordButton);
    addAndMakeVisible(loopButton);

    // Labels
    tempoLabel.setText("120.0 BPM", juce::dontSendNotification);
    tempoLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
    tempoLabel.setFont(juce::Font(16.0f));
    addAndMakeVisible(tempoLabel);

    positionLabel.setText("1.1.0", juce::dontSendNotification);
    positionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    positionLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    addAndMakeVisible(positionLabel);

    modeLabel.setText("PRO", juce::dontSendNotification);
    modeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff6699ff));
    modeLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(modeLabel);

    // Track area
    trackViewport.setViewedComponent(&trackContainer, false);
    trackViewport.setScrollBarsShown(true, false);
    addAndMakeVisible(trackViewport);

    addTrackButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3d5c8a));
    addTrackButton.onClick = [this] { onAddTrack(); };
    addAndMakeVisible(addTrackButton);

    // Master volume
    masterVolume.setSliderStyle(juce::Slider::LinearHorizontal);
    masterVolume.setRange(0.0, 1.0, 0.01);
    masterVolume.setValue(0.8);
    masterVolume.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    addAndMakeVisible(masterVolume);

    masterLabel.setText("Master", juce::dontSendNotification);
    masterLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(masterLabel);

    // Default tracks
    onAddTrack();
    onAddTrack();

    startTimerHz(15);
}

ProMainView::~ProMainView() {
    stopTimer();
}

void ProMainView::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff12121e));

    // Transport bar background
    auto transportArea = getLocalBounds().removeFromTop(48);
    g.setColour(juce::Colour(0xff1e1e32));
    g.fillRect(transportArea);

    // Bottom bar background
    auto bottomArea = getLocalBounds().removeFromBottom(40);
    g.setColour(juce::Colour(0xff1e1e32));
    g.fillRect(bottomArea);
}

void ProMainView::resized() {
    auto area = getLocalBounds();

    // Transport bar (top)
    auto transport = area.removeFromTop(48).reduced(8, 8);
    modeLabel.setBounds(transport.removeFromLeft(40));
    transport.removeFromLeft(10);
    playButton.setBounds(transport.removeFromLeft(36));
    stopButton.setBounds(transport.removeFromLeft(36));
    recordButton.setBounds(transport.removeFromLeft(36));
    loopButton.setBounds(transport.removeFromLeft(36));
    transport.removeFromLeft(20);
    positionLabel.setBounds(transport.removeFromLeft(80));
    tempoLabel.setBounds(transport.removeFromLeft(100));
    addTrackButton.setBounds(transport.removeFromRight(120));

    // Bottom bar (master)
    auto bottom = area.removeFromBottom(40).reduced(8, 6);
    masterLabel.setBounds(bottom.removeFromLeft(60));
    masterVolume.setBounds(bottom.removeFromLeft(200));

    // Track viewport (middle)
    trackViewport.setBounds(area);
    int totalHeight = trackLanes.size() * 60;
    trackContainer.setSize(trackViewport.getWidth(), juce::jmax(totalHeight, area.getHeight()));

    for (int i = 0; i < trackLanes.size(); i++) {
        trackLanes[i]->setBounds(0, i * 60, trackContainer.getWidth(), 60);
    }
}

void ProMainView::timerCallback() {
    // Update position display (placeholder)
    positionLabel.setText("1.1.0", juce::dontSendNotification);
}

void ProMainView::onPlay() {
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff40cc70));
}

void ProMainView::onStop() {
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d8a4e));
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffcc3333));
}

void ProMainView::onRecord() {
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffff4444));
}

void ProMainView::onAddTrack() {
    int idx = trackLanes.size();
    auto name = "Track " + juce::String(idx + 1);
    auto* lane = new TrackLaneComponent(idx, name);
    trackLanes.add(lane);
    trackContainer.addAndMakeVisible(lane);
    resized();
}

void ProMainView::rebuildTrackLanes() {
    trackContainer.removeAllChildren();
    trackLanes.clear();
}

}  // namespace aidaw
