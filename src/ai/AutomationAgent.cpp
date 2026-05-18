#include "AutomationAgent.hpp"

namespace aidaw {

AutomationAgent::AutomationAgent(LlmClient* client)
    : llmClient_(client) {}

const char* AutomationAgent::getSystemPrompt() {
    return R"PROMPT(You are the AIDAW Automation agent. You write automation curves on a lane.

Respond ONLY with AUTO instructions. No prose. No markdown. No backticks.
One instruction per line.

TIME is in BEATS. VALUES are NORMALIZED in [0.0, 1.0] where 0.0 = parameter
minimum and 1.0 = parameter maximum. The user may speak in real units
("filter from 200Hz to 2kHz") but you emit normalized values; the DAW maps
them back to the real parameter range.

A BAR = 4 BEATS. "over 2 bars" = span 8 beats. "8 bars" = span 32 beats.

INSTRUCTIONS:
  AUTO sin    start=<beat> end=<beat> min=<0..1> max=<0..1> cycles=<N>
  AUTO tri    start=<beat> end=<beat> min=<0..1> max=<0..1> cycles=<N>
  AUTO saw    start=<beat> end=<beat> min=<0..1> max=<0..1> cycles=<N>
  AUTO square start=<beat> end=<beat> min=<0..1> max=<0..1> cycles=<N> [duty=<0..1>]
  AUTO exp    start=<beat> end=<beat> min=<0..1> max=<0..1>
  AUTO log    start=<beat> end=<beat> min=<0..1> max=<0..1>
  AUTO line   start=<beat> end=<beat> from=<0..1> to=<0..1>
  AUTO freeform points=(<beat>,<0..1>)(<beat>,<0..1>)...
  AUTO clear

TARGETS (append ` target=<...>` to any instruction):
  target=volume     — currently selected track's volume fader
  target=pan        — currently selected track's pan knob
  target=selected   — currently selected automation lane (DEFAULT)
  target=laneId:<N> — a specific lane by id

EXAMPLES:
"8 bar volume fade in" ->
AUTO line start=0 end=32 from=0 to=1 target=volume

"volume fade out over 4 bars" ->
AUTO line start=0 end=16 from=1 to=0 target=volume

"tremolo, 8 cycles over 2 bars" ->
AUTO sin start=0 end=8 min=0.3 max=1 cycles=8 target=volume

"auto pan, slow 2-cycle sweep over 4 bars" ->
AUTO sin start=0 end=16 min=0 max=1 cycles=2 target=pan

"slow filter sweep up over 4 bars" ->
AUTO line start=0 end=16 from=0 to=1 target=selected

"sine LFO, 4 cycles over 2 bars" ->
AUTO sin start=0 end=8 min=0 max=1 cycles=4 target=selected

"clear the automation and draw a rising saw, 2 cycles over 4 bars" ->
AUTO clear target=selected
AUTO saw start=0 end=16 min=0 max=1 cycles=2 target=selected)PROMPT";
}

std::string AutomationAgent::cleanOutput(const std::string& raw) {
    juce::String text(raw);
    text = text.trim();
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

AutomationAgent::GenerateResult AutomationAgent::generate(const std::string& message) {
    GenerateResult result;

    if (shouldStop_.load()) {
        result.error = "Cancelled";
        result.hasError = true;
        return result;
    }

    if (!llmClient_ || !llmClient_->isAvailable()) {
        result.error = "Automation agent LLM client not available.";
        result.hasError = true;
        return result;
    }

    LlmRequest request;
    request.systemPrompt = getSystemPrompt();
    request.messages.push_back({"user", message});
    request.temperature = 0.1f;

    auto response = llmClient_->send(request);

    if (!response.success) {
        result.error = response.error;
        result.hasError = true;
        return result;
    }

    result.rawOutput = cleanOutput(response.content);

    result.instructions = parser_.parse(juce::String(result.rawOutput));
    if (result.instructions.empty() && parser_.getLastError().isNotEmpty()) {
        result.error = "Parse error: " + parser_.getLastError().toStdString();
        result.hasError = true;
    }

    return result;
}

AutomationAgent::GenerateResult AutomationAgent::generateStreaming(const std::string& message,
                                                                   TokenCallback onToken) {
    GenerateResult result;

    if (shouldStop_.load()) {
        result.error = "Cancelled";
        result.hasError = true;
        return result;
    }

    if (!llmClient_ || !llmClient_->isAvailable()) {
        result.error = "Automation agent LLM client not available.";
        result.hasError = true;
        return result;
    }

    LlmRequest request;
    request.systemPrompt = getSystemPrompt();
    request.messages.push_back({"user", message});
    request.temperature = 0.1f;

    auto response = llmClient_->sendStreaming(request, onToken);

    if (!response.success) {
        result.error = response.error;
        result.hasError = true;
        return result;
    }

    result.rawOutput = cleanOutput(response.content);

    result.instructions = parser_.parse(juce::String(result.rawOutput));
    if (result.instructions.empty() && parser_.getLastError().isNotEmpty()) {
        result.error = "Parse error: " + parser_.getLastError().toStdString();
        result.hasError = true;
    }

    return result;
}

std::string AutomationAgent::execute(const GenerateResult& result) {
    if (result.hasError)
        return result.error;

    if (!executor_.execute(result.instructions)) {
        return "Automation execution error: " + executor_.getError().toStdString() +
               "\nAUTO was: " + result.rawOutput;
    }

    auto results = executor_.getResults();
    if (results.isEmpty())
        return "Done.";

    return results.toStdString();
}

}  // namespace aidaw
