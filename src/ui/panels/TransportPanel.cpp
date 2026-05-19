#include "TransportPanel.hpp"

namespace aidaw {

TransportPanel::TransportPanel() {
    playBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d8a4e));
    stopBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a4a6a));
    recBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffcc3333));
    loopBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a4a6a));
    rewindBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a4a6a));
    ffwdBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a4a6a));

    playBtn.onClick = [this] { if (onPlay) onPlay(); };
    stopBtn.onClick = [this] { if (onStop) onStop(); };
    recBtn.onClick = [this] { if (onRecord) onRecord(); };
    loopBtn.onClick = [this] { if (onLoop) onLoop(); };

    for (auto* b : {&rewindBtn, &playBtn, &stopBtn, &recBtn, &ffwdBtn, &loopBtn})
        addAndMakeVisible(b);

    tempoLabel.setText("120.0 BPM", juce::dontSendNotification);
    tempoLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
    posLabel.setText("1.1.000", juce::dontSendNotification);
    posLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    posLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    timeSigLabel.setText("4/4", juce::dontSendNotification);
    timeSigLabel.setColour(juce::Label::textColourId, juce::Colours::grey);

    addAndMakeVisible(tempoLabel);
    addAndMakeVisible(posLabel);
    addAndMakeVisible(timeSigLabel);
}

void TransportPanel::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff1a1a2e));
    g.setColour(juce::Colour(0xff3d3d5c));
    g.drawLine(0, (float)getHeight(), (float)getWidth(), (float)getHeight());
}

void TransportPanel::resized() {
    auto area = getLocalBounds().reduced(6, 4);
    int btnW = 32;
    rewindBtn.setBounds(area.removeFromLeft(btnW).reduced(1));
    playBtn.setBounds(area.removeFromLeft(btnW).reduced(1));
    stopBtn.setBounds(area.removeFromLeft(btnW).reduced(1));
    recBtn.setBounds(area.removeFromLeft(btnW).reduced(1));
    ffwdBtn.setBounds(area.removeFromLeft(btnW).reduced(1));
    loopBtn.setBounds(area.removeFromLeft(btnW).reduced(1));
    area.removeFromLeft(16);
    posLabel.setBounds(area.removeFromLeft(100));
    tempoLabel.setBounds(area.removeFromLeft(90));
    timeSigLabel.setBounds(area.removeFromLeft(40));
}

void TransportPanel::setPlaying(bool p) {
    isPlaying = p;
    playBtn.setColour(juce::TextButton::buttonColourId,
        p ? juce::Colour(0xff40cc70) : juce::Colour(0xff2d8a4e));
}

void TransportPanel::setRecording(bool r) {
    isRecording = r;
    recBtn.setColour(juce::TextButton::buttonColourId,
        r ? juce::Colour(0xffff4444) : juce::Colour(0xffcc3333));
}

void TransportPanel::setLooping(bool l) {
    isLooping = l;
    loopBtn.setColour(juce::TextButton::buttonColourId,
        l ? juce::Colour(0xff6699ff) : juce::Colour(0xff4a4a6a));
}

void TransportPanel::setTempo(double bpm) {
    tempoLabel.setText(juce::String(bpm, 1) + " BPM", juce::dontSendNotification);
}

void TransportPanel::setPosition(double beats) {
    int bar = (int)(beats / 4.0) + 1;
    int beat = (int)std::fmod(beats, 4.0) + 1;
    int tick = (int)(std::fmod(beats, 1.0) * 1000);
    posLabel.setText(juce::String(bar) + "." + juce::String(beat) + "." +
        juce::String(tick).paddedLeft('0', 3), juce::dontSendNotification);
}

void TransportPanel::setTimeSignature(int num, int den) {
    timeSigLabel.setText(juce::String(num) + "/" + juce::String(den), juce::dontSendNotification);
}

}  // namespace aidaw
