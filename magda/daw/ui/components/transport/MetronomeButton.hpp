#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace magda {

/**
 * @brief Toggle button for metronome on/off in the transport bar.
 *
 * Left-click toggles metronome enabled state.
 * Right-click opens settings popup (volume, count-in bars, subdivision).
 */
class MetronomeButton : public juce::Component {
  public:
    MetronomeButton();
    ~MetronomeButton() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

    // State
    void setEnabled(bool enabled);
    bool isMetronomeEnabled() const noexcept { return enabled_; }

    // Settings
    void setVolume(float volume);
    float getVolume() const noexcept { return volume_; }

    void setCountIn(int bars);
    int getCountIn() const noexcept { return countInBars_; }

    void setSubdivision(int subdivision);
    int getSubdivision() const noexcept { return subdivision_; }

    // Callbacks
    std::function<void(bool enabled)> onToggle;
    std::function<void(float volume)> onVolumeChanged;
    std::function<void(int bars)> onCountInChanged;
    std::function<void(int subdivision)> onSubdivisionChanged;

  private:
    void showSettingsMenu();

    bool enabled_ = false;
    float volume_ = 0.8f;
    int countInBars_ = 0;
    int subdivision_ = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MetronomeButton)
};

}  // namespace magda
