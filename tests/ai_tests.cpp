#include "ai/MusicInstruction.hpp"
#include "ai/PromptParser.hpp"
#include <cassert>
#include <iostream>

using namespace aidaw;

void testPromptParser() {
    PromptParser parser;

    auto instructions = parser.parseDirectCommands("tempo 120");
    assert(instructions.size() == 1);
    assert(instructions[0].type == InstructionType::SetTempo);
    assert(instructions[0].params[0] == "120");

    auto keyInstr = parser.parseDirectCommands("key C major");
    assert(keyInstr.size() == 1);
    assert(keyInstr[0].type == InstructionType::SetKey);

    std::cout << "PromptParser tests passed\n";
}

void testMusicInstruction() {
    MusicInstruction instr;
    instr.type = InstructionType::AddChord;
    instr.params = {"C", "maj"};
    instr.bar = 1;
    instr.beat = 1;
    instr.duration = 4.0f;

    assert(instr.params.size() == 2);
    assert(instr.bar == 1);

    std::cout << "MusicInstruction tests passed\n";
}

int main() {
    testPromptParser();
    testMusicInstruction();
    std::cout << "All AI tests passed!\n";
    return 0;
}
