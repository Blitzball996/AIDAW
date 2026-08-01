// Regression tests for LLM response parsing.
//
// Motivating failure: a relay channel that prepends a Claude Code agent system
// prompt pushes the model into tool-calling mode. It then returns a perfectly
// well-formed response carrying tool_calls and an empty content string. The
// parser used to report that as "Failed to parse response: <first 200 chars>",
// which truncates before the tool_calls field — so the user saw a parse error
// with no indication of the real cause and reasonably assumed a timeout.
//
// The JSON in the first test is a verbatim capture from that relay.

#include <catch2/catch_test_macros.hpp>

#include <juce_core/juce_core.h>

#include "juce_llm/llm/LLMTypes.h"
#include "juce_llm/llm/LLMClient.h"
#include "juce_llm/llm/providers/OpenAIChatClient.h"
#include "juce_llm/llm/providers/AnthropicClient.h"

namespace {

llm::ProviderConfig dummyConfig() {
    llm::ProviderConfig cfg;
    cfg.model = "test-model";
    cfg.baseUrl = "https://example.invalid/v1";
    cfg.apiKey = "test-key";
    return cfg;
}

}  // namespace

TEST_CASE("OpenAI-format tool call with empty content is diagnosed, not mis-parsed") {
    llm::OpenAIChatClient client{dummyConfig()};

    const juce::String body = R"JSON({
      "id": "msg_011CdXMgPbWR62MfPz28YFso",
      "model": "claude-fable-5",
      "object": "chat.completion",
      "created": 1785374134,
      "choices": [{
        "index": 0,
        "message": {
          "role": "assistant",
          "content": "",
          "tool_calls": [{
            "id": "toolu_01D2hRNp6ukUEbCZMNrQdxA8",
            "type": "function",
            "function": {"name": "Glob", "arguments": "{\"pattern\":\"**/*\"}"}
          }]
        },
        "finish_reason": "tool_calls"
      }]
    })JSON";

    auto response = client.parseResponseBody(body);

    // Parsing itself succeeds — a tool call is a well-formed reply. Whether it
    // is usable depends on whether the caller offered tools, which is decided
    // one layer up.
    REQUIRE(response.toolCalls.size() == 1);
    REQUIRE(response.toolCalls[0].name == "Glob");

    llm::Request requestWithoutTools;  // we offered nothing
    llm::LLMClient::applyUnsolicitedToolCallDiagnostic(requestWithoutTools, response);

    REQUIRE_FALSE(response.success);
    // Must name the offending tool so the cause is identifiable from the UI.
    REQUIRE(response.error.contains("Glob"));
    // Must not fall back to the uninformative generic message.
    REQUIRE_FALSE(response.error.startsWith("Failed to parse response"));
}

TEST_CASE("A tool call we did ask for is left alone") {
    llm::Response response;
    response.success = true;
    response.toolCalls.push_back({"call_1", "read_clip", "{}"});

    llm::Request request;
    request.tools.push_back({"read_clip", "Read notes", juce::var()});

    llm::LLMClient::applyUnsolicitedToolCallDiagnostic(request, response);

    REQUIRE(response.success);
    REQUIRE(response.error.isEmpty());
}

TEST_CASE("OpenAI-format normal content still parses") {
    llm::OpenAIChatClient client{dummyConfig()};

    const juce::String body =
        R"JSON({"choices":[{"message":{"role":"assistant","content":"  notes.add(pitch=C4)  "}}]})JSON";

    auto response = client.parseResponseBody(body);

    REQUIRE(response.success);
    REQUIRE(response.text == "notes.add(pitch=C4)");
}

TEST_CASE("Anthropic-format tool_use with no text is diagnosed") {
    llm::AnthropicClient client{dummyConfig()};

    const juce::String body = R"JSON({
      "content": [
        {"type": "tool_use", "id": "toolu_1", "name": "Glob", "input": {"pattern": "**/*"}}
      ],
      "stop_reason": "tool_use"
    })JSON";

    auto response = client.parseResponseBody(body);

    REQUIRE(response.toolCalls.size() == 1);
    REQUIRE(response.toolCalls[0].name == "Glob");

    llm::Request requestWithoutTools;
    llm::LLMClient::applyUnsolicitedToolCallDiagnostic(requestWithoutTools, response);

    REQUIRE_FALSE(response.success);
    REQUIRE(response.error.contains("Glob"));
    REQUIRE_FALSE(response.error.startsWith("Failed to parse response"));
}

TEST_CASE("Anthropic-format text after a leading non-text block is still found") {
    llm::AnthropicClient client{dummyConfig()};

    // The answer lives in a later block; reading only content[0] would miss it
    // and report an empty response.
    const juce::String body = R"JSON({
      "content": [
        {"type": "thinking", "thinking": "considering the progression"},
        {"type": "text", "text": "notes.add_chord(root=C4, quality=min, beat=0, length=4)"}
      ],
      "stop_reason": "end_turn"
    })JSON";

    auto response = client.parseResponseBody(body);

    REQUIRE(response.success);
    REQUIRE(response.text == "notes.add_chord(root=C4, quality=min, beat=0, length=4)");
}
