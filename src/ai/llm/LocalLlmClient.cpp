#include "LocalLlmClient.hpp"

namespace aidaw {

LocalLlmClient::LocalLlmClient(const std::string& path)
    : modelPath(path) {}

LocalLlmClient::~LocalLlmClient() {
    unloadModel();
}

bool LocalLlmClient::isAvailable() const {
#ifdef AIDAW_HAS_LOCAL_LLM
    return modelLoaded;
#else
    return false;
#endif
}

LlmResponse LocalLlmClient::send(const LlmRequest& request) {
    LlmResponse response;
#ifdef AIDAW_HAS_LOCAL_LLM
    if (!modelLoaded) {
        response.error = "Model not loaded";
        return response;
    }
    // TODO: llama.cpp inference
    response.success = true;
    response.content = "[local inference placeholder]";
#else
    response.error = "Local LLM not compiled in";
#endif
    return response;
}

LlmResponse LocalLlmClient::sendStreaming(const LlmRequest& request, TokenCallback onToken) {
    // For now, delegate to non-streaming
    auto response = send(request);
    if (response.success && onToken) {
        onToken(response.content);
    }
    return response;
}

void LocalLlmClient::loadModel(const std::string& path) {
    modelPath = path;
#ifdef AIDAW_HAS_LOCAL_LLM
    // TODO: llama_model_load
    modelLoaded = true;
#endif
}

void LocalLlmClient::unloadModel() {
#ifdef AIDAW_HAS_LOCAL_LLM
    // TODO: llama_model_free
    modelLoaded = false;
#endif
}

}  // namespace aidaw
