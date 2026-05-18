#include "MainWindow.hpp"
#include "pro/ProMainView.hpp"
#include "ai/AiMainView.hpp"

namespace aidaw {

MainWindowComponent::MainWindowComponent() {
    ModeController::getInstance().addListener(this);

    modeToggleButton.onClick = [this] {
        ModeController::getInstance().toggleMode();
    };
    addAndMakeVisible(modeToggleButton);

    updateView();
    setSize(1280, 800);
}

MainWindowComponent::~MainWindowComponent() {
    ModeController::getInstance().removeListener(this);
}

void MainWindowComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff1e1e2e));
}

void MainWindowComponent::resized() {
    auto area = getLocalBounds();
    modeToggleButton.setBounds(area.removeFromTop(32).removeFromRight(180).reduced(4));
    if (currentView) {
        currentView->setBounds(area);
    }
}

void MainWindowComponent::modeChanged(AppMode newMode, const ModeProfile& /*profile*/) {
    auto label = (newMode == AppMode::Professional)
        ? "Switch to AI Mode"
        : "Switch to Pro Mode";
    modeToggleButton.setButtonText(label);
    updateView();
}

void MainWindowComponent::updateView() {
    currentView.reset();

    auto mode = ModeController::getInstance().getCurrentMode();
    if (mode == AppMode::Professional) {
        currentView = std::make_unique<ProMainView>();
    } else {
        currentView = std::make_unique<AiMainView>();
    }

    addAndMakeVisible(currentView.get());
    resized();
}

}  // namespace aidaw
