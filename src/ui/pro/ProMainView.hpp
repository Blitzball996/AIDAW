#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace aidaw {

class ProMainView : public juce::Component {
public:
    ProMainView();
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label titleLabel;
    juce::TextButton playButton{"Play"};
    juce::TextButton stopButton{"Stop"};
    juce::TextButton recordButton{"Record"};
};

}  // namespace aidaw
