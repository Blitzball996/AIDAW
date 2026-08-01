#pragma once

#include <juce_core/juce_core.h>

#include <array>
#include <map>
#include <string>
#include <vector>

#include "music_helpers.hpp"

namespace magda {

/**
 * @brief Music-theory constraint reference (functional harmony).
 *
 * Header-only, DAW-free so the prompt builder, the harmony validator and unit
 * tests can all share it. Distilled from common-practice + popular functional
 * harmony: diatonic function (T/S/D), functional flow, cadences, secondary
 * dominants, modal interchange, and tension appropriateness.
 */
namespace theory {

// === Scales: semitone offsets from the tonic ================================
enum class ScaleType {
    Major, NaturalMinor, HarmonicMinor, MelodicMinor,
    Dorian, Phrygian, Lydian, Mixolydian, Locrian
};

inline const std::vector<int>& scaleIntervals(ScaleType s) {
    static const std::map<ScaleType, std::vector<int>> table = {
        {ScaleType::Major, {0, 2, 4, 5, 7, 9, 11}},
        {ScaleType::NaturalMinor, {0, 2, 3, 5, 7, 8, 10}},
        {ScaleType::HarmonicMinor, {0, 2, 3, 5, 7, 8, 11}},
        {ScaleType::MelodicMinor, {0, 2, 3, 5, 7, 9, 11}},
        {ScaleType::Dorian, {0, 2, 3, 5, 7, 9, 10}},
        {ScaleType::Phrygian, {0, 1, 3, 5, 7, 8, 10}},
        {ScaleType::Lydian, {0, 2, 4, 6, 7, 9, 11}},
        {ScaleType::Mixolydian, {0, 2, 4, 5, 7, 9, 10}},
        {ScaleType::Locrian, {0, 1, 3, 5, 6, 8, 10}},
    };
    return table.at(s);
}

// === Harmonic function ======================================================
// Tonal function of a diatonic scale degree. Every diatonic chord groups into
// one of three families:
//   Tonic (T):       I, vi, iii  — rest / arrival
//   Subdominant (S): IV, ii      — motion away, prepares the dominant
//   Dominant (D):    V, vii      — tension that resolves to T

enum class Function { Tonic, Subdominant, Dominant };

inline const char* functionName(Function f) {
    switch (f) {
        case Function::Tonic: return "Tonic";
        case Function::Subdominant: return "Subdominant";
        case Function::Dominant: return "Dominant";
    }
    return "?";
}

/** Function of each diatonic degree (1-7) in MAJOR. */
inline Function majorDegreeFunction(int degree) {
    switch (degree) {
        case 1: return Function::Tonic;        // I
        case 2: return Function::Subdominant;  // ii
        case 3: return Function::Tonic;        // iii
        case 4: return Function::Subdominant;  // IV
        case 5: return Function::Dominant;     // V
        case 6: return Function::Tonic;        // vi
        case 7: return Function::Dominant;     // vii
        default: return Function::Tonic;
    }
}

/** Function of each diatonic degree (1-7) in MINOR. */
inline Function minorDegreeFunction(int degree) {
    switch (degree) {
        case 1: return Function::Tonic;        // i
        case 2: return Function::Subdominant;  // ii°
        case 3: return Function::Tonic;        // III
        case 4: return Function::Subdominant;  // iv
        case 5: return Function::Dominant;     // V/v
        case 6: return Function::Subdominant;  // VI
        case 7: return Function::Dominant;     // VII/vii°
        default: return Function::Tonic;
    }
}

inline Function degreeFunction(int degree, bool minorKey) {
    return minorKey ? minorDegreeFunction(degree) : majorDegreeFunction(degree);
}

/**
 * @brief Default triad quality of a diatonic degree.
 * @return quality string compatible with music::chordQualities().
 */
inline std::string diatonicTriadQuality(int degree, bool minorKey) {
    if (!minorKey) {
        static const std::array<const char*, 7> q = {
            "major", "min", "min", "major", "major", "min", "dim"};
        return q[(degree - 1) % 7];
    }
    static const std::array<const char*, 7> q = {
        "min", "dim", "major", "min", "min", "major", "major"};
    return q[(degree - 1) % 7];
}

/** Semitone of a diatonic degree above the tonic. */
inline int degreeSemitone(int degree, bool minorKey) {
    const auto& iv = scaleIntervals(minorKey ? ScaleType::NaturalMinor : ScaleType::Major);
    return iv[(degree - 1) % 7];
}

// === Functional flow validation =============================================
// Common-practice rule of thumb: T -> S -> D -> T.

/** @return true if a -> b respects forward functional flow. */
inline bool isForwardMotion(Function a, Function b) {
    if (a == Function::Tonic) return true;            // T can go anywhere
    if (a == Function::Subdominant) return true;      // S->D best, S->T (plagal) ok
    return b == Function::Tonic || b == Function::Dominant;  // D resolves or prolongs
}

/** @return warning if the motion is a retrogression, else "". */
inline std::string motionWarning(Function a, Function b) {
    if (a == Function::Dominant && b == Function::Subdominant)
        return "Retrogression D->S (ok in pop/rock, unusual in tonal style)";
    return "";
}

// === Cadence templates ======================================================

enum class Cadence { Authentic, Plagal, Half, Deceptive, None };

inline const char* cadenceName(Cadence c) {
    switch (c) {
        case Cadence::Authentic: return "Authentic (V-I)";
        case Cadence::Plagal: return "Plagal (IV-I)";
        case Cadence::Half: return "Half (->V)";
        case Cadence::Deceptive: return "Deceptive (V-vi)";
        case Cadence::None: return "None";
    }
    return "?";
}

/** Classify the cadence formed by the last two roman-numeral degrees. */
inline Cadence classifyCadence(int penultDegree, int finalDegree) {
    if (penultDegree == 5 && finalDegree == 1) return Cadence::Authentic;
    if (penultDegree == 4 && finalDegree == 1) return Cadence::Plagal;
    if (penultDegree == 5 && finalDegree == 6) return Cadence::Deceptive;
    if (finalDegree == 5) return Cadence::Half;
    return Cadence::None;
}

// === Tensions / extensions ==================================================
// Which added tones are idiomatic on a chord of a given function. Keeps the LLM
// from proposing e.g. a maj7 on a dominant or an 11 that clashes with V's 3rd.

/** @return true if `tension` is idiomatic on a triad of the given function. */
inline bool tensionAllowed(Function f, const std::string& tension) {
    if (tension == "add9" || tension == "9") return true;
    if (tension == "6") return f != Function::Dominant;
    if (tension == "maj7") return f == Function::Tonic || f == Function::Subdominant;
    if (tension == "7") return f == Function::Dominant || f == Function::Subdominant;
    if (tension == "sus2" || tension == "sus4") return true;
    if (tension == "11") return f != Function::Dominant;
    if (tension == "13") return f == Function::Dominant || f == Function::Subdominant;
    return true;
}

inline std::string tensionWarning(Function f, const std::string& tension) {
    if (!tensionAllowed(f, tension))
        return "Tension '" + tension + "' is unusual on a " +
               std::string(functionName(f)) + " chord";
    return "";
}

// === Secondary dominants & borrowed chords ==================================

/** Root semitone (above tonic) of the secondary dominant V/x targeting degree. */
inline int secondaryDominantRoot(int targetDegree, bool minorKey) {
    return (degreeSemitone(targetDegree, minorKey) + 7) % 12;  // a 5th above target
}

/** Common modal-interchange chords in a major key: {semitone-above-tonic, quality}. */
inline const std::vector<std::pair<int, std::string>>& borrowedChordsMajor() {
    static const std::vector<std::pair<int, std::string>> v = {
        {3, "major"},   // bIII
        {5, "min"},     // iv
        {8, "major"},   // bVI
        {10, "major"},  // bVII
        {2, "dim"},     // ii°
    };
    return v;
}

// === Note-name helpers ======================================================

/** Pitch-class name (sharps) for a semitone 0..11. */
inline std::string pitchClassName(int semitone) {
    static const std::array<const char*, 12> names = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    return names[((semitone % 12) + 12) % 12];
}

/** Roman numeral for a degree, cased by triad quality (uppercase major/aug). */
inline std::string romanNumeral(int degree, bool minorKey) {
    static const std::array<const char*, 7> upper = {"I", "II", "III", "IV", "V", "VI", "VII"};
    static const std::array<const char*, 7> lower = {"i", "ii", "iii", "iv", "v", "vi", "vii"};
    auto q = diatonicTriadQuality(degree, minorKey);
    bool isMajorish = (q == "major" || q == "aug");
    std::string r = (isMajorish ? upper : lower)[(degree - 1) % 7];
    if (q == "dim") r += "\u00B0";
    return r;
}

// === Prompt text ============================================================
// A compact, authoritative theory brief injected into the music agent's system
// prompt so the model's proposals stay inside the rules above.

inline const char* constraintBrief() {
    return R"BRIEF(
MUSIC THEORY CONSTRAINTS (obey when proposing harmony):
FUNCTION: Group chords by function. Major key: I/vi/iii=Tonic, ii/IV=Subdominant, V/vii=Dominant.
FLOW: Prefer Tonic -> Subdominant -> Dominant -> Tonic. A Dominant (V, vii) should resolve to
  Tonic (I, or vi for a deceptive cadence). Avoid Dominant -> Subdominant unless style is pop/rock.
CADENCE: End phrases with a clear cadence: Authentic V-I (strong), Plagal IV-I, Half ...->V (open),
  or Deceptive V-vi (surprise).
INVERSIONS: Use inversions for smooth bass / voice leading (e.g. I-V6-vi keeps the bass stepwise).
  Prefer the smallest melodic distance between chord tones across changes.
TENSIONS: add9/9 are broadly safe. maj7 fits Tonic/Subdominant, not Dominant. A dominant 7th wants
  to resolve. Use sus2/sus4 to delay resolution. Avoid 11 on a Dominant (clashes with the 3rd).
BASS: Outline chord roots, optionally passing through 3rds/5ths; move mostly by step or by the
  chord change; avoid large leaps unless intentional.
MELODY: Stay within the key's scale; resolve the leading tone (7th degree) up to the tonic;
  approach and leave leaps by step where possible.
SECONDARY DOMINANTS: A V/x (dominant 7th a fifth above the target root) can tonicize any diatonic
  chord (e.g. V/V = D7 in C major).
BORROWED CHORDS: In a major key you may borrow from the parallel minor (iv, bVI, bVII, bIII) for color.
)BRIEF";
}

}  // namespace theory
}  // namespace magda
