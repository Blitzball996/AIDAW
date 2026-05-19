#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace aidaw {

class ZoomControls : public juce::Component {
public:
    ZoomControls() {
        zoomInBtn.onClick = [this] { if (onZoomIn) onZoomIn(); };
        zoomOutBtn.onClick = [this] { if (onZoomOut) onZoomOut(); };
        fitBtn.onClick = [this] { if (onFitToWindow) onFitToWindow(); };
        addAndMakeVisible(zoomInBtn);
        addAndMakeVisible(zoomOutBtn);
        addAndMakeVisible(fitBtn);
    }

    void resized() override {
        auto area = getLocalBounds();
        int btnW = area.getWidth() / 3;
        zoomOutBtn.setBounds(area.removeFromLeft(btnW).reduced(1));
        fitBtn.setBounds(area.removeFromLeft(btnW).reduced(1));
        zoomInBtn.setBounds(area.reduced(1));
    }

    std::function<void()> onZoomIn, onZoomOut, onFitToWindow;

private:
    juce::TextButton zoomInBtn{"+"}, zoomOutBtn{"-"}, fitBtn{"[]"};
};

}  // namespace aidaw
