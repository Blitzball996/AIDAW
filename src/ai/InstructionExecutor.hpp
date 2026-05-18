#pragma once

#include "MusicInstruction.hpp"
#include <vector>

namespace aidaw {

class InstructionExecutor {
public:
    void execute(const std::vector<MusicInstruction>& instructions);
    void executeSingle(const MusicInstruction& instruction);

private:
    void handleCreateTrack(const MusicInstruction& instr);
    void handleAddChord(const MusicInstruction& instr);
    void handleAddNote(const MusicInstruction& instr);
    void handleSetTempo(const MusicInstruction& instr);
    void handleSetKey(const MusicInstruction& instr);
};

}  // namespace aidaw
