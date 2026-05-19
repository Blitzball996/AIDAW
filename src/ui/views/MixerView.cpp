#include "MixerView.hpp"

namespace aidaw {

MixerView::ChannelStrip::ChannelStrip() {
    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.8);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(volumeSlider);

    panKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    panKnob.setRange(-1.0, 1.0, 0.01);
    panKnob.setValue(0.0);
    panKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(panKnob);

    muteBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3d3d5c));
    soloBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3d3d5c));
    addAndMakeVisible(muteBtn);
    addAndMakeVisible(soloBtn);

    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(nameLabel);
}

void MixerView::ChannelStrip::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff1e1e32));
    g.setColour(juce::Colour(0xff3d3d5c));
    g.drawRect(getLocalBounds());

    // Simple meter bars
    auto meterArea = getLocalBounds().removeFromRight(8).reduced(1, 40);
    g.setColour(juce::Colour(0xff2d8a4e));
    int meterH = (int)(meterArea.getHeight() * data.peakL);
    g.fillRect(meterArea.getX(), meterArea.getBottom() - meterH, 3, meterH);
    g.fillRect(meterArea.getX() + 4, meterArea.getBottom() - (int)(meterArea.getHeight() * data.peakR), 3, (int)(meterArea.getHeight() * data.peakR));
}

void MixerView::ChannelStrip::resized() {
    auto area = getLocalBounds().reduced(4);
    nameLabel.setBounds(area.removeFromBottom(20));
    auto btns = area.removeFromBottom(24);
    muteBtn.setBounds(btns.removeFromLeft(btns.getWidth() / 2).reduced(1));
    soloBtn.setBounds(btns.reduced(1));
    panKnob.setBounds(area.removeFromBottom(40).reduced(4));
    volumeSlider.setBounds(area.reduced(8, 4));
}

void MixerView::ChannelStrip::updateFromData() {
    volumeSlider.setValue(data.volume, juce::dontSendNotification);
    panKnob.setValue(data.pan, juce::dontSendNotification);
    nameLabel.setText(data.name, juce::dontSendNotification);
    muteBtn.setColour(juce::TextButton::buttonColourId,
        data.muted ? juce::Colour(0xffcc6633) : juce::Colour(0xff3d3d5c));
    soloBtn.setColour(juce::TextButton::buttonColourId,
        data.solo ? juce::Colour(0xffcccc33) : juce::Colour(0xff3d3d5c));
}

MixerView::MixerView() {
    viewport.setViewedComponent(&container, false);
    viewport.setScrollBarsShown(false, true);
    addAndMakeVisible(viewport);

    masterVolume.setSliderStyle(juce::Slider::LinearVertical);
    masterVolume.setRange(0.0, 1.0, 0.01);
    masterVolume.setValue(0.8);
    masterVolume.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    addAndMakeVisible(masterVolume);

    masterLabel.setText("Master", juce::dontSendNotification);
    masterLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    masterLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(masterLabel);
}

void MixerView::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff14141e));
}

void MixerView::resized() {
    auto area = getLocalBounds();
    auto masterArea = area.removeFromRight(70);
    masterLabel.setBounds(masterArea.removeFromBottom(20));
    masterVolume.setBounds(masterArea.reduced(10, 20));
    viewport.setBounds(area);

    int stripW = 70;
    container.setSize(strips.size() * stripW, area.getHeight());
    for (int i = 0; i < strips.size(); i++)
        strips[i]->setBounds(i * stripW, 0, stripW, container.getHeight());
}

void MixerView::setChannels(const std::vector<ChannelStripData>& channels) {
    strips.clear();
    for (const auto& ch : channels) {
        auto* strip = new ChannelStrip();
        strip->data = ch;
        strip->updateFromData();
        strips.add(strip);
        container.addAndMakeVisible(strip);
    }
    resized();
}

void MixerView::updateMeter(int trackId, float peakL, float peakR) {
    for (auto* strip : strips) {
        if (strip->data.trackId == trackId) {
            strip->data.peakL = peakL;
            strip->data.peakR = peakR;
            strip->repaint();
            break;
        }
    }
}

}  // namespace aidaw
