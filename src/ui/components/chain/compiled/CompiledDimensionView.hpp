#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

#include "audio/plugins/compiled/MagdaDimensionCompiledPlugin.hpp"
#include "compiled/CompiledPluginPresentation.hpp"
#include "core/DeviceInfo.hpp"

namespace aidaw::daw::ui {

/**
 * @brief Compact engine indicator for the Dimension widener.
 *
 * No curve to plot here — Dimension's three engines are distinct enough
 * that a label + a "stereo image" indicator (two dots that pull apart
 * with Amount × Width) reads more useful than a magnitude trace. Mirrors
 * the read-only-panel pattern other compiled plugins use, just minimal.
 */
class CompiledDimensionView final : public juce::Component,
                                    public CompiledDevicePanel,
                                    private juce::Timer {
  public:
    explicit CompiledDimensionView(juce::String pluginId);

    int getPreferredHeight() const {
        return 56;
    }

    void setCompiledPlugin(aidaw::daw::audio::compiled::MagdaDimensionCompiledPlugin* plugin);
    void updateFromDevice(const aidaw::DeviceInfo& device) override;

    juce::Component& component() override {
        return *this;
    }
    void bindPlugin(te::Plugin* plugin) override;
    void setOnParameterChanged(std::function<void(int, float)>) override {}  // read-only
    int preferredHeight() const override {
        return getPreferredHeight();
    }

    void paint(juce::Graphics& g) override;

  private:
    using Plugin = aidaw::daw::audio::compiled::MagdaDimensionCompiledPlugin;

    void timerCallback() override;
    void resampleFromDevice();

    aidaw::daw::audio::compiled::MagdaDimensionCompiledPlugin* compiledPlugin_ = nullptr;
    aidaw::DeviceInfo deviceSnapshot_;

    int engine_ = 0;
    float amount_ = 0.5f;
    float width_ = 100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompiledDimensionView)
};

}  // namespace aidaw::daw::ui
