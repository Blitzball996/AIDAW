#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <vector>
#include <string>

namespace aidaw {

class ChatPanel : public juce::Component {
public:
    ChatPanel();
    void paint(juce::Graphics& g) override;
    void resized() override;

    void addMessage(const std::string& role, const std::string& content);
    void clear();

private:
    struct ChatMessage {
        std::string role;
        std::string content;
    };

    std::vector<ChatMessage> messages;
    juce::TextEditor inputEditor;
    juce::TextButton sendButton{"Send"};
    juce::Viewport messageViewport;
    juce::Component messageContainer;

    void onSend();
    void refreshMessages();
};

}  // namespace aidaw
