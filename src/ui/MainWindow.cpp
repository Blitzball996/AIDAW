#include "MainWindow.hpp"
#include "ai/AiMainView.hpp"

namespace aidaw {

MainWindowComponent::MainWindowComponent() {
    ModeController::getInstance().addListener(this);

    modeToggleButton.onClick = [this] {
        ModeController::getInstance().toggleMode();
    };
    modeToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a4a6a));
    addAndMakeVisible(modeToggleButton);

    // Transport panel
    addAndMakeVisible(transportPanel);

    // View tabs
    arrangeTab.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3d5c8a));
    mixerTab.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a4a6a));
    sessionTab.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a4a6a));

    arrangeTab.onClick = [this] { switchProView(0); };
    mixerTab.onClick = [this] { switchProView(1); };
    sessionTab.onClick = [this] { switchProView(2); };

    addAndMakeVisible(arrangeTab);
    addAndMakeVisible(mixerTab);
    addAndMakeVisible(sessionTab);

    // Views
    addChildComponent(arrangementView);
    addChildComponent(mixerView);
    addChildComponent(sessionView);

    // Start in Pro/Arrange mode
    showProMode();
    setSize(1280, 800);
}

MainWindowComponent::~MainWindowComponent() {
    ModeController::getInstance().removeListener(this);
}

void MainWindowComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff0e0e1a));
}

void MainWindowComponent::resized() {
    auto area = getLocalBounds();

    // Top bar: mode toggle + view tabs
    auto topBar = area.removeFromTop(36);
    modeToggleButton.setBounds(topBar.removeFromRight(150).reduced(3));

    if (ModeController::getInstance().getCurrentMode() == AppMode::Professional) {
        arrangeTab.setBounds(topBar.removeFromLeft(90).reduced(3));
        mixerTab.setBounds(topBar.removeFromLeft(90).reduced(3));
        sessionTab.setBounds(topBar.removeFromLeft(90).reduced(3));

        // Transport panel below top bar
        transportPanel.setBounds(area.removeFromTop(44));

        // Main view area
        arrangementView.setBounds(area);
        mixerView.setBounds(area);
        sessionView.setBounds(area);
    } else {
        arrangeTab.setVisible(false);
        mixerTab.setVisible(false);
        sessionTab.setVisible(false);
        transportPanel.setVisible(false);

        if (aiView)
            aiView->setBounds(area);
    }
}

void MainWindowComponent::modeChanged(AppMode newMode, const ModeProfile& /*profile*/) {
    modeToggleButton.setButtonText(
        newMode == AppMode::Professional ? "Switch to AI Mode" : "Switch to Pro Mode");
    updateView();
}

void MainWindowComponent::updateView() {
    auto mode = ModeController::getInstance().getCurrentMode();
    if (mode == AppMode::Professional) {
        showProMode();
    } else {
        showAiMode();
    }
    resized();
}

void MainWindowComponent::switchProView(int viewIndex) {
    currentProView = viewIndex;

    arrangementView.setVisible(viewIndex == 0);
    mixerView.setVisible(viewIndex == 1);
    sessionView.setVisible(viewIndex == 2);

    auto activeColour = juce::Colour(0xff3d5c8a);
    auto inactiveColour = juce::Colour(0xff4a4a6a);

    arrangeTab.setColour(juce::TextButton::buttonColourId, viewIndex == 0 ? activeColour : inactiveColour);
    mixerTab.setColour(juce::TextButton::buttonColourId, viewIndex == 1 ? activeColour : inactiveColour);
    sessionTab.setColour(juce::TextButton::buttonColourId, viewIndex == 2 ? activeColour : inactiveColour);
}

void MainWindowComponent::showProMode() {
    // Show pro components
    transportPanel.setVisible(true);
    arrangeTab.setVisible(true);
    mixerTab.setVisible(true);
    sessionTab.setVisible(true);

    switchProView(currentProView);

    // Hide AI view
    if (aiView) {
        removeChildComponent(aiView.get());
        aiView.reset();
    }
}

void MainWindowComponent::showAiMode() {
    // Hide pro components
    transportPanel.setVisible(false);
    arrangeTab.setVisible(false);
    mixerTab.setVisible(false);
    sessionTab.setVisible(false);
    arrangementView.setVisible(false);
    mixerView.setVisible(false);
    sessionView.setVisible(false);

    // Show AI view
    if (!aiView) {
        aiView = std::make_unique<AiMainView>();
        addAndMakeVisible(aiView.get());
    }
}

}  // namespace aidaw
