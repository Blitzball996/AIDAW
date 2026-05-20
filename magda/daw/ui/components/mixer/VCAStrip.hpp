#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <memory>

#include "../common/TextSlider.hpp"

namespace magda {

/**
 * @brief VCA fader strip component for the mixer view.
 *
 * Displays:
 * - VCA name (editable on double-click)
 * - Vertical fader for VCA level
 * - Mute/Solo buttons
 * - Assigned tracks indicator (count badge)
 */
class VCAStrip : public juce::Component {
  public:
    VCAStrip();
    ~VCAStrip() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // State
    void setVCAName(const juce::String& name);
    juce::String getVCAName() const;

    void setLevel(float level);
    float getLevel() const;

    void setMute(bool mute);
    bool isMuted() const noexcept { return muted_; }

    void setSolo(bool solo);
    bool isSoloed() const noexcept { return soloed_; }

    void setAssignedTrackCount(int count);

    // Callbacks
    std::function<void(float level)> onLevelChanged;
    std::function<void(bool mute)> onMuteToggled;
    std::function<void(bool solo)> onSoloToggled;
    std::function<void(const juce::String& name)> onNameChanged;

  private:
    juce::Label nameLabel_;
    std::unique_ptr<daw::ui::TextSlider> faderSlider_;
    juce::TextButton muteButton_;
    juce::TextButton soloButton_;
    juce::Label trackCountLabel_;

    bool muted_ = false;
    bool soloed_ = false;
    int assignedTrackCount_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VCAStrip)
};

}  // namespace magda
