#pragma once

#include "LlmClient.hpp"

namespace aidaw {

class LocalLlmClient : public LlmClient {
public:
    explicit LocalLlmClient(const std::string& modelPath);
    ~LocalLlmClient() override;

    std::string getName() const override { return "LocalLlama"; }
    bool isAvailable() const override;
    LlmResponse send(const LlmRequest& request) override;
    LlmResponse sendStreaming(const LlmRequest& request, TokenCallback onToken) override;

    void loadModel(const std::string& path);
    void unloadModel();

private:
    std::string modelPath;
    bool modelLoaded = false;
};

}  // namespace aidaw
