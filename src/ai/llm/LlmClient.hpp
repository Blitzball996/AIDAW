#pragma once

#include <functional>
#include <string>
#include <vector>

namespace aidaw {

using TokenCallback = std::function<void(const std::string& token)>;

struct LlmMessage {
    std::string role;
    std::string content;
};

struct LlmRequest {
    std::vector<LlmMessage> messages;
    std::string systemPrompt;
    float temperature = 0.7f;
    int maxTokens = 2048;
};

struct LlmResponse {
    std::string content;
    std::string error;
    bool success = false;
    int tokensUsed = 0;
};

class LlmClient {
public:
    virtual ~LlmClient() = default;
    virtual std::string getName() const = 0;
    virtual bool isAvailable() const = 0;
    virtual LlmResponse send(const LlmRequest& request) = 0;
    virtual LlmResponse sendStreaming(const LlmRequest& request, TokenCallback onToken) = 0;
};

}  // namespace aidaw
