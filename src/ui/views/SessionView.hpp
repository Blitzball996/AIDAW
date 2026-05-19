#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <vector>

namespace aidaw {

class SessionView : public juce::Component {
public:
    SessionView();
    void paint(juce::Graphics& g) override;
    void resized() override;

    void setTrackCount(int count) { trackCount = count; resized(); }
    void setSceneCount(int count) { sceneCount = count; resized(); }

    std::function<void(int track, int scene)> onClipLaunch;
    std::function<void(int scene)> onSceneLaunch;

private:
    int trackCount = 4;
    int sceneCount = 8;
    int cellWidth = 100;
    int cellHeight = 40;

    juce::Viewport viewport;
    juce::Component grid;

    struct ClipSlot : public juce::Component {
        int trackIdx, sceneIdx;
        bool hasClip = false;
        bool isPlaying = false;
        juce::Colour clipColour{0xff4a6a8a};

        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        std::function<void()> onClick;
    };

    juce::OwnedArray<ClipSlot> slots;
    void rebuildGrid();
};

}  // namespace aidaw
