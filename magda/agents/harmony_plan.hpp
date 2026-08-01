#pragma once

#include <juce_core/juce_core.h>

#include <functional>
#include <string>
#include <vector>

#include "harmony_theory.hpp"
#include "compact_parser.hpp"

namespace magda {

/**
 * @brief A structured, reviewable harmony proposal.
 *
 * The music agent (or a deterministic generator) fills a HarmonyPlan. Before
 * any note is written, the plan is surfaced to the user for approval — either
 * step by step (chord, then bass, then melody, then tensions) or all at once
 * ("allow all"). This is the human-in-the-loop layer: the user decides key,
 * progression, whether to use inversions, which tensions (add9/sus/...), the
 * bass motion and the melodic direction before anything touches the timeline.
 *
 * Modeled on CloseCrab's PermissionEngine (DEFAULT / AUTO / BYPASS) — here the
 * equivalents are STEP_BY_STEP / SMART / ALLOW_ALL.
 */

// === A single chord slot in the progression ================================
struct ChordStep {
    int degree = 1;             // roman-numeral scale degree 1..7
    int rootSemitone = 0;       // semitone above tonic (0..11), incl. borrowed
    std::string rootName;       // e.g. "C", "F#"
    std::string quality;        // music::chordQualities() key, e.g. "maj7"
    int inversion = 0;          // 0=root, 1=first, 2=second
    std::string tension;        // optional added tone label ("add9","sus4",...)
    double startBeat = 0.0;     // within the clip
    double lengthBeats = 4.0;
    bool isBorrowed = false;    // modal interchange
    bool isSecondaryDom = false;
    theory::Function function = theory::Function::Tonic;

    /// Human-readable roman-numeral label, e.g. "ii7" or "V/V".
    std::string label(bool minorKey) const;
};

// === The full plan ==========================================================
struct HarmonyPlan {
    // Tonal center
    std::string keyName = "C";       // tonic pitch-class name
    int tonicSemitone = 0;           // 0..11
    bool minorKey = false;
    int octave = 4;                  // base octave for chord roots
    double tempo = 120.0;            // informational
    std::string style;               // "jazz","pop","cinematic",... (informational)

    // The progression
    std::vector<ChordStep> chords;

    // Optional generated layers (filled only when the user opts in)
    bool includeBass = false;
    bool includeMelody = false;
    std::string bassMotion = "roots";   // "roots","walking","octaves","pedal"
    std::string melodyDirection = "arch";  // "arch","ascending","descending","static"

    // The cadence that closes the progression
    theory::Cadence cadence = theory::Cadence::None;

    // Free-form description the agent gives the user.
    std::string description;

    bool empty() const { return chords.empty(); }
};

// === Validation =============================================================
// Checks a plan against harmony_theory rules and returns human-readable
// warnings (it never rejects — the user is the final authority).
struct HarmonyIssue {
    enum class Severity { Info, Warning } severity = Severity::Warning;
    int chordIndex = -1;   // -1 == plan-level
    std::string message;
};

std::vector<HarmonyIssue> validatePlan(const HarmonyPlan& plan);

/// Render the plan as a readable multi-line summary for the chat / approval UI.
juce::String summarizePlan(const HarmonyPlan& plan);

/**
 * @brief Convert an approved plan into executor IR instructions.
 *
 * Emits one CHORD op per chord step (with inversion / tension applied) and,
 * when requested, NOTE ops for a bass line and/or melody. The instructions are
 * appended to `out`; callers feed them to InstructionExecutor.
 */
void planToInstructions(const HarmonyPlan& plan, std::vector<Instruction>& out);

/**
 * @brief Derive a reviewable HarmonyPlan from raw chord/note IR.
 *
 * The music agent currently emits CHORD/NOTE ops directly. To bring those under
 * human review we reconstruct a structured plan: infer the key from the chord
 * roots, classify each chord's scale degree and function, detect inversions and
 * tensions, and detect bass/melody layers. The reconstruction is heuristic but
 * good enough to drive the approval UI; the original IR remains the source of
 * truth for anything the user does not edit.
 *
 * @param instrs  parsed IR from the music agent
 * @param keyHint optional "C major"/"A minor" hint (from memory/context); may be empty
 */
HarmonyPlan planFromInstructions(const std::vector<Instruction>& instrs,
                                 const juce::String& keyHint = {});

// === Approval engine ========================================================
// Mirrors CloseCrab's PermissionEngine: an interaction mode plus an "ask"
// callback the UI supplies. The engine walks the plan as a sequence of review
// stages; for each stage it either auto-approves (ALLOW_ALL) or calls back into
// the UI to let the user approve / edit / reject.

enum class ReviewMode {
    STEP_BY_STEP,  // ask the user at every stage (like CloseCrab DEFAULT)
    SMART,         // auto-approve stages with no theory warnings, ask on warnings (AUTO)
    ALLOW_ALL      // apply the whole plan, no prompts (BYPASS)
};

enum class StageKind { Key, Progression, Inversions, Tensions, Bass, Melody, Cadence };

const char* stageKindName(StageKind k);

struct ReviewStage {
    StageKind kind;
    juce::String title;          // "Chord progression"
    juce::String detail;         // the proposed content for this stage
    std::vector<HarmonyIssue> issues;  // theory warnings for this stage
};

enum class StageDecision { Approve, Edit, Reject };

// The UI implements this. `editedDetail` is meaningful only for Edit.
using AskStageFn =
    std::function<StageDecision(const ReviewStage& stage, juce::String& editedDetail)>;

class HarmonyApprovalEngine {
  public:
    void setMode(ReviewMode m) { mode_ = m; }
    ReviewMode getMode() const { return mode_; }
    const char* getModeName() const;

    void setAskCallback(AskStageFn fn) { askFn_ = std::move(fn); }

    /// Build the ordered review stages for a plan (key, progression, ...).
    static std::vector<ReviewStage> buildStages(const HarmonyPlan& plan);

    /**
     * @brief Run the approval flow over a plan.
     *
     * Returns true if the plan is approved (in whole or after edits) and should
     * be applied; false if the user rejected it. In ALLOW_ALL mode this returns
     * true immediately without invoking the callback.
     *
     * @param plan      the proposal (may be mutated by accepted edits)
     * @param rejectedOut on false, the stage name the user rejected at.
     */
    bool review(HarmonyPlan& plan, juce::String& rejectedOut);

  private:
    ReviewMode mode_ = ReviewMode::STEP_BY_STEP;
    AskStageFn askFn_;
};

}  // namespace magda
