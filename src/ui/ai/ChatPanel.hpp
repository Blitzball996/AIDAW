#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <vector>
#include <string>
#include <thread>
#include <atomic>

namespace aidaw {

class AiEngine;

class ChatPanel : public juce::Component {
public:
    ChatPanel();
    ~ChatPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setAiEngine(AiEngine* engine) { aiEngine = engine; }
    void addMessage(const std::string& role, const std::string& content);
    void clear();

private:
    struct ChatMessage {
        std::string role;
        std::string content;
    };

    AiEngine* aiEngine = nullptr;
    std::vector<ChatMessage> messages;
    juce::TextEditor inputEditor;
    juce::TextButton sendButton{"Send"};
    juce::Viewport messageViewport;
    juce::Component messageContainer;
    std::thread workerThread;
    std::atomic<bool> processing{false};

    void onSend();
    void refreshMessages();
};

}  // namespace aidaw
