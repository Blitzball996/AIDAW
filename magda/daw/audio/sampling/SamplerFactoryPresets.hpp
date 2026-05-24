#pragma once

#include <map>
#include <string>
#include <vector>

namespace magda::daw::audio {

struct SamplerPreset {
    std::string name;
    std::string category;
    std::string description;
    std::map<std::string, float> params;
};

const std::vector<SamplerPreset>& getSamplerFactoryPresets();

}  // namespace magda::daw::audio
