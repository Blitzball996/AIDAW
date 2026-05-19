#pragma once

#include <juce_core/juce_core.h>

#include <functional>
#include <unordered_map>

#include "core/TypeIds.hpp"

namespace aidaw {
struct DeviceInfo;
}

namespace aidaw::daw::ui {

class ParamHostComponent;

struct ParameterLearnHighlightState {
    int lockedParamIndex = -1;
    juce::uint32 lockTimeMs = 0;
    std::unordered_map<int, float> lastValueByParam;

    void reset();
};

void updateCachedParameterValue(aidaw::DeviceInfo& device, int paramIndex, float newValue);

bool refreshEngineAwareCompiledSlots(aidaw::DeviceInfo& device, aidaw::DeviceId deviceId,
                                     int changedParamIndex, ParamHostComponent& paramGrid);

void applyLearnModeParameterHighlight(aidaw::DeviceInfo& device, ParamHostComponent& paramGrid,
                                      int paramIndex, float newValue,
                                      ParameterLearnHighlightState& state,
                                      const std::function<void()>& onPageChanged);

void updateCurrentPageParameterSlotValue(const aidaw::DeviceInfo& device,
                                         ParamHostComponent& paramGrid, int paramIndex,
                                         float newValue);

}  // namespace aidaw::daw::ui
