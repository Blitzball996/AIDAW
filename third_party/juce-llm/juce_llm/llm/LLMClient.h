#pragma once

namespace llm {

//==============================================================================
/** Abstract base class for LLM provider clients.
    Each provider implements payload building, URL/header generation, and response parsing.
    Includes a synchronous sendRequest() that handles HTTP transport via juce::URL.
*/
class LLMClient {
  public:
    explicit LLMClient(const ProviderConfig& config) : config_(config) {}
    virtual ~LLMClient() = default;

    /** Get the provider name for logging/debugging. */
    virtual juce::String getName() const = 0;

    //==============================================================================
    // Data interface — override these in each provider

    /** Build the JSON payload string for this provider. */
    virtual juce::String buildRequestBody(const Request& request) const = 0;

    /** Get the full endpoint URL. */
    virtual juce::String getEndpointUrl() const = 0;

    /** Get the HTTP headers as key-value pairs. */
    virtual juce::StringPairArray getHeaders() const = 0;

    /** Parse a JSON response string into a Response. */
    virtual Response parseResponseBody(const juce::String& jsonString) const = 0;

    /** Turn an unrequested tool call into a diagnosed failure.

        parseResponseBody cannot decide this on its own: a tool call with no
        text is a perfectly good reply when the caller offered tools, and a
        symptom of an endpoint injecting its own agent harness when it did not.
        Only the request knows which case applies, so sendRequest applies this
        after parsing. Exposed as a static so it can be tested directly. */
    static void applyUnsolicitedToolCallDiagnostic(const Request& request, Response& response);

    //==============================================================================
    // Data interface — streaming (optional override)

    /** Get the endpoint URL for streaming requests. Default returns getEndpointUrl(). */
    virtual juce::String getStreamingEndpointUrl() const;

    /** Build JSON payload with streaming enabled. Default adds "stream":true. */
    virtual juce::String buildStreamingRequestBody(const Request& request) const;

    /** Parse one SSE data line into a content token. Return empty if not a content chunk. */
    virtual juce::String parseStreamChunk(const juce::String& dataLine) const;

    //==============================================================================
    // HTTP transport

    /** Synchronous HTTP request. Call from any thread. */
    virtual Response sendRequest(const Request& request) const;

    /** Streaming HTTP request. Calls onToken for each content chunk.
        Returns the final accumulated Response. Call from any thread. */
    virtual Response sendStreamingRequest(const Request& request, StreamCallback onToken) const;

    /** Access the config. */
    const ProviderConfig& getConfig() const {
        return config_;
    }

  protected:
    ProviderConfig config_;
};

}  // namespace llm
