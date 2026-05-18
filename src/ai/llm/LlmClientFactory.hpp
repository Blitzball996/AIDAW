#pragma once

#include "LlmClient.hpp"
#include "LocalLlmClient.hpp"
#include "CloudLlmClient.hpp"
#include "RelayLlmClient.hpp"
#include <memory>
#include <string>

namespace aidaw {

enum class LlmBackend { Local, Cloud, Relay };

class LlmClientFactory {
public:
    static std::unique_ptr<LlmClient> create(LlmBackend backend,
                                              const std::string& endpoint,
                                              const std::string& key = "") {
        switch (backend) {
            case LlmBackend::Local:
                return std::make_unique<LocalLlmClient>(endpoint);
            case LlmBackend::Cloud:
                return std::make_unique<CloudLlmClient>(endpoint, key);
            case LlmBackend::Relay:
                return std::make_unique<RelayLlmClient>(endpoint);
        }
        return nullptr;
    }
};

}  // namespace aidaw
