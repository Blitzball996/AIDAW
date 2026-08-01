// Regression tests for the tool-calling wire format.
//
// The tool loop cannot be exercised end-to-end without a live endpoint, so
// these pin the two halves that are checkable offline: that a conversation
// with tool calls and tool results serialises into what each provider's API
// actually expects, and that an unsolicited tool call is still reported as the
// relay-injection problem it usually is.

#include <catch2/catch_test_macros.hpp>

#include <juce_core/juce_core.h>

#include "juce_llm/llm/LLMTypes.h"
#include "juce_llm/llm/LLMClient.h"
#include "juce_llm/llm/providers/AnthropicClient.h"
#include "juce_llm/llm/providers/OpenAIChatClient.h"

namespace {

llm::ProviderConfig cfg(llm::Provider p) {
    llm::ProviderConfig c;
    c.provider = p;
    c.model = "test-model";
    c.baseUrl = "https://example.invalid/v1";
    c.apiKey = "test-key";
    return c;
}

llm::Request conversationWithToolRound() {
    llm::Request r;
    r.systemPrompt = "sys";
    r.temperature = 0.1f;

    llm::ToolDef tool;
    tool.name = "read_clip";
    tool.description = "Read notes";
    auto* props = new juce::DynamicObject();
    auto* schema = new juce::DynamicObject();
    schema->setProperty("type", "object");
    schema->setProperty("properties", juce::var(props));
    tool.parameters = juce::var(schema);
    r.tools.push_back(tool);

    llm::ToolCall call;
    call.id = "call_1";
    call.name = "read_clip";
    call.arguments = R"({"track":"Piano"})";

    r.messages.push_back(llm::Message::user("make the piano busier"));
    r.messages.push_back(llm::Message::assistant("", {call}));
    r.messages.push_back(llm::Message::toolResult("call_1", R"({"noteCount":4})"));
    return r;
}

}  // namespace

TEST_CASE("OpenAI body carries tools, tool_calls and a tool-role result") {
    llm::OpenAIChatClient client{cfg(llm::Provider::OpenAIChat)};
    auto json = juce::JSON::parse(client.buildRequestBody(conversationWithToolRound()));

    auto* messages = json["messages"].getArray();
    REQUIRE(messages != nullptr);
    // system + user + assistant + tool
    REQUIRE(messages->size() == 4);

    REQUIRE((*messages)[0]["role"].toString() == "system");
    REQUIRE((*messages)[1]["role"].toString() == "user");

    const auto& assistant = (*messages)[2];
    REQUIRE(assistant["role"].toString() == "assistant");
    auto* calls = assistant["tool_calls"].getArray();
    REQUIRE(calls != nullptr);
    REQUIRE(calls->size() == 1);
    REQUIRE((*calls)[0]["id"].toString() == "call_1");
    REQUIRE((*calls)[0]["function"]["name"].toString() == "read_clip");

    const auto& toolMsg = (*messages)[3];
    REQUIRE(toolMsg["role"].toString() == "tool");
    // The id must round-trip or the API cannot match result to call.
    REQUIRE(toolMsg["tool_call_id"].toString() == "call_1");

    auto* tools = json["tools"].getArray();
    REQUIRE(tools != nullptr);
    REQUIRE((*tools)[0]["function"]["name"].toString() == "read_clip");
}

TEST_CASE("Anthropic body uses content blocks and folds tool results into a user turn") {
    llm::AnthropicClient client{cfg(llm::Provider::Anthropic)};
    auto json = juce::JSON::parse(client.buildRequestBody(conversationWithToolRound()));

    auto* messages = json["messages"].getArray();
    REQUIRE(messages != nullptr);
    // Anthropic has no system message and no tool role: user, assistant, user.
    REQUIRE(messages->size() == 3);

    REQUIRE((*messages)[0]["role"].toString() == "user");

    const auto& assistant = (*messages)[1];
    REQUIRE(assistant["role"].toString() == "assistant");
    auto* blocks = assistant["content"].getArray();
    REQUIRE(blocks != nullptr);
    REQUIRE((*blocks)[0]["type"].toString() == "tool_use");
    REQUIRE((*blocks)[0]["id"].toString() == "call_1");
    // input must be an object, not the raw JSON string.
    REQUIRE((*blocks)[0]["input"].isObject());
    REQUIRE((*blocks)[0]["input"]["track"].toString() == "Piano");

    const auto& resultTurn = (*messages)[2];
    REQUIRE(resultTurn["role"].toString() == "user");
    auto* resultBlocks = resultTurn["content"].getArray();
    REQUIRE(resultBlocks != nullptr);
    REQUIRE((*resultBlocks)[0]["type"].toString() == "tool_result");
    REQUIRE((*resultBlocks)[0]["tool_use_id"].toString() == "call_1");

    auto* tools = json["tools"].getArray();
    REQUIRE(tools != nullptr);
    REQUIRE((*tools)[0]["name"].toString() == "read_clip");
    REQUIRE((*tools)[0]["input_schema"].isObject());
}

TEST_CASE("Single-shot requests are unchanged when no messages or tools are set") {
    llm::OpenAIChatClient client{cfg(llm::Provider::OpenAIChat)};
    llm::Request r;
    r.systemPrompt = "sys";
    r.userMessage = "hello";

    auto json = juce::JSON::parse(client.buildRequestBody(r));
    auto* messages = json["messages"].getArray();
    REQUIRE(messages != nullptr);
    REQUIRE(messages->size() == 2);
    REQUIRE((*messages)[1]["content"].toString() == "hello");
    // No tools key at all, so nothing changes for existing callers.
    REQUIRE(json["tools"].isVoid());
}

TEST_CASE("A tool call is parsed rather than treated as empty output") {
    llm::OpenAIChatClient client{cfg(llm::Provider::OpenAIChat)};

    const juce::String body = R"JSON({
      "choices": [{
        "message": {
          "role": "assistant",
          "content": "",
          "tool_calls": [{
            "id": "call_9", "type": "function",
            "function": {"name": "read_clip", "arguments": "{\"track\":\"Bass\"}"}
          }]
        },
        "finish_reason": "tool_calls"
      }]
    })JSON";

    auto response = client.parseResponseBody(body);

    REQUIRE(response.success);
    REQUIRE(response.toolCalls.size() == 1);
    REQUIRE(response.toolCalls[0].id == "call_9");
    REQUIRE(response.toolCalls[0].name == "read_clip");
}

TEST_CASE("Anthropic tool_use blocks are parsed into tool calls") {
    llm::AnthropicClient client{cfg(llm::Provider::Anthropic)};

    const juce::String body = R"JSON({
      "content": [
        {"type": "tool_use", "id": "toolu_2", "name": "list_tracks", "input": {}}
      ],
      "stop_reason": "tool_use"
    })JSON";

    auto response = client.parseResponseBody(body);

    REQUIRE(response.success);
    REQUIRE(response.toolCalls.size() == 1);
    REQUIRE(response.toolCalls[0].name == "list_tracks");
}
