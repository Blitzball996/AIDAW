#pragma once

#include <juce_core/juce_core.h>
#include <string>

namespace aidaw {

struct AppConfig {
    std::string projectDir;
    std::string modelPath;
    std::string cloudApiUrl;
    std::string cloudApiKey;
    std::string relayUrl;
    int audioBufferSize = 512;
    int sampleRate = 44100;

    static AppConfig load();
    void save() const;
    juce::File getConfigFile() const;
};

}  // namespace aidaw
