---
name: llm-backend
description: LLM client implementation patterns. Use when adding new LLM providers, modifying API calls, handling streaming, or debugging AI connectivity.
---

# LLM Backend Integration

## Interface

All backends implement `LlmClient`:

```cpp
class LlmClient {
    virtual std::string getName() const = 0;
    virtual bool isAvailable() const = 0;
    virtual LlmResponse send(const LlmRequest& request) = 0;
    virtual LlmResponse sendStreaming(const LlmRequest& request, TokenCallback onToken) = 0;
};
```

## Adding a New Backend

1. Create `src/ai/llm/NewClient.hpp` and `.cpp`
2. Implement `LlmClient` interface
3. Add to `LlmBackend` enum in `LlmClientFactory.hpp`
4. Add factory case in `LlmClientFactory::create()`
5. Add to `AiEngine::initialize()` and `getActiveClient()`

## Cloud API Compatibility

`CloudLlmClient` supports two response formats:

### OpenAI-compatible
```json
{"choices": [{"message": {"content": "..."}}]}
```

### Anthropic-compatible
```json
{"content": [{"text": "..."}]}
```

## Relay Server Protocol

`RelayLlmClient` expects the relay to accept:

```json
POST /
{
  "messages": [...],
  "max_tokens": 2048,
  "temperature": 0.7,
  "system": "..."
}
```

And respond with:
```json
{"content": "..."}
```

## Error Handling

- Always check `LlmResponse::success` before using `content`
- Network errors set `error` field with description
- Timeout: implement in the HTTP layer (JUCE URL timeout)
- Model not loaded: `LocalLlmClient::isAvailable()` returns false

## Streaming

- `TokenCallback` receives partial tokens as they arrive
- Accumulated output is returned in the final `LlmResponse`
- Cancel via `MusicAgent::requestCancel()` (sets atomic flag)

## Configuration

Stored in `AppConfig`:
- `modelPath` — path to .gguf file for local inference
- `cloudApiUrl` — endpoint URL (e.g., https://api.anthropic.com/v1/messages)
- `cloudApiKey` — API key (never log this)
- `relayUrl` — custom relay server URL
