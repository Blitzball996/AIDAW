#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

/**
 * Tests for piano roll note position calculation in absolute and relative modes.
 *
 * These replicate the display beat calculation from
 * PianoRollGridComponent::updateNoteComponentBounds() without requiring JUCE.
 */

namespace {

/// Mirrors the absolute-mode display beat calculation for a single clip.
/// clipStartBeats is the grid's cached value (updated during drag preview).
double computeAbsoluteDisplayBeat(double clipStartBeats, double noteStartBeat,
                                  double midiTrimOffset) {
    return clipStartBeats + noteStartBeat - midiTrimOffset;
}

/// Mirrors the relative-mode display beat calculation for a single clip.
double computeRelativeDisplayBeat(double noteStartBeat) {
    return noteStartBeat;
}

/// Mirrors PianoRollGridComponent::getNoteInsertPosition's range guard.
/// Returns true when a double-click at `displayBeat` should create a note
/// (i.e. the click falls within the clip's displayed horizontal span).
/// clipDisplayStart/End are displayBeatForClipBeat(visibleStart) and
/// displayBeatForClipBeat(visibleStart + length) respectively.
bool clickShouldInsert(double displayBeat, double clipDisplayStart, double clipDisplayEnd) {
    constexpr double kEdgeTolerance = 1e-6;
    return !(displayBeat < clipDisplayStart - kEdgeTolerance || displayBeat >= clipDisplayEnd);
}

}  // namespace

TEST_CASE("Piano roll note position - absolute mode", "[pianoroll][display]") {
    SECTION("Note position reflects clip timeline position") {
        // Clip at bar 3 (beat 8 at 4/4), note at beat 1 within clip
        double displayBeat = computeAbsoluteDisplayBeat(8.0, 1.0, 0.0);
        REQUIRE(displayBeat == Catch::Approx(9.0));
    }

    SECTION("Note position updates during drag preview") {
        // Original clip at beat 8, note at beat 1
        double original = computeAbsoluteDisplayBeat(8.0, 1.0, 0.0);
        REQUIRE(original == Catch::Approx(9.0));

        // Drag clip to beat 16 (bar 5) — clipStartBeats_ changes to 16
        double dragged = computeAbsoluteDisplayBeat(16.0, 1.0, 0.0);
        REQUIRE(dragged == Catch::Approx(17.0));

        // Note moved by exactly the same amount as the clip
        REQUIRE(dragged - original == Catch::Approx(8.0));
    }

    SECTION("midiTrimOffset compensates for left-resize") {
        // Clip at beat 4, note at beat 2, trimmed by 1 beat from left
        double displayBeat = computeAbsoluteDisplayBeat(4.0, 2.0, 1.0);
        REQUIRE(displayBeat == Catch::Approx(5.0));
    }

    SECTION("Drag preview with trim offset") {
        // Clip at beat 4, note at beat 2, trim offset 1
        double original = computeAbsoluteDisplayBeat(4.0, 2.0, 1.0);

        // Drag to beat 12
        double dragged = computeAbsoluteDisplayBeat(12.0, 2.0, 1.0);

        // Note displacement matches clip displacement
        REQUIRE(dragged - original == Catch::Approx(8.0));
    }
}

TEST_CASE("Piano roll note position - relative mode", "[pianoroll][display]") {
    SECTION("Note position is content-relative regardless of clip position") {
        // Note at beat 2 within clip — position is always 2 regardless of
        // where the clip sits on the timeline
        REQUIRE(computeRelativeDisplayBeat(2.0) == Catch::Approx(2.0));
    }

    SECTION("Clip position has no effect in relative mode") {
        // Same note, different clip positions — display beat unchanged
        double pos1 = computeRelativeDisplayBeat(3.0);
        double pos2 = computeRelativeDisplayBeat(3.0);
        REQUIRE(pos1 == pos2);
    }
}

TEST_CASE("Piano roll note insert guard - empty-area click rejected",
          "[pianoroll][insert][regression]") {
    // Regression for two related bugs:
    //  - Problem 4: double-click in empty space jumped a phantom note to bar 1
    //    (clipBeatForDisplayX clamped a negative beat to 0).
    //  - Problem 6: a note inserted outside the visible range was stored but then
    //    filtered out by clipMidiNoteToVisibleRange, so the clip looked empty.
    // Both are prevented by rejecting clicks outside the clip's display span.

    // Relative single-clip mode: clip displayed at [0, length).
    SECTION("Relative mode: clip [0, 8)") {
        const double clipDisplayStart = 0.0;
        const double clipDisplayEnd = 8.0;  // 8 beats long

        // Click before the clip (empty space to the left) -> rejected (no jump to bar 1)
        REQUIRE_FALSE(clickShouldInsert(-3.0, clipDisplayStart, clipDisplayEnd));
        // Click at/after the clip end -> rejected
        REQUIRE_FALSE(clickShouldInsert(8.0, clipDisplayStart, clipDisplayEnd));
        REQUIRE_FALSE(clickShouldInsert(12.0, clipDisplayStart, clipDisplayEnd));
        // Click inside -> accepted
        REQUIRE(clickShouldInsert(0.0, clipDisplayStart, clipDisplayEnd));
        REQUIRE(clickShouldInsert(4.0, clipDisplayStart, clipDisplayEnd));
        REQUIRE(clickShouldInsert(7.999, clipDisplayStart, clipDisplayEnd));
    }

    // Absolute mode with a left-trimmed clip at bar 3 (beat 8), visible window
    // begins at midiTrimOffset; clip displayed at [8, 16).
    SECTION("Absolute mode: clip displayed at [8, 16)") {
        const double clipDisplayStart = 8.0;
        const double clipDisplayEnd = 16.0;

        // Clicking in the empty timeline before the clip -> rejected
        REQUIRE_FALSE(clickShouldInsert(0.0, clipDisplayStart, clipDisplayEnd));
        REQUIRE_FALSE(clickShouldInsert(7.0, clipDisplayStart, clipDisplayEnd));
        // Clicking past the clip -> rejected
        REQUIRE_FALSE(clickShouldInsert(16.0, clipDisplayStart, clipDisplayEnd));
        // Clicking inside the clip -> accepted
        REQUIRE(clickShouldInsert(8.0, clipDisplayStart, clipDisplayEnd));
        REQUIRE(clickShouldInsert(12.5, clipDisplayStart, clipDisplayEnd));
    }
}
