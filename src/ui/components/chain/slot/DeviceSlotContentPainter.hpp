#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "core/DeviceInfo.hpp"
#include "slot/DeviceSlotTraits.hpp"

namespace aidaw::daw::ui {

struct DeviceSlotStepRecordingPaintState {
    bool active = false;
    int position = 0;
    int maxSteps = 1;
};

struct DeviceSlotContentPaintState {
    const DeviceSlotTraits& traits;
    aidaw::DeviceLoadState loadState = aidaw::DeviceLoadState::Loaded;
    bool collapsed = false;
    bool bypassed = false;
    bool internalDevice = false;
    bool hasCustomUI = false;
    juce::String manufacturer;
    juce::String deviceName;
    juce::Drawable* tracktionLogo = nullptr;
    DeviceSlotStepRecordingPaintState stepRecording;
};

void paintDeviceSlotContent(juce::Graphics& g, juce::Rectangle<int> contentArea,
                            const DeviceSlotContentPaintState& state, int meterStripWidth,
                            int contentHeaderHeight, int paginationHeight, int faustHeaderHeight);

}  // namespace aidaw::daw::ui
