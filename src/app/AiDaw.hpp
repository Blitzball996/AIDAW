#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace aidaw {

class AiDawApplication : public juce::JUCEApplication {
public:
    const juce::String getApplicationName() override { return "AIDAW"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise(const juce::String& commandLine) override;
    void shutdown() override;
    void systemRequestedQuit() override;

private:
    std::unique_ptr<juce::DocumentWindow> mainWindow;
};

}  // namespace aidaw
