#include "PromptParser.hpp"
#include <sstream>

namespace aidaw {

std::vector<MusicInstruction> PromptParser::parseDirectCommands(const std::string& input) {
    std::vector<MusicInstruction> instructions;
    // Direct command parsing for simple cases like "tempo 120" or "key C major"
    std::istringstream stream(input);
    std::string word;
    stream >> word;

    if (word == "tempo" || word == "bpm") {
        MusicInstruction instr;
        instr.type = InstructionType::SetTempo;
        std::string val;
        stream >> val;
        instr.params.push_back(val);
        instructions.push_back(instr);
    } else if (word == "key") {
        MusicInstruction instr;
        instr.type = InstructionType::SetKey;
        std::string key, scale;
        stream >> key >> scale;
        instr.params.push_back(key);
        instr.params.push_back(scale);
        instructions.push_back(instr);
    }

    return instructions;
}

std::string PromptParser::buildMusicPrompt(const std::string& userInput,
                                            const std::string& context) {
    std::string prompt = userInput;
    if (!context.empty()) {
        prompt = "Current project context: " + context + "\n\nUser request: " + userInput;
    }
    return prompt;
}

}  // namespace aidaw
