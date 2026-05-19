#pragma once

#include "core/TypeIds.hpp"

namespace aidaw {
struct ChainNodePath;
}

namespace aidaw::daw::ui {

class DeviceCustomUIManager;

void bindDeviceSlotMidiCustomUIs(DeviceCustomUIManager& customUI, aidaw::DeviceId deviceId,
                                 const aidaw::ChainNodePath& nodePath);

}  // namespace aidaw::daw::ui
