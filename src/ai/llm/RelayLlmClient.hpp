#pragma once

#include "LlmClient.hpp"
#include <string>

namespace aidaw {

class RelayLlmClient : public LlmClient {
public:
    explicit RelayLlmClient(const std::string& relayUrl);

    std::string getName() const override { return "Relay"; }
    bool isAvailable() const override;
    LlmResponse send(const LlmRequest& request) override;
    LlmResponse sendStreaming(const LlmRequest& request, TokenCallback onToken) override;

    void setRelayUrl(const std::string& url) { relayUrl = url; }

private:
    std::string relayUrl;
};

}  // namespace aidaw
