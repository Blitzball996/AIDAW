#include "harmony_plan.hpp"

#include <algorithm>
#include <cmath>

namespace magda {

// ============================================================================
// ChordStep
// ============================================================================

std::string ChordStep::label(bool minorKey) const {
    if (isSecondaryDom) {
        // V/x — root is a fifth above the tonicized degree; report as V/<deg>.
        return "V/" + theory::romanNumeral(degree, minorKey);
    }
    std::string base = theory::romanNumeral(degree, minorKey);
    if (isBorrowed)
        base = "b" + base;  // rough flag; borrowed roots are usually flatted
    // Append quality/tension shorthand where it is not implied by the numeral.
    if (!tension.empty())
        base += tension;
    else if (quality == "maj7" || quality == "min7" || quality == "7" || quality == "dom7")
        base += "7";
    if (inversion == 1) base += "/3";
    else if (inversion == 2) base += "/5";
    return base;
}

// ============================================================================
// Validation
// ============================================================================

std::vector<HarmonyIssue> validatePlan(const HarmonyPlan& plan) {
    std::vector<HarmonyIssue> out;
    if (plan.chords.empty()) {
        out.push_back({HarmonyIssue::Severity::Warning, -1, "Plan has no chords."});
        return out;
    }

    // Functional flow + tension checks across the progression.
    for (size_t i = 0; i < plan.chords.size(); ++i) {
        const auto& c = plan.chords[i];

        // Tension appropriateness for this chord's function.
        if (!c.tension.empty()) {
            auto w = theory::tensionWarning(c.function, c.tension);
            if (!w.empty())
                out.push_back({HarmonyIssue::Severity::Warning, static_cast<int>(i), w});
        }

        // Functional motion to the next chord.
        if (i + 1 < plan.chords.size()) {
            const auto& n = plan.chords[i + 1];
            auto w = theory::motionWarning(c.function, n.function);
            if (!w.empty())
                out.push_back({HarmonyIssue::Severity::Warning, static_cast<int>(i), w});
        }
    }

    // Cadence check: does the last move form a recognized cadence?
    if (plan.chords.size() >= 2) {
        const auto& penult = plan.chords[plan.chords.size() - 2];
        const auto& last = plan.chords.back();
        auto cad = theory::classifyCadence(penult.degree, last.degree);
        if (cad == theory::Cadence::None) {
            out.push_back({HarmonyIssue::Severity::Info, -1,
                           "Progression does not end on a textbook cadence; consider V-I, IV-I, "
                           "->V, or V-vi."});
        }
    }

    // APPEND_MARKER_VALIDATE
    return out;
}

// ============================================================================
// Summary rendering
// ============================================================================

juce::String summarizePlan(const HarmonyPlan& plan) {
    juce::String s;
    s << "Key: " << plan.keyName << (plan.minorKey ? " minor" : " major");
    if (!plan.style.empty())
        s << "  |  Style: " << juce::String(plan.style);
    s << "  |  " << juce::String(plan.tempo, 0) << " BPM\n";

    s << "Progression: ";
    for (size_t i = 0; i < plan.chords.size(); ++i) {
        const auto& c = plan.chords[i];
        s << juce::String(c.label(plan.minorKey));
        s << " (" << juce::String(c.rootName) << " " << juce::String(c.quality) << ")";
        if (i + 1 < plan.chords.size())
            s << " -> ";
    }
    s << "\n";

    s << "Cadence: " << theory::cadenceName(plan.cadence) << "\n";
    s << "Bass: " << (plan.includeBass ? plan.bassMotion : std::string("(none)")) << "\n";
    s << "Melody: " << (plan.includeMelody ? plan.melodyDirection : std::string("(none)")) << "\n";

    auto issues = validatePlan(plan);
    if (!issues.empty()) {
        s << "Notes:\n";
        for (const auto& is : issues) {
            s << "  - ";
            if (is.chordIndex >= 0)
                s << "[chord " << (is.chordIndex + 1) << "] ";
            s << juce::String(is.message) << "\n";
        }
    }
    return s;
}

// ============================================================================
// HarmonyApprovalEngine
// ============================================================================

const char* stageKindName(StageKind k) {
    switch (k) {
        case StageKind::Key: return "Key";
        case StageKind::Progression: return "Progression";
        case StageKind::Inversions: return "Inversions";
        case StageKind::Tensions: return "Tensions";
        case StageKind::Bass: return "Bass";
        case StageKind::Melody: return "Melody";
        case StageKind::Cadence: return "Cadence";
    }
    return "?";
}

const char* HarmonyApprovalEngine::getModeName() const {
    switch (mode_) {
        case ReviewMode::STEP_BY_STEP: return "step";
        case ReviewMode::SMART: return "smart";
        case ReviewMode::ALLOW_ALL: return "allow-all";
    }
    return "?";
}

std::vector<ReviewStage> HarmonyApprovalEngine::buildStages(const HarmonyPlan& plan) {
    std::vector<ReviewStage> stages;
    auto allIssues = validatePlan(plan);

    auto issuesForStage = [&](StageKind k) {
        std::vector<HarmonyIssue> v;
        for (const auto& is : allIssues) {
            bool match = false;
            if (k == StageKind::Tensions && is.message.find("Tension") != std::string::npos)
                match = true;
            else if (k == StageKind::Progression &&
                     (is.message.find("Retrogression") != std::string::npos ||
                      is.message.find("no chords") != std::string::npos))
                match = true;
            else if (k == StageKind::Cadence && is.message.find("cadence") != std::string::npos)
                match = true;
            if (match) v.push_back(is);
        }
        return v;
    };

    // Stage 1: Key
    {
        ReviewStage st{StageKind::Key, "Key / tonal center", {}, {}};
        st.detail = juce::String(plan.keyName) + (plan.minorKey ? " minor" : " major") + ", " +
                    juce::String(plan.tempo, 0) + " BPM";
        stages.push_back(std::move(st));
    }
    // Stage 2: Progression
    {
        ReviewStage st{StageKind::Progression, "Chord progression", {}, issuesForStage(StageKind::Progression)};
        juce::String d;
        for (size_t i = 0; i < plan.chords.size(); ++i) {
            d << juce::String(plan.chords[i].label(plan.minorKey));
            if (i + 1 < plan.chords.size()) d << " -> ";
        }
        st.detail = d;
        stages.push_back(std::move(st));
    }
    // Stage 3: Inversions
    {
        ReviewStage st{StageKind::Inversions, "Inversions / voice leading", {}, {}};
        juce::String d;
        bool any = false;
        for (const auto& c : plan.chords) {
            if (c.inversion != 0) any = true;
            d << juce::String(c.rootName) << (c.inversion == 1 ? "/3 " : c.inversion == 2 ? "/5 " : " ");
        }
        st.detail = any ? d : juce::String("All root position");
        stages.push_back(std::move(st));
    }
    // Stage 4: Tensions
    {
        ReviewStage st{StageKind::Tensions, "Tensions (add9/sus/7/...)", {}, issuesForStage(StageKind::Tensions)};
        juce::String d;
        bool any = false;
        for (const auto& c : plan.chords) {
            if (!c.tension.empty()) { any = true; d << juce::String(c.rootName) << ":" << juce::String(c.tension) << " "; }
        }
        st.detail = any ? d : juce::String("No added tensions");
        stages.push_back(std::move(st));
    }
    // Stage 5: Bass (only if requested)
    if (plan.includeBass) {
        ReviewStage st{StageKind::Bass, "Bass line", {}, {}};
        st.detail = juce::String(plan.bassMotion);
        stages.push_back(std::move(st));
    }
    // Stage 6: Melody (only if requested)
    if (plan.includeMelody) {
        ReviewStage st{StageKind::Melody, "Melody direction", {}, {}};
        st.detail = juce::String(plan.melodyDirection);
        stages.push_back(std::move(st));
    }
    // Stage 7: Cadence
    {
        ReviewStage st{StageKind::Cadence, "Cadence", {}, issuesForStage(StageKind::Cadence)};
        st.detail = juce::String(theory::cadenceName(plan.cadence));
        stages.push_back(std::move(st));
    }
    return stages;
}

namespace {
// Apply a user-edited stage detail string back onto the plan. Edits are
// best-effort and intentionally forgiving: anything we cannot parse is left
// untouched so a stray keystroke never corrupts the plan.
void applyStageEdit(HarmonyPlan& plan, const ReviewStage& stage, const juce::String& edited) {
    auto txt = edited.trim();
    switch (stage.kind) {
        case StageKind::Key: {
            // "<Key> [minor|major][, <bpm> BPM]"
            auto lower = txt.toLowerCase();
            plan.minorKey = lower.contains("min");
            auto tok = txt.upToFirstOccurrenceOf(" ", false, false).trim();
            if (tok.isNotEmpty()) {
                plan.keyName = tok.toStdString();
                int pc = music::parseNoteName((plan.keyName + "0"));
                if (pc >= 0) plan.tonicSemitone = pc % 12;
            }
            auto bpmIdx = lower.indexOf("bpm");
            if (bpmIdx > 0) {
                auto num = txt.substring(0, bpmIdx).retainCharacters("0123456789.").getDoubleValue();
                if (num > 0) plan.tempo = num;
            }
            break;
        }
        case StageKind::Bass:
            plan.bassMotion = txt.toStdString();
            plan.includeBass = !txt.equalsIgnoreCase("none") && txt.isNotEmpty();
            break;
        case StageKind::Melody:
            plan.melodyDirection = txt.toStdString();
            plan.includeMelody = !txt.equalsIgnoreCase("none") && txt.isNotEmpty();
            break;
        default:
            // Progression / inversion / tension / cadence edits are structural;
            // we keep the original structured plan and only record the user's
            // intent in the description so the agent can honor it next turn.
            plan.description += "\n[user edit @" + std::string(stageKindName(stage.kind)) +
                                "]: " + txt.toStdString();
            break;
    }
}
}  // namespace

bool HarmonyApprovalEngine::review(HarmonyPlan& plan, juce::String& rejectedOut) {
    if (mode_ == ReviewMode::ALLOW_ALL)
        return true;
    if (!askFn_)
        return true;  // no UI hooked up — fail open, like BYPASS

    auto stages = buildStages(plan);
    for (const auto& stage : stages) {
        // SMART mode: auto-approve any stage that carries no warnings.
        if (mode_ == ReviewMode::SMART) {
            bool hasWarning = std::any_of(stage.issues.begin(), stage.issues.end(),
                                          [](const HarmonyIssue& i) {
                                              return i.severity == HarmonyIssue::Severity::Warning;
                                          });
            if (!hasWarning)
                continue;
        }

        juce::String edited = stage.detail;
        auto decision = askFn_(stage, edited);
        if (decision == StageDecision::Reject) {
            rejectedOut = stage.title;
            return false;
        }
        if (decision == StageDecision::Edit)
            applyStageEdit(plan, stage, edited);
    }
    return true;
}

// ============================================================================
// Plan -> IR conversion
// ============================================================================

namespace {
// Merge a base triad quality with an optional tension label into a single
// music::chordQualities() key. Falls back to the base quality if the
// combination is not a known extended quality.
std::string resolveQuality(const std::string& base, const std::string& tension) {
    if (tension.empty()) return base;
    const auto& q = music::chordQualities();
    bool minorBase = (base == "min" || base == "minor");

    // Suspensions replace the third entirely.
    if (tension == "sus2" || tension == "sus4")
        return tension;
    if (tension == "add9")
        return minorBase ? (q.count("madd9") ? "madd9" : "add9") : "add9";
    if (tension == "6")
        return minorBase ? (q.count("min6") ? "min6" : "6") : "6";
    if (tension == "maj7")
        return q.count("maj7") ? "maj7" : base;
    if (tension == "7")
        return minorBase ? (q.count("min7") ? "min7" : "7") : "7";
    if (tension == "9")
        return minorBase ? (q.count("min9") ? "min9" : "9") : "9";
    if (tension == "11")
        return minorBase ? (q.count("min11") ? "min11" : "11") : "11";
    if (tension == "13")
        return minorBase ? (q.count("min13") ? "min13" : "13") : "13";
    // Unknown tension: try base+tension, else base.
    if (q.count(base + tension)) return base + tension;
    return base;
}
}  // namespace

void planToInstructions(const HarmonyPlan& plan, std::vector<Instruction>& out) {
    for (const auto& c : plan.chords) {
        ChordOp op;
        op.root = juce::String(c.rootName) + juce::String(plan.octave);
        op.quality = juce::String(resolveQuality(c.quality, c.tension));
        op.beat = c.startBeat;
        op.length = c.lengthBeats;
        op.inversion = c.inversion;
        out.push_back({OpCode::Chord, op});
    }

    // Bass line: one root note per chord, an octave below the chord octave.
    if (plan.includeBass) {
        int bassOctave = std::max(0, plan.octave - 1);
        for (const auto& c : plan.chords) {
            NoteOp n;
            n.pitch = juce::String(c.rootName) + juce::String(bassOctave);
            n.beat = c.startBeat;
            n.length = c.lengthBeats;
            n.velocity = 90;
            out.push_back({OpCode::Note, n});
            if (plan.bassMotion == "octaves") {
                NoteOp hi = n;
                hi.pitch = juce::String(c.rootName) + juce::String(bassOctave + 1);
                hi.beat = c.startBeat + c.lengthBeats / 2.0;
                hi.length = c.lengthBeats / 2.0;
                out.push_back({OpCode::Note, hi});
            }
        }
    }

    // Melody: chord-tone guide line following the requested contour. Uses the
    // chord's top tone (5th by default) and nudges it by direction so the agent
    // gets a sensible starting melody the user can refine.
    if (plan.includeMelody) {
        int melOctave = plan.octave + 1;
        size_t i = 0, n = plan.chords.size();
        for (const auto& c : plan.chords) {
            int third = music::parseNoteName(c.rootName + std::to_string(melOctave));
            if (third < 0) { ++i; continue; }
            // direction offset in scale steps
            int off = 0;
            double t = (n > 1) ? static_cast<double>(i) / static_cast<double>(n - 1) : 0.0;
            if (plan.melodyDirection == "ascending") off = static_cast<int>(t * 7);
            else if (plan.melodyDirection == "descending") off = -static_cast<int>(t * 7);
            else if (plan.melodyDirection == "arch") off = static_cast<int>(std::sin(t * 3.14159) * 5);
            NoteOp m;
            int pitch = juce::jlimit(0, 127, third + off);
            m.pitch = juce::String(pitch);
            m.beat = c.startBeat;
            m.length = std::min(c.lengthBeats, 1.0);
            m.velocity = 100;
            out.push_back({OpCode::Note, m});
            ++i;
        }
    }
}

// ============================================================================
// IR -> Plan reconstruction
// ============================================================================

namespace {
// Map a chord-quality string to {base triad quality, tension label}.
std::pair<std::string, std::string> splitQuality(const std::string& q) {
    if (q == "sus2" || q == "sus4") return {"major", q};
    if (q == "add9") return {"major", "add9"};
    if (q == "madd9") return {"min", "add9"};
    if (q == "maj7" || q == "maj9" || q == "maj11" || q == "maj13") return {"major", "maj7"};
    if (q == "min7" || q == "min9" || q == "min11" || q == "min13") return {"min", "7"};
    if (q == "7" || q == "dom7" || q == "9" || q == "11" || q == "13") return {"major", "7"};
    if (q == "6" || q == "maj6") return {"major", "6"};
    if (q == "min6") return {"min", "6"};
    if (q == "min" || q == "minor") return {"min", ""};
    if (q == "dim" || q == "dim7") return {"dim", ""};
    if (q == "aug") return {"aug", ""};
    return {"major", ""};
}

// Parse "C major" / "a minor" style hint -> {tonicSemitone, minorKey}.
bool parseKeyHint(const juce::String& hint, int& tonicOut, bool& minorOut) {
    auto h = hint.trim();
    if (h.isEmpty()) return false;
    auto tok = h.upToFirstOccurrenceOf(" ", false, false).trim();
    int pc = music::parseNoteName(tok.toStdString() + "0");
    if (pc < 0) return false;
    tonicOut = pc % 12;
    minorOut = h.toLowerCase().contains("min");
    return true;
}
}  // namespace

HarmonyPlan planFromInstructions(const std::vector<Instruction>& instrs,
                                 const juce::String& keyHint) {
    HarmonyPlan plan;

    // Collect chord ops in order.
    std::vector<const ChordOp*> chordOps;
    int noteCount = 0;
    for (const auto& in : instrs) {
        if (in.opcode == OpCode::Chord)
            chordOps.push_back(&std::get<ChordOp>(in.payload));
        else if (in.opcode == OpCode::Note)
            ++noteCount;
    }
    if (chordOps.empty()) {
        plan.includeMelody = noteCount > 0;  // pure note/melody output
        return plan;
    }

    // Key: use the hint, else assume the first chord is the tonic.
    int tonic = 0;
    bool minorKey = false;
    if (!parseKeyHint(keyHint, tonic, minorKey)) {
        int r = music::parseNoteName(chordOps.front()->root.toStdString());
        if (r >= 0) tonic = r % 12;
        auto sq = splitQuality(chordOps.front()->quality.toStdString());
        minorKey = (sq.first == "min");
    }
    plan.tonicSemitone = tonic;
    plan.minorKey = minorKey;
    plan.keyName = theory::pitchClassName(tonic);

    // Base octave from the first chord root.
    {
        int r = music::parseNoteName(chordOps.front()->root.toStdString());
        if (r >= 0) plan.octave = (r / 12) - 1;
    }

    // Build chord steps.
    for (const auto* op : chordOps) {
        ChordStep st;
        int r = music::parseNoteName(op->root.toStdString());
        int sem = (r >= 0) ? (r % 12) : tonic;
        st.rootSemitone = sem;
        st.rootName = theory::pitchClassName(sem);
        auto sq = splitQuality(op->quality.toStdString());
        st.quality = sq.first;
        st.tension = sq.second;
        st.inversion = op->inversion;
        st.startBeat = op->beat;
        st.lengthBeats = op->length > 0 ? op->length : 4.0;

        // Scale degree relative to tonic.
        int rel = ((sem - tonic) % 12 + 12) % 12;
        const auto& iv = theory::scaleIntervals(minorKey ? theory::ScaleType::NaturalMinor
                                                         : theory::ScaleType::Major);
        st.degree = 1;
        st.isBorrowed = true;
        for (int d = 0; d < 7; ++d) {
            if (iv[d] == rel) { st.degree = d + 1; st.isBorrowed = false; break; }
        }
        st.function = theory::degreeFunction(st.degree, minorKey);
        plan.chords.push_back(std::move(st));
    }

    // Cadence from the last two degrees.
    if (plan.chords.size() >= 2) {
        plan.cadence = theory::classifyCadence(plan.chords[plan.chords.size() - 2].degree,
                                               plan.chords.back().degree);
    }

    plan.includeBass = false;       // reconstructed plans default to chords only
    plan.includeMelody = false;
    return plan;
}

}  // namespace magda
