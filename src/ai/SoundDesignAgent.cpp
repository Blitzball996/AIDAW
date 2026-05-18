#include "SoundDesignAgent.hpp"

namespace aidaw {

std::unique_ptr<SoundDesignAgent> createSoundDesignAgentFor(const juce::String& /*pluginId*/) {
    // TODO: Add device-specific SoundDesignAgent implementations as
    // internal synth plugins are ported (e.g. FourOsc).
    return nullptr;
}

bool isSoundDesignSupported(const juce::String& pluginId) {
    return createSoundDesignAgentFor(pluginId) != nullptr;
}

}  // namespace aidaw
