#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "compiled/CompiledPluginPresentation.hpp"
#include "core/DeviceInfo.hpp"

namespace aidaw::daw::audio::compiled {
class MagdaGritCompiledPlugin;
}

namespace aidaw::daw::ui {

/**
 * @brief Spectrum-shape visual for the compiled grit texturizer.
 *
 * Plots the carrier's energy on a log-frequency axis. In Noise / Wide
 * Noise modes that's the bandpass response curve (peak at Frequency,
 * narrow/wide by Width). In Sine mode it's a single vertical spike at
 * Frequency. Amount scales the curve's height. Polls the host params
 * at ~30 Hz and only repaints when one moves materially.
 */
class CompiledGritCurveView final : public juce::Component,
                                    public CompiledDevicePanel,
                                    private juce::Timer {
  public:
    explicit CompiledGritCurveView(juce::String pluginId);

    int getPreferredHeight() const {
        return 140;
    }

    void setCompiledPlugin(aidaw::daw::audio::compiled::MagdaGritCompiledPlugin* plugin);
    void updateFromDevice(const aidaw::DeviceInfo& device) override;

    juce::Component& component() override {
        return *this;
    }
    void bindPlugin(te::Plugin* plugin) override;
    void setOnParameterChanged(std::function<void(int, float)>) override {}
    int preferredHeight() const override {
        return getPreferredHeight();
    }
    void paint(juce::Graphics& g) override;

  private:
    void timerCallback() override;
    void resampleFromPlugin();

    aidaw::daw::audio::compiled::MagdaGritCompiledPlugin* compiledPlugin_ = nullptr;
    aidaw::DeviceInfo deviceSnapshot_;

    float frequencyHz_ = 1000.0f;
    float width_ = 0.5f;
    float amount_ = 0.0f;
    int modeIndex_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompiledGritCurveView)
};

}  // namespace aidaw::daw::ui
