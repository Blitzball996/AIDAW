#pragma once

#include <tracktion_engine/tracktion_engine.h>

#include <memory>

#include "core/TypeIds.hpp"

namespace aidaw {

class DeviceProcessor;

std::unique_ptr<DeviceProcessor> createDeviceProcessorForPlugin(
    DeviceId deviceId, tracktion::engine::Plugin::Ptr plugin, const juce::String& pluginId);

}  // namespace aidaw
