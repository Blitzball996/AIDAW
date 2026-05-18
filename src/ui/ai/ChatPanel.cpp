#include "ChatPanel.hpp"

namespace aidaw {

ChatPanel::ChatPanel() {
    inputEditor.setMultiLine(false);
    inputEditor.setReturnKeyStartsNewLine(false);
    inputEditor.setTextToShowWhenEmpty("Type a music instruction...",
                                       juce::Colours::grey);
    inputEditor.onReturnKey = [this] { onSend(); };
    addAndMakeVisible(inputEditor);

    sendButton.onClick = [this] { onSend(); };
    addAndMakeVisible(sendButton);

    messageViewport.setViewedComponent(&messageContainer, false);
    addAndMakeVisible(messageViewport);
}

void ChatPanel::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff181828));
    g.setColour(juce::Colour(0xff3d3d5c));
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 8.0f, 1.0f);
}

void ChatPanel::resized() {
    auto area = getLocalBounds().reduced(8);
    auto inputArea = area.removeFromBottom(36);
    sendButton.setBounds(inputArea.removeFromRight(70).reduced(2));
    inputEditor.setBounds(inputArea.reduced(2));
    messageViewport.setBounds(area.reduced(0, 4));
}

void ChatPanel::addMessage(const std::string& role, const std::string& content) {
    messages.push_back({role, content});
    refreshMessages();
}

void ChatPanel::clear() {
    messages.clear();
    refreshMessages();
}

void ChatPanel::onSend() {
    auto text = inputEditor.getText().toStdString();
    if (text.empty()) return;

    addMessage("user", text);
    inputEditor.clear();

    // TODO: send to AiEngine and display response
    addMessage("ai", "[Processing: " + text + "]");
}

void ChatPanel::refreshMessages() {
    // Simple text display for now
    messageContainer.removeAllChildren();
    int y = 0;
    for (const auto& msg : messages) {
        auto* label = new juce::Label();
        auto prefix = (msg.role == "user") ? "> " : "AI: ";
        label->setText(juce::String(prefix + msg.content), juce::dontSendNotification);
        label->setColour(juce::Label::textColourId,
            msg.role == "user" ? juce::Colours::lightblue : juce::Colours::lightgreen);
        label->setBounds(0, y, messageViewport.getWidth() - 10, 24);
        messageContainer.addAndMakeVisible(label);
        y += 26;
    }
    messageContainer.setSize(messageViewport.getWidth(), y);
}

}  // namespace aidaw
