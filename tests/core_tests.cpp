#include "core/TrackManager.hpp"
#include "core/ClipManager.hpp"
#include "core/TransportControl.hpp"
#include <cassert>
#include <iostream>

using namespace aidaw;

#ifndef AIDAW_HAS_TRACKTION

void testTrackManager() {
    TrackManager tm;
    int id = tm.createTrack("Piano", "Piano");
    assert(id == 1);
    assert(tm.getAllTracks().size() == 1);
    assert(tm.getTrack(id)->name == "Piano");

    tm.setTrackVolume(id, 0.5f);
    assert(tm.getTrack(id)->volume == 0.5f);

    tm.deleteTrack(id);
    assert(tm.getAllTracks().empty());

    std::cout << "testTrackManager passed\n";
}

void testClipManager() {
    ClipManager cm;
    int id = cm.createClip(1, "Clip1", 0.0, 4.0);
    assert(id == 1);
    assert(cm.getClip(id)->lengthBeats == 4.0);

    cm.deleteClip(id);
    assert(cm.getClip(id) == nullptr);

    std::cout << "testClipManager passed\n";
}

void testTransport() {
    TransportControl tc;
    assert(!tc.isPlaying());
    tc.play();
    assert(tc.isPlaying());
    tc.setTempo(140.0);
    assert(tc.getTempo() == 140.0);
    tc.stop();
    assert(!tc.isPlaying());

    std::cout << "testTransport passed\n";
}

void testTransportPlayStopTransitions() {
    TransportControl tc;

    // Initial state: stopped
    assert(!tc.isPlaying());
    assert(!tc.isRecording());

    // Play
    tc.play();
    assert(tc.isPlaying());
    assert(!tc.isRecording());

    // Stop from playing
    tc.stop();
    assert(!tc.isPlaying());
    assert(!tc.isRecording());

    std::cout << "testTransportPlayStopTransitions passed\n";
}

void testTransportRecordState() {
    TransportControl tc;

    // Record should set both playing and recording
    tc.record();
    assert(tc.isPlaying());
    assert(tc.isRecording());

    // Stop should clear both
    tc.stop();
    assert(!tc.isPlaying());
    assert(!tc.isRecording());

    std::cout << "testTransportRecordState passed\n";
}

void testTransportRecordThenPlay() {
    TransportControl tc;

    // Start recording
    tc.record();
    assert(tc.isPlaying());
    assert(tc.isRecording());

    // Switching to play should clear recording
    tc.play();
    assert(tc.isPlaying());
    assert(!tc.isRecording());

    std::cout << "testTransportRecordThenPlay passed\n";
}

void testTransportTempoBoundsValidation() {
    TransportControl tc;

    // Default tempo
    assert(tc.getTempo() == 120.0);

    // Valid tempo changes
    tc.setTempo(60.0);
    assert(tc.getTempo() == 60.0);

    tc.setTempo(200.0);
    assert(tc.getTempo() == 200.0);

    tc.setTempo(1.0);
    assert(tc.getTempo() == 1.0);

    tc.setTempo(998.0);
    assert(tc.getTempo() == 998.0);

    std::cout << "testTransportTempoBoundsValidation passed\n";
}

void testTransportTempoRejectsInvalid() {
    TransportControl tc;

    // Set a known good value first
    tc.setTempo(120.0);

    // Zero should be rejected (bpm > 0.0 check)
    tc.setTempo(0.0);
    assert(tc.getTempo() == 120.0);

    // Negative should be rejected
    tc.setTempo(-10.0);
    assert(tc.getTempo() == 120.0);

    // 999 or above should be rejected (bpm < 999.0 check)
    tc.setTempo(999.0);
    assert(tc.getTempo() == 120.0);

    tc.setTempo(1500.0);
    assert(tc.getTempo() == 120.0);

    std::cout << "testTransportTempoRejectsInvalid passed\n";
}

void testTransportPosition() {
    TransportControl tc;

    assert(tc.getPosition() == 0.0);

    tc.setPosition(5.0);
    assert(tc.getPosition() == 5.0);

    tc.setPosition(0.0);
    assert(tc.getPosition() == 0.0);

    // Negative position should be rejected
    tc.setPosition(10.0);
    tc.setPosition(-1.0);
    assert(tc.getPosition() == 10.0);

    std::cout << "testTransportPosition passed\n";
}

void testTransportMultiplePlayCalls() {
    TransportControl tc;

    // Multiple play calls should be idempotent
    tc.play();
    tc.play();
    tc.play();
    assert(tc.isPlaying());

    tc.stop();
    assert(!tc.isPlaying());

    std::cout << "testTransportMultiplePlayCalls passed\n";
}

void testTransportMultipleStopCalls() {
    TransportControl tc;

    // Multiple stop calls should be safe
    tc.stop();
    tc.stop();
    assert(!tc.isPlaying());

    std::cout << "testTransportMultipleStopCalls passed\n";
}

int main() {
    testTrackManager();
    testClipManager();
    testTransport();
    testTransportPlayStopTransitions();
    testTransportRecordState();
    testTransportRecordThenPlay();
    testTransportTempoBoundsValidation();
    testTransportTempoRejectsInvalid();
    testTransportPosition();
    testTransportMultiplePlayCalls();
    testTransportMultipleStopCalls();
    std::cout << "All core tests passed!\n";
    return 0;
}

#else

int main() {
    std::cout << "Core tests skipped (AIDAW_HAS_TRACKTION defined)\n";
    return 0;
}

#endif
