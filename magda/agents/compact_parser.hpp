#pragma once

#include <juce_core/juce_core.h>

#include <functional>
#include <string>
#include <variant>
#include <vector>

namespace magda {

/** Called for each streamed token. Return false to cancel. */
using TokenCallback = std::function<bool(const juce::String& token)>;

// ============================================================================
// IR Instruction Types
// ============================================================================

enum class OpCode {
    Track,       // Create or reference track
    Del,         // Delete track
    Mute,        // Mute track by name
    Solo,        // Solo track by name
    Set,         // Set track properties
    Param,       // Set parameters on a track's instrument / last-added plugin
    Clip,        // Create clip
    Fx,          // Add FX
    Select,      // Select clips/tracks by criteria
    Arp,         // Add arpeggio (on last clip target)
    Chord,       // Add chord (on last clip target)
    Note,        // Add note (on last clip target)
    TrackSwitch, // Switch target track by name (multi-track music generation)
    Repeat,      // Repeat a beat range N times
    Ask,         // Agent needs a decision from the user; nothing is applied
    Say,         // Agent note shown alongside whatever was applied
};

/** How a track is referenced — by 1-based index, by name, or implicitly (last TRACK). */
struct TrackRef {
    int id = -1;  // 1-based index, or -1 if by name
    juce::String name;
    bool implicit = false;  // true = use last TRACK context

    bool isById() const {
        return id > 0;
    }
    bool isImplicit() const {
        return implicit;
    }
};

// --- Per-opcode payloads ---------------------------------------------------

struct TrackOp {
    juce::String name;
    juce::String fxAlias;  // non-empty = create track + add this plugin, name from plugin
};

struct DelOp {
    TrackRef target;
};

struct MuteOp {
    TrackRef target;  // implicit/by-id/by-name — falls back to current track
};

struct SoloOp {
    TrackRef target;  // implicit/by-id/by-name — falls back to current track
};

struct SetOp {
    TrackRef target;
    juce::StringPairArray props;  // key=value pairs (vol, pan, mute, solo …)
};

/** Set parameters on a plugin rather than on the track itself.
    Targets the track's last-added plugin, falling back to its instrument. */
struct ParamOp {
    TrackRef target;
    juce::StringPairArray params;  // parameter name → value, matched loosely
};

struct ClipOp {
    TrackRef target;
    double bar = 1.0;
    double lengthBars = 4.0;
    juce::String name;  // optional clip name
};

struct FxOp {
    TrackRef target;  // may be implicit (use current track)
    juce::String fxName;
};

struct SelectOp {
    enum class Target { Clips, Tracks };
    Target target = Target::Clips;

    // Optional predicate: field op value
    juce::String field;  // "length", "bar", "track", "name" (empty = select all)
    juce::String op;     // "<", ">", "<=", ">=", "=", "!="
    juce::String value;  // number (bars) or quoted string
};

struct ArpOp {
    juce::String root;
    juce::String quality;
    double beat = 0.0;
    double step = 0.5;
    double beats = -1.0;   // -1 = not specified
    int inversion = 0;     // 0=root, 1=first, 2=second
    juce::String pattern;  // "up", "down", "updown" (empty = up)
};

struct ChordOp {
    juce::String root;
    juce::String quality;
    double beat = 0.0;
    double length = 1.0;
    int velocity = -1;  // -1 = not specified
    int inversion = 0;  // 0=root, 1=first, 2=second
};

struct NoteOp {
    juce::String pitch;
    double beat = 0.0;
    double length = 1.0;
    int velocity = -1;  // -1 = not specified
};

struct TrackSwitchOp {
    juce::String name;  // target track name
};

struct RepeatOp {
    double fromBeat = 0.0;
    double toBeat = 0.0;
    double pasteAt = 0.0;
    int times = 1;
};

/** The agent needs the user to decide something before it can act.

    This is the agent's only way to say "I need input" — without it, a model
    that asks a question just produces unparseable prose that gets dropped, and
    the user sees nothing happen. An Ask aborts the batch: nothing is applied,
    because a model that is unsure should not guess and half-write first. */
struct AskOp {
    juce::String question;
};

/** A note to show the user alongside whatever was applied. Non-blocking —
    use this for "I did X, you might also want Y", not for real questions. */
struct SayOp {
    juce::String message;
};

using OpPayload = std::variant<TrackOp, DelOp, MuteOp, SoloOp, SetOp, ParamOp, ClipOp, FxOp,
                               SelectOp, ArpOp, ChordOp, NoteOp, TrackSwitchOp, RepeatOp, AskOp,
                               SayOp>;

struct Instruction {
    OpCode opcode;
    OpPayload payload;
};

// ============================================================================
// Compact Parser — LLM text → IR instructions
// ============================================================================

class CompactParser {
  public:
    /**
     * @brief Parse compact assembler output into IR instructions.
     * @param compact  Multi-line compact instructions from the LLM
     * @return List of instructions, empty on error (check getLastError())
     */
    std::vector<Instruction> parse(const juce::String& compact);

    juce::String getLastError() const {
        return lastError_;
    }

  private:
    static TrackRef parseRef(const juce::String& token);
    static bool isInteger(const juce::String& s);

    juce::String lastError_;
};

}  // namespace magda
