#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <vector>

namespace aidaw {

struct ChannelStripData {
    int trackId = 0;
    juce::String name;
    float volume = 0.8f;
    float pan = 0.0f;
    float peakL = 0.0f, peakR = 0.0f;
    bool muted = false, solo = false;
};

class MixerView : public juce::Component {
public:
    MixerView();
    void paint(juce::Graphics& g) override;
    void resized() override;

    void setChannels(const std::vector<ChannelStripData>& channels);
    void updateMeter(int trackId, float peakL, float peakR);

    std::function<void(int, float)> onVolumeChange;
    std::function<void(int, float)> onPanChange;
    std::function<void(int, bool)> onMuteToggle;
    std::function<void(int, bool)> onSoloToggle;

private:
    struct ChannelStrip : public juce::Component {
        ChannelStripData data;
        juce::Slider volumeSlider, panKnob;
        juce::TextButton muteBtn{"M"}, soloBtn{"S"};
        juce::Label nameLabel;

        ChannelStrip();
        void paint(juce::Graphics& g) override;
        void resized() override;
        void updateFromData();
    };

    juce::OwnedArray<ChannelStrip> strips;
    juce::Viewport viewport;
    juce::Component container;

    // Master
    juce::Slider masterVolume;
    juce::Label masterLabel;
};

}  // namespace aidaw
