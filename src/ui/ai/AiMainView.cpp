#include "AiMainView.hpp"

namespace aidaw {

AiMainView::AiMainView() {
    titleLabel.setText("AI Composition Mode", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.0f));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    addAndMakeVisible(chatPanel);
    addAndMakeVisible(voiceButton);
}

void AiMainView::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff0f0f1a));
}

void AiMainView::resized() {
    auto area = getLocalBounds().reduced(20);
    titleLabel.setBounds(area.removeFromTop(50));
    voiceButton.setBounds(area.removeFromBottom(50).reduced(area.getWidth() / 4, 5));
    chatPanel.setBounds(area.reduced(0, 10));
}

}  // namespace aidaw
