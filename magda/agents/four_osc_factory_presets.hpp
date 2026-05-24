#pragma once

#include "four_osc_agent.hpp"

#include <vector>

namespace magda {

/**
 * @brief Built-in factory presets for the 4OSC synth.
 *
 * These are installed to the Presets/Devices/4OSC Synth/ directory on first
 * launch (or when the directory is empty). Users can overwrite or delete them.
 */
const std::vector<FourOscAgent::Preset>& getFactoryFourOscPresets();

}  // namespace magda
