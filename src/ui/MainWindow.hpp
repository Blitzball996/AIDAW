#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "modes/ModeController.hpp"
#include "panels/TransportPanel.hpp"
#include "views/ArrangementView.hpp"
#include "views/MixerView.hpp"
#include "views/SessionView.hpp"

namespace aidaw {

class MainWindowComponent : public juce::Component, public ModeListener {
public:
    MainWindowComponent();
    ~MainWindowComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void modeChanged(AppMode newMode, const ModeProfile& profile) override;

private:
    // Mode toggle
    juce::TextButton modeToggleButton{"Switch to AI Mode"};

    // Pro mode components
    TransportPanel transportPanel;
    ArrangementView arrangementView;
    MixerView mixerView;
    SessionView sessionView;

    // View tabs for Pro mode
    juce::TextButton arrangeTab{"Arrange"};
    juce::TextButton mixerTab{"Mixer"};
    juce::TextButton sessionTab{"Session"};
    int currentProView = 0; // 0=arrange, 1=mixer, 2=session

    // AI mode
    std::unique_ptr<juce::Component> aiView;

    void updateView();
    void switchProView(int viewIndex);
    void showProMode();
    void showAiMode();
};

}  // namespace aidaw
