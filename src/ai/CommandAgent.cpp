#include "CommandAgent.hpp"

namespace aidaw {

CommandAgent::CommandAgent(LlmClient* client)
    : llmClient_(client) {}

const char* CommandAgent::getSystemPrompt() {
    return R"PROMPT(You are the AIDAW Command agent. You generate DSL instructions to control the DAW.

Respond ONLY with DSL instructions. No prose. No markdown. No backticks.
One instruction per line.

INSTRUCTIONS:
  TRACK <name>              — Create a new track
  TRACK FX <plugin_name>    — Create track + add plugin
  DEL <ref>                 — Delete track (by index or name)
  MUTE [<ref>]              — Mute track (implicit = current)
  SOLO [<ref>]              — Solo track (implicit = current)
  SET [<ref>] key=val ...   — Set track properties (vol, pan, mute, solo)
  CLIP <bar> <length_bars>  — Create clip on current track
  CLIP <ref> <bar> <len>    — Create clip on specific track
  FX <fx_name>              — Add FX to current track
  FX <ref> <fx_name>        — Add FX to specific track
  SELECT CLIPS [WHERE <field> <op> <value>]
  SELECT TRACKS [WHERE <field> <op> <value>]
  ARP <root> <quality> <beat> <step> [<beats>]
  CHORD <root> <quality> <beat> <length> [<velocity>]
  NOTE <pitch> <beat> <length> [<velocity>]

TRACK REFERENCES:
  - By index (1-based): 1, 2, 3...
  - By name: "My Track"
  - Implicit (omit): uses last TRACK or current selection

EXAMPLES:
"create a bass track with a 4-bar clip" ->
TRACK Bass
CLIP 1 4

"add reverb to track 2" ->
FX 2 Reverb

"write a C minor chord at beat 0 for 2 beats" ->
CHORD C min 0 2

"mute the drums" ->
MUTE Drums)PROMPT";
}

std::string CommandAgent::extractDSL(const std::string& raw) {
    juce::String text(raw);
    text = text.trim();

    // Strip ```dsl ... ``` or ``` ... ``` fences
    if (text.contains("```")) {
        auto start = text.indexOf("```");
        auto afterFence = text.indexOf(start, "\n");
        if (afterFence < 0)
            afterFence = start + 3;
        else
            afterFence += 1;

        auto end = text.lastIndexOf("```");
        if (end > start)
            text = text.substring(afterFence, end).trim();
    }

    return text.toStdString();
}

void CommandAgent::setStateContext(const std::string& stateJson) {
    stateContext_ = stateJson;
}

CommandAgent::GenerateResult CommandAgent::generate(const std::string& message) {
    GenerateResult result;

    if (shouldStop_.load()) {
        result.error = "Cancelled";
        result.hasError = true;
        return result;
    }

    if (!llmClient_ || !llmClient_->isAvailable()) {
        result.error = "Command agent LLM client not available.";
        result.hasError = true;
        return result;
    }

    LlmRequest request;
    std::string systemPrompt = getSystemPrompt();
    if (!stateContext_.empty())
        systemPrompt += "\n\nCurrent DAW state:\n" + stateContext_;

    request.systemPrompt = systemPrompt;
    request.messages.push_back({"user", message});
    request.temperature = 0.1f;

    auto response = llmClient_->send(request);

    if (!response.success) {
        result.error = response.error;
        result.hasError = true;
        return result;
    }

    result.dslOutput = extractDSL(response.content);
    return result;
}

CommandAgent::GenerateResult CommandAgent::generateStreaming(const std::string& message,
                                                            TokenCallback onToken) {
    GenerateResult result;

    if (shouldStop_.load()) {
        result.error = "Cancelled";
        result.hasError = true;
        return result;
    }

    if (!llmClient_ || !llmClient_->isAvailable()) {
        result.error = "Command agent LLM client not available.";
        result.hasError = true;
        return result;
    }

    LlmRequest request;
    std::string systemPrompt = getSystemPrompt();
    if (!stateContext_.empty())
        systemPrompt += "\n\nCurrent DAW state:\n" + stateContext_;

    request.systemPrompt = systemPrompt;
    request.messages.push_back({"user", message});
    request.temperature = 0.1f;

    auto response = llmClient_->sendStreaming(request, onToken);

    if (!response.success) {
        result.error = response.error;
        result.hasError = true;
        return result;
    }

    result.dslOutput = extractDSL(response.content);
    return result;
}

}  // namespace aidaw
