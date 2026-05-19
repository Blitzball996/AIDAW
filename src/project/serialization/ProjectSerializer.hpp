#pragma once

#include <juce_core/juce_core.h>
#include "../ProjectInfo.hpp"

namespace aidaw {

class ProjectSerializer {
public:
    static bool saveProject(const ProjectInfo& info, const juce::File& file);
    static bool loadProject(const juce::File& file, ProjectInfo& outInfo);

    static juce::var projectInfoToJson(const ProjectInfo& info);
    static bool jsonToProjectInfo(const juce::var& json, ProjectInfo& outInfo);
};

}  // namespace aidaw
