#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <functional>

#include "compiled/CompiledPluginPresentation.hpp"
#include "core/DeviceInfo.hpp"
#include "params/ParamLinkResolver.hpp"
#include "params/ParamSlotComponent.hpp"
#include "ui/components/common/DraggableValueLabel.hpp"

namespace aidaw::daw::audio::compiled {
class MagdaUtilityCompiledPlugin;
}

namespace aidaw::daw::ui {

class CompiledUtilityView final : public juce::Component, public CompiledDevicePanel {
  public:
    explicit CompiledUtilityView(juce::String pluginId);
    ~CompiledUtilityView() override;

    void updateFromDevice(const aidaw::DeviceInfo& device) override;
    void updateFromDevice(const aidaw::DeviceInfo& device,
                          const ParamLinkContext* linkContext) override;

    juce::Component& component() override {
        return *this;
    }
    void bindPlugin(te::Plugin* plugin) override;
    void setOnParameterChanged(std::function<void(int, float)> cb) override {
        onParameterChanged = std::move(cb);
    }
    void setOnLinkRequested(std::function<void(int, float)> cb) override {
        onLinkRequested = std::move(cb);
    }
    void setOnLinkAmountChanged(std::function<void(int, float)> cb) override {
        onLinkAmountChanged = std::move(cb);
    }
    int preferredHeight() const override {
        return 320;
    }
    bool wantsFullBody() const override {
        return true;
    }

    std::function<void(int slotIndex, float displayValue)> onParameterChanged;

    void resized() override;

  private:
    void syncFromDevice();
    void writeParameter(int slotIndex, float displayValue);
    void configureLinkSlots();
    void refreshLinkSlotContext();
    void updateLinkSlotValues();

    aidaw::DeviceInfo deviceSnapshot_;
    aidaw::daw::audio::compiled::MagdaUtilityCompiledPlugin* compiledPlugin_ = nullptr;
    ParamLinkContext linkContext_;
    bool hasLinkContext_ = false;

    aidaw::DraggableValueLabel gainFader_{aidaw::DraggableValueLabel::Format::Decibels};
    juce::Label gainValue_;
    aidaw::DraggableValueLabel panLabel_{aidaw::DraggableValueLabel::Format::Pan};
    aidaw::DraggableValueLabel widthLabel_{aidaw::DraggableValueLabel::Format::Raw};
    aidaw::DraggableValueLabel xoverLabel_{aidaw::DraggableValueLabel::Format::Integer};
    ParamSlotComponent gainLinkSlot_{0};
    ParamSlotComponent panLinkSlot_{1};
    bool gainLinkInfoSet_ = false;
    bool panLinkInfoSet_ = false;

    juce::Label gainName_;
    juce::Label panName_;
    juce::Label widthName_;
    juce::Label xoverName_;

    static constexpr std::array<const char*, 4> kBtnLabels{"MONO", "LOW MONO", "FLIP L", "FLIP R"};
    std::array<juce::TextButton, 4> btns_;
    std::function<void(int slotIndex, float amount)> onLinkRequested;
    std::function<void(int slotIndex, float amount)> onLinkAmountChanged;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompiledUtilityView)
};

}  // namespace aidaw::daw::ui
