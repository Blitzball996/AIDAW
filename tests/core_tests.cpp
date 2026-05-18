#include "core/TrackManager.hpp"
#include "core/ClipManager.hpp"
#include "core/TransportControl.hpp"
#include <cassert>
#include <iostream>

using namespace aidaw;

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

    std::cout << "TrackManager tests passed\n";
}

void testClipManager() {
    ClipManager cm;
    int id = cm.createClip(1, "Clip1", 0.0, 4.0);
    assert(id == 1);
    assert(cm.getClip(id)->lengthBeats == 4.0);

    cm.deleteClip(id);
    assert(cm.getClip(id) == nullptr);

    std::cout << "ClipManager tests passed\n";
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

    std::cout << "TransportControl tests passed\n";
}

int main() {
    testTrackManager();
    testClipManager();
    testTransport();
    std::cout << "All core tests passed!\n";
    return 0;
}
