#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "ChatPanel.hpp"

namespace aidaw {

class AiMainView : public juce::Component {
public:
    AiMainView();
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label titleLabel;
    ChatPanel chatPanel;
    juce::TextButton voiceButton{"Hold to Speak"};
};

}  // namespace aidaw
