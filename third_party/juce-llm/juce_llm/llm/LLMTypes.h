#pragma once

#include <utility>
#include <vector>

namespace llm {

//==============================================================================
enum class Provider {
    OpenAIChat,       // Chat Completions — DeepSeek, OpenRouter, local llama-server
    OpenAIResponses,  // Responses API — GPT-5+
    Anthropic,        // Messages API — Claude models
    Gemini            // generateContent — Gemini models
};

//==============================================================================
struct ProviderConfig {
    Provider provider;
    juce::String baseUrl;
    juce::String apiKey;
    juce::String model;

    int maxTokens = 0;  // max output tokens (0 = provider default — don't send a cap)

    // Provider-specific options
    bool noTemperature = false;    // GPT-5 doesn't support temperature
    juce::String reasoningEffort;  // "none", "low", "medium", "high", "xhigh"
    juce::String grammar;          // GBNF grammar for llama-server
    int connectionTimeoutMs = 0;   // 0 = system default; useful for local server checks

    // Application identity — used for User-Agent and provider-specific headers
    juce::String userAgent;  // e.g. "MAGDA/0.3.0"
    juce::String appUrl;     // e.g. "https://magda.dev" (for OpenRouter HTTP-Referer)
};

//==============================================================================
/** A tool the model may call. Only the providers that implement tool calling
    (OpenAI Chat, Anthropic) act on these; the others ignore them and the
    request behaves as a plain single-shot completion. */
struct ToolDef {
    juce::String name;
    juce::String description;
    juce::var parameters;  // JSON Schema object describing the arguments
};

/** One tool invocation requested by the model. */
struct ToolCall {
    juce::String id;         // provider-assigned; echoed back with the result
    juce::String name;
    juce::String arguments;  // raw JSON object as a string
};

/** One turn in a conversation. Used for the tool loop: the caller replays the
    whole exchange each round so the model can see what its tools returned. */
struct Message {
    enum class Role { User, Assistant, Tool };

    Role role = Role::User;
    juce::String content;

    /** Set on an Assistant turn that asked for tools. */
    std::vector<ToolCall> toolCalls;

    /** Set on a Tool turn: which call this is the result of. */
    juce::String toolCallId;

    static Message user(juce::String text) {
        Message m;
        m.role = Role::User;
        m.content = std::move(text);
        return m;
    }
    static Message assistant(juce::String text, std::vector<ToolCall> calls = {}) {
        Message m;
        m.role = Role::Assistant;
        m.content = std::move(text);
        m.toolCalls = std::move(calls);
        return m;
    }
    static Message toolResult(juce::String callId, juce::String text) {
        Message m;
        m.role = Role::Tool;
        m.toolCallId = std::move(callId);
        m.content = std::move(text);
        return m;
    }
};

//==============================================================================
struct Request {
    juce::String systemPrompt;
    juce::String userMessage;
    float temperature = 0.1f;

    /** Full conversation, newest last. When non-empty this supersedes
        userMessage; when empty the providers fall back to sending
        userMessage as a single user turn, which is what every existing
        single-shot caller relies on. */
    std::vector<Message> messages;

    /** Tools offered to the model this round. Empty = no tool calling. */
    std::vector<ToolDef> tools;

    /** Optional JSON schema for structured output.
        Built via Schema::object(), Schema::array(), etc.
        When set, providers will use their native structured output mechanism. */
    juce::var schema;

    /** Optional CFG grammar for constrained output (Lark format for OpenAI Responses API,
        GBNF for llama-server). When set, the provider will constrain the model output to match
        the grammar. For OpenAI Responses, this uses a custom tool with grammar format. */
    juce::String grammar;

    /** Tool name used for CFG grammar-constrained output (OpenAI Responses API).
        Defaults to "grammar_tool" if not set. */
    juce::String grammarToolName;

    /** Tool description for CFG grammar output. If empty, systemPrompt is used. */
    juce::String grammarToolDescription;

    /** Override max output tokens for this request (0 = use provider config default). */
    int maxTokens = 0;

    /** When true, sendStreamingRequest will add provider-specific streaming flags. */
    bool stream = false;
};

//==============================================================================
struct Response {
    juce::String text;
    double wallSeconds = 0.0;
    bool success = false;
    juce::String error;

    /** True when the provider stopped because the output token cap was hit
        (OpenAI finish_reason "length", Anthropic stop_reason "max_tokens").
        The text is then a valid prefix of an unfinished answer — callers that
        parse structured output should treat the tail as unreliable and say so
        rather than silently using a truncated result. */
    bool truncated = false;

    /** Tools the model wants run. Non-empty means this is not a final answer:
        run them, append the results as Tool messages, and send again. */
    std::vector<ToolCall> toolCalls;
};

//==============================================================================
using ResponseCallback = std::function<void(Response)>;

/** Called for each token/chunk during streaming. Return false to cancel. */
using StreamCallback = std::function<bool(const juce::String& token)>;

}  // namespace llm
