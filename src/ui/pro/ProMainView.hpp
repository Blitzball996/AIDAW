#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace aidaw {

class TrackLaneComponent : public juce::Component {
public:
    TrackLaneComponent(int trackIndex, const juce::String& name);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    int index;
    juce::String trackName;
    juce::Slider volumeSlider;
    juce::TextButton muteButton{"M"};
    juce::TextButton soloButton{"S"};
};

class ProMainView : public juce::Component, public juce::Timer {
public:
    ProMainView();
    ~ProMainView() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    // Transport bar
    juce::TextButton playButton{">"};
    juce::TextButton stopButton{"[]"};
    juce::TextButton recordButton{"O"};
    juce::TextButton loopButton{"L"};
    juce::Label tempoLabel;
    juce::Label positionLabel;
    juce::Label modeLabel;

    // Track area
    juce::Viewport trackViewport;
    juce::Component trackContainer;
    juce::OwnedArray<TrackLaneComponent> trackLanes;
    juce::TextButton addTrackButton{"+ Add Track"};

    // Bottom mixer
    juce::Slider masterVolume;
    juce::Label masterLabel;

    void onPlay();
    void onStop();
    void onRecord();
    void onAddTrack();
    void rebuildTrackLanes();
};

}  // namespace aidaw
