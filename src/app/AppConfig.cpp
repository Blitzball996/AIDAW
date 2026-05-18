#include "AppConfig.hpp"

namespace aidaw {

AppConfig AppConfig::load() {
    AppConfig config;
    auto file = config.getConfigFile();
    if (file.existsAsFile()) {
        auto json = juce::JSON::parse(file);
        if (auto* obj = json.getDynamicObject()) {
            config.modelPath = obj->getProperty("modelPath").toString().toStdString();
            config.cloudApiUrl = obj->getProperty("cloudApiUrl").toString().toStdString();
            config.cloudApiKey = obj->getProperty("cloudApiKey").toString().toStdString();
            config.relayUrl = obj->getProperty("relayUrl").toString().toStdString();
            config.audioBufferSize = obj->getProperty("audioBufferSize");
            config.sampleRate = obj->getProperty("sampleRate");
        }
    }
    return config;
}

void AppConfig::save() const {
    auto obj = std::make_unique<juce::DynamicObject>();
    obj->setProperty("modelPath", juce::String(modelPath));
    obj->setProperty("cloudApiUrl", juce::String(cloudApiUrl));
    obj->setProperty("cloudApiKey", juce::String(cloudApiKey));
    obj->setProperty("relayUrl", juce::String(relayUrl));
    obj->setProperty("audioBufferSize", audioBufferSize);
    obj->setProperty("sampleRate", sampleRate);

    auto file = getConfigFile();
    file.getParentDirectory().createDirectory();
    file.replaceWithText(juce::JSON::toString(juce::var(obj.release())));
}

juce::File AppConfig::getConfigFile() const {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("AIDAW")
        .getChildFile("config.json");
}

}  // namespace aidaw
