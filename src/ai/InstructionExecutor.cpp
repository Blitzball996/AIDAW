#include "InstructionExecutor.hpp"

namespace aidaw {

void InstructionExecutor::execute(const std::vector<MusicInstruction>& instructions) {
    for (const auto& instr : instructions) {
        executeSingle(instr);
    }
}

void InstructionExecutor::executeSingle(const MusicInstruction& instruction) {
    switch (instruction.type) {
        case InstructionType::CreateTrack:  handleCreateTrack(instruction); break;
        case InstructionType::AddChord:     handleAddChord(instruction); break;
        case InstructionType::AddNote:      handleAddNote(instruction); break;
        case InstructionType::SetTempo:     handleSetTempo(instruction); break;
        case InstructionType::SetKey:       handleSetKey(instruction); break;
        default: break;
    }
}

void InstructionExecutor::handleCreateTrack(const MusicInstruction& /*instr*/) {
    // TODO: call TrackManager to create a new track with instrument
}

void InstructionExecutor::handleAddChord(const MusicInstruction& /*instr*/) {
    // TODO: add MIDI chord to clip at specified position
}

void InstructionExecutor::handleAddNote(const MusicInstruction& /*instr*/) {
    // TODO: add MIDI note to clip
}

void InstructionExecutor::handleSetTempo(const MusicInstruction& /*instr*/) {
    // TODO: set transport tempo
}

void InstructionExecutor::handleSetKey(const MusicInstruction& /*instr*/) {
    // TODO: set project key signature
}

}  // namespace aidaw
