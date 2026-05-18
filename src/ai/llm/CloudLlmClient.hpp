#pragma once

#include "LlmClient.hpp"
#include <string>

namespace aidaw {

class CloudLlmClient : public LlmClient {
public:
    CloudLlmClient(const std::string& apiUrl, const std::string& apiKey);

    std::string getName() const override { return "CloudAPI"; }
    bool isAvailable() const override;
    LlmResponse send(const LlmRequest& request) override;
    LlmResponse sendStreaming(const LlmRequest& request, TokenCallback onToken) override;

    void setApiKey(const std::string& key) { apiKey = key; }
    void setApiUrl(const std::string& url) { apiUrl = url; }

private:
    std::string apiUrl;
    std::string apiKey;

    std::string buildRequestJson(const LlmRequest& request) const;
    LlmResponse parseResponseJson(const std::string& json) const;
};

}  // namespace aidaw
