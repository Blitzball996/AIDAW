#include "custom_ui/StrudelEditorWindow.hpp"

#include "ui/themes/DarkTheme.hpp"
#include "ui/themes/FontManager.hpp"

namespace magda::daw::ui {

class StrudelEditorWindow::Content : public juce::Component {
  public:
    Content(const juce::String& initialCode, EvalFn onEval)
        : editor_(document_, nullptr), onEval_(std::move(onEval)) {
        document_.replaceAllContent(initialCode);

        editor_.setColour(juce::CodeEditorComponent::backgroundColourId,
                          juce::Colour(0xff1e1e2e));
        editor_.setColour(juce::CodeEditorComponent::defaultTextColourId,
                          juce::Colour(0xffcdd6f4));
        editor_.setFont(FontManager::getInstance().getMonoFont(13.0f));
        editor_.setTabSize(2, true);
        addAndMakeVisible(editor_);

        evalBtn_.setButtonText("Eval (Ctrl+Enter)");
        evalBtn_.onClick = [this] { eval(); };
        addAndMakeVisible(evalBtn_);

        statusLabel_.setFont(FontManager::getInstance().getMonoFont(11.0f));
        statusLabel_.setJustificationType(juce::Justification::topLeft);
        statusLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff89b4fa));
        statusLabel_.setText("Ready — write a Strudel pattern", juce::dontSendNotification);
        addAndMakeVisible(statusLabel_);

        helpLabel_.setFont(FontManager::getInstance().getMonoFont(10.0f));
        helpLabel_.setJustificationType(juce::Justification::topLeft);
        helpLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff6c7086));
        helpLabel_.setText(
            "Examples:  note(\"c3 e3 g3 b3\")  |  "
            "sound(\"bd hh sd hh\")  |  "
            "note(\"c3 e3 g3\").fast(2)",
            juce::dontSendNotification);
        addAndMakeVisible(helpLabel_);

        setSize(700, 480);
    }

    void resized() override {
        auto area = getLocalBounds().reduced(8);
        auto bottom = area.removeFromBottom(90);

        auto btnRow = bottom.removeFromTop(30);
        evalBtn_.setBounds(btnRow.removeFromLeft(160));
        bottom.removeFromTop(4);

        helpLabel_.setBounds(bottom.removeFromBottom(20));
        statusLabel_.setBounds(bottom);

        editor_.setBounds(area);
    }

    bool keyPressed(const juce::KeyPress& key) override {
        if (key.getModifiers().isCtrlDown() && key.getKeyCode() == juce::KeyPress::returnKey) {
            eval();
            return true;
        }
        return false;
    }

  private:
    void eval() {
        const auto code = document_.getAllContent();
        if (onEval_) {
            auto err = onEval_(code);
            if (err.isEmpty()) {
                statusLabel_.setColour(juce::Label::textColourId, juce::Colour(0xffa6e3a1));
                statusLabel_.setText("Pattern active", juce::dontSendNotification);
            } else {
                statusLabel_.setColour(juce::Label::textColourId, juce::Colour(0xfff38ba8));
                statusLabel_.setText(err, juce::dontSendNotification);
            }
        }
    }

    juce::CodeDocument document_;
    juce::CodeEditorComponent editor_;
    juce::TextButton evalBtn_;
    juce::Label statusLabel_;
    juce::Label helpLabel_;
    EvalFn onEval_;
};

StrudelEditorWindow::StrudelEditorWindow(const juce::String& title,
                                         const juce::String& initialCode, EvalFn onEval)
    : juce::DocumentWindow(title, juce::Colour(0xff1e1e2e),
                           juce::DocumentWindow::allButtons) {
    content_ = std::make_unique<Content>(initialCode, std::move(onEval));
    setContentNonOwned(content_.get(), true);
    setUsingNativeTitleBar(false);
    setTitleBarHeight(28);
    setResizable(true, false);
    centreWithSize(700, 480);
    setVisible(true);
    setAlwaysOnTop(true);
}

StrudelEditorWindow::~StrudelEditorWindow() {
    setContentNonOwned(nullptr, false);
}

void StrudelEditorWindow::closeButtonPressed() {
    setVisible(false);
}

}  // namespace magda::daw::ui
