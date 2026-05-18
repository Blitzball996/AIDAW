#pragma once

#include "MusicInstruction.hpp"
#include <string>
#include <vector>

namespace aidaw {

class PromptParser {
public:
    std::vector<MusicInstruction> parseDirectCommands(const std::string& input);
    std::string buildMusicPrompt(const std::string& userInput, const std::string& context = "");
};

}  // namespace aidaw
