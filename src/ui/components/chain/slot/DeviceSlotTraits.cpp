#include "slot/DeviceSlotTraits.hpp"

#include "../../../../../agents/coder_agent.hpp"
#include "../../../../../agents/internal_plugins.hpp"
#include "../../../../../agents/sound_design_agent.hpp"
#include "core/InternalDeviceKind.hpp"

namespace aidaw::daw::ui {

DeviceSlotTraits makeDeviceSlotTraits(const juce::String& pluginId) {
    const auto kind = aidaw::classifyInternalDevice(pluginId);

    DeviceSlotTraits traits;
    traits.isDrumGrid = kind == aidaw::InternalDeviceKind::DrumGrid;
    traits.isChordEngine = kind == aidaw::InternalDeviceKind::MidiChordEngine;
    traits.isArpeggiator = kind == aidaw::InternalDeviceKind::Arpeggiator;
    traits.isStepSequencer = kind == aidaw::InternalDeviceKind::StepSequencer;
    traits.isFaust = kind == aidaw::InternalDeviceKind::Faust;
    traits.isAISupported = aidaw::isDeviceAISupported(pluginId);
    traits.isSoundDesignSupported = aidaw::isSoundDesignSupported(pluginId);
    traits.isTracktionDevice = aidaw::isTracktionEngineStockPlugin(pluginId);
    traits.compiledPresentation = findCompiledPresentation(pluginId);
    return traits;
}

}  // namespace aidaw::daw::ui
