#pragma once

#include <juce_core/juce_core.h>
#include <string>

namespace aidaw {

class JsonUtils {
public:
    static juce::var parse(const std::string& json) {
        return juce::JSON::parse(juce::String(json));
    }

    static std::string stringify(const juce::var& value) {
        return juce::JSON::toString(value).toStdString();
    }
};

}  // namespace aidaw
