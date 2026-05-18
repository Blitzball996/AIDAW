#pragma once

#include <string>
#include <vector>

namespace aidaw {

enum class InstructionType {
    CreateTrack,
    AddChord,
    AddNote,
    AddArpeggio,
    SetTempo,
    SetKey,
    SetTimeSignature,
    AddDrum,
    AddBass,
    AddMelody,
    SetInstrument,
    Repeat,
    Delete,
    Modify
};

struct MusicInstruction {
    InstructionType type;
    std::string target;
    std::vector<std::string> params;
    int bar = 0;
    int beat = 0;
    float duration = 1.0f;
    int velocity = 100;
};

}  // namespace aidaw
