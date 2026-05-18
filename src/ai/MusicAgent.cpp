#include "MusicAgent.hpp"
#include <sstream>

namespace aidaw {

MusicAgent::MusicAgent(LlmClient* client)
    : llmClient(client) {}

const char* MusicAgent::getSystemPrompt() {
    return R"(You are a music composition AI. Given a user's description, generate music instructions.

Output format (one per line):
TEMPO <bpm>
KEY <key> <scale>
TIME <numerator>/<denominator>
TRACK <name> <instrument>
CHORD <root> <quality> <bar> <beat> <duration>
NOTE <pitch> <bar> <beat> <duration> <velocity>
DRUM <pattern_name> <bar> <duration>
ARP <root> <quality> <pattern> <bar> <beat> <duration>

Example:
TEMPO 120
KEY C major
TIME 4/4
TRACK piano Piano
CHORD C maj 1 1 4
CHORD F maj 2 1 4
CHORD G maj 3 1 4
CHORD C maj 4 1 4
)";
}

MusicAgent::GenerateResult MusicAgent::generate(const std::string& userMessage) {
    GenerateResult result;

    if (!llmClient || !llmClient->isAvailable()) {
        result.hasError = true;
        result.error = "LLM client not available";
        return result;
    }

    LlmRequest request;
    request.systemPrompt = getSystemPrompt();
    request.messages.push_back({"user", userMessage});
    request.temperature = 0.7f;
    request.maxTokens = 2048;

    auto response = llmClient->send(request);
    if (!response.success) {
        result.hasError = true;
        result.error = response.error;
        return result;
    }

    result.rawOutput = response.content;
    result.instructions = parseOutput(response.content);
    return result;
}

MusicAgent::GenerateResult MusicAgent::generateStreaming(const std::string& userMessage,
                                                          TokenCallback onToken) {
    GenerateResult result;

    if (!llmClient || !llmClient->isAvailable()) {
        result.hasError = true;
        result.error = "LLM client not available";
        return result;
    }

    LlmRequest request;
    request.systemPrompt = getSystemPrompt();
    request.messages.push_back({"user", userMessage});

    std::string accumulated;
    auto response = llmClient->sendStreaming(request, [&](const std::string& token) {
        accumulated += token;
        if (onToken) onToken(token);
    });

    if (!response.success) {
        result.hasError = true;
        result.error = response.error;
        return result;
    }

    result.rawOutput = accumulated.empty() ? response.content : accumulated;
    result.instructions = parseOutput(result.rawOutput);
    return result;
}

std::vector<MusicInstruction> MusicAgent::parseOutput(const std::string& output) {
    std::vector<MusicInstruction> instructions;
    std::istringstream stream(output);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        MusicInstruction instr;
        std::istringstream ls(line);
        std::string cmd;
        ls >> cmd;

        if (cmd == "TEMPO") {
            instr.type = InstructionType::SetTempo;
            std::string bpm;
            ls >> bpm;
            instr.params.push_back(bpm);
        } else if (cmd == "KEY") {
            instr.type = InstructionType::SetKey;
            std::string key, scale;
            ls >> key >> scale;
            instr.params.push_back(key);
            instr.params.push_back(scale);
        } else if (cmd == "TRACK") {
            instr.type = InstructionType::CreateTrack;
            std::string name, instrument;
            ls >> name >> instrument;
            instr.target = name;
            instr.params.push_back(instrument);
        } else if (cmd == "CHORD") {
            instr.type = InstructionType::AddChord;
            std::string root, quality;
            ls >> root >> quality >> instr.bar >> instr.beat >> instr.duration;
            instr.params.push_back(root);
            instr.params.push_back(quality);
        } else if (cmd == "NOTE") {
            instr.type = InstructionType::AddNote;
            std::string pitch;
            ls >> pitch >> instr.bar >> instr.beat >> instr.duration >> instr.velocity;
            instr.params.push_back(pitch);
        } else {
            continue;
        }

        instructions.push_back(instr);
    }

    return instructions;
}

}  // namespace aidaw
