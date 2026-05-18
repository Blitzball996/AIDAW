#include "AiDaw.hpp"
#include "ui/MainWindow.hpp"

namespace aidaw {

class MainAppWindow : public juce::DocumentWindow {
public:
    MainAppWindow()
        : DocumentWindow("AIDAW",
                         juce::Desktop::getInstance().getDefaultLookAndFeel()
                             .findColour(ResizableWindow::backgroundColourId),
                         DocumentWindow::allButtons) {
        setUsingNativeTitleBar(true);
        setContentOwned(new MainWindowComponent(), true);
        setResizable(true, true);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
    }

    void closeButtonPressed() override {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

void AiDawApplication::initialise(const juce::String& /*commandLine*/) {
    mainWindow = std::make_unique<MainAppWindow>();
}

void AiDawApplication::shutdown() {
    mainWindow.reset();
}

void AiDawApplication::systemRequestedQuit() {
    quit();
}

}  // namespace aidaw
