#include "ModelManager.hpp"
#include <juce_core/juce_core.h>

namespace aidaw {

std::vector<ModelInfo> ModelManager::getAvailableModels() const {
    std::vector<ModelInfo> models;
    auto dir = juce::File(juce::String(modelsDir));
    if (dir.isDirectory()) {
        for (const auto& file : dir.findChildFiles(juce::File::findFiles, false, "*.gguf")) {
            ModelInfo info;
            info.name = file.getFileNameWithoutExtension().toStdString();
            info.path = file.getFullPathName().toStdString();
            info.sizeBytes = file.getSize();
            info.downloaded = true;
            models.push_back(info);
        }
    }
    return models;
}

bool ModelManager::downloadModel(const std::string& /*url*/, const std::string& /*destPath*/) {
    // TODO: HTTP download with progress
    return false;
}

bool ModelManager::isModelValid(const std::string& path) const {
    auto file = juce::File(juce::String(path));
    return file.existsAsFile() && file.getSize() > 0;
}

}  // namespace aidaw
