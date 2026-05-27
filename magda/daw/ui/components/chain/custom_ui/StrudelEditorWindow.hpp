#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include <functional>

namespace magda::daw::ui {

class StrudelEditorWindow : public juce::DocumentWindow {
  public:
    using EvalFn = std::function<juce::String(const juce::String& code)>;

    StrudelEditorWindow(const juce::String& title, const juce::String& initialCode,
                        EvalFn onEval);
    ~StrudelEditorWindow() override;

    void closeButtonPressed() override;

  private:
    class Content;
    std::unique_ptr<Content> content_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StrudelEditorWindow)
};

}  // namespace magda::daw::ui
