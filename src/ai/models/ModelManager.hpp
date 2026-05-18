#pragma once

#include <string>
#include <vector>

namespace aidaw {

struct ModelInfo {
    std::string name;
    std::string path;
    int64_t sizeBytes = 0;
    bool downloaded = false;
};

class ModelManager {
public:
    std::vector<ModelInfo> getAvailableModels() const;
    bool downloadModel(const std::string& url, const std::string& destPath);
    bool isModelValid(const std::string& path) const;

private:
    std::string modelsDir;
};

}  // namespace aidaw
