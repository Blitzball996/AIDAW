#pragma once

#include <memory>

namespace aidaw::daw::ui {

class DeviceParamLayout;
struct DeviceSlotTraits;

std::unique_ptr<DeviceParamLayout> createDeviceSlotParamLayout(const DeviceSlotTraits& traits);

}  // namespace aidaw::daw::ui
