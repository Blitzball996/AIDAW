#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "modes/ModeController.hpp"

namespace aidaw {

class MainWindowComponent : public juce::Component, public ModeListener {
public:
    MainWindowComponent();
    ~MainWindowComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void modeChanged(AppMode newMode, const ModeProfile& profile) override;

private:
    std::unique_ptr<juce::Component> currentView;
    juce::TextButton modeToggleButton{"Switch to AI Mode"};

    void updateView();
};

}  // namespace aidaw
