#include "strudel_agent.hpp"

StrudelAgent::StrudelAgent() = default;
StrudelAgent::~StrudelAgent() = default;

std::map<std::string, std::string> StrudelAgent::getCapabilities() const {
    return {
        {"pattern_generation", "Generate Strudel/Tidal mini notation patterns"},
        {"drum_patterns", "Create drum beats and rhythms"},
        {"melodic_patterns", "Create melodies and chord progressions"},
        {"pattern_modification", "Modify existing patterns (faster, slower, reverse)"},
    };
}

bool StrudelAgent::start() {
    running_ = true;
    return true;
}

void StrudelAgent::stop() {
    running_ = false;
}

void StrudelAgent::setMessageCallback(
    std::function<void(const std::string&, const std::string&)> callback) {
    messageCallback_ = std::move(callback);
}

std::string StrudelAgent::getSystemPrompt() const {
    return R"(You are a Tidal/Strudel pattern generator for a DAW.
You generate mini notation patterns. Respond ONLY with the pattern code, no explanation.

Mini notation syntax:
- Notes: c3 d3 e3 f3 g3 a3 b3 (note + octave)
- Sharps/flats: c#3 db4
- Rest: ~ or -
- Space separates events in a cycle
- Numbers: 60 64 67 (MIDI note numbers)

Examples:
- Simple melody: c3 e3 g3 b3
- With rests: c4 ~ e4 ~ g4 ~ b4 ~
- Drum pattern: 60 ~ 62 ~ 60 60 62 ~
- Fast notes: c3 d3 e3 f3 g3 a3 b3 c4
- Chord (simultaneous): c3 e3 g3

When asked for drums, use MIDI notes:
- Kick: 36
- Snare: 38
- Closed HH: 42
- Open HH: 46
- Tom: 45
- Clap: 39)";
}

std::string StrudelAgent::generatePattern(const std::string& request) {
    // Simple rule-based pattern generation (no LLM needed for basic patterns)
    auto req = juce::String(request).toLowerCase();

    if (req.contains("kick") || req.contains("bass drum")) {
        return "36 ~ ~ ~ 36 ~ ~ ~";
    }
    if (req.contains("snare")) {
        return "~ ~ 38 ~ ~ ~ 38 ~";
    }
    if (req.contains("hihat") || req.contains("hi-hat") || req.contains("hat")) {
        return "42 42 42 42 42 42 42 42";
    }
    if (req.contains("drum") || req.contains("beat") || req.contains("rhythm")) {
        return "36 42 38 42 36 42 38 42";
    }
    if (req.contains("bass")) {
        return "c2 ~ c2 ~ e2 ~ g2 ~";
    }
    if (req.contains("chord") || req.contains("pad")) {
        return "c3 e3 g3 b3";
    }
    if (req.contains("arp")) {
        return "c3 e3 g3 b3 c4 b3 g3 e3";
    }
    if (req.contains("melody") || req.contains("lead")) {
        return "e4 g4 a4 g4 e4 d4 c4 d4";
    }
    if (req.contains("ambient") || req.contains("slow")) {
        return "c3 ~ ~ e3 ~ ~ g3 ~";
    }
    if (req.contains("fast") || req.contains("quick")) {
        return "c4 d4 e4 f4 g4 a4 b4 c5";
    }
    if (req.contains("minor")) {
        return "a3 c4 e4 a4 e4 c4 a3 e3";
    }
    if (req.contains("major")) {
        return "c3 e3 g3 c4 g3 e3 c3 g2";
    }

    // Default: simple pattern
    return "c3 e3 g3 b3";
}

std::string StrudelAgent::processMessage(const std::string& message) {
    if (!running_) return "Agent not running";

    auto pattern = generatePattern(message);

    // Return as a structured response the DAW can parse
    std::string response = "PATTERN:" + pattern;

    if (messageCallback_) {
        messageCallback_(getId(), response);
    }

    return response;
}
