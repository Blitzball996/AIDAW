#include "TrackManager.hpp"
#include <algorithm>

namespace aidaw {

int TrackManager::createTrack(const std::string& name, const std::string& instrument) {
    TrackInfo track;
    track.id = nextId++;
    track.name = name;
    track.instrument = instrument;
    tracks.push_back(track);
    return track.id;
}

void TrackManager::deleteTrack(int trackId) {
    tracks.erase(
        std::remove_if(tracks.begin(), tracks.end(),
            [trackId](const TrackInfo& t) { return t.id == trackId; }),
        tracks.end());
}

TrackInfo* TrackManager::getTrack(int trackId) {
    for (auto& t : tracks) {
        if (t.id == trackId) return &t;
    }
    return nullptr;
}

void TrackManager::setTrackVolume(int trackId, float volume) {
    if (auto* t = getTrack(trackId)) t->volume = volume;
}

void TrackManager::setTrackPan(int trackId, float pan) {
    if (auto* t = getTrack(trackId)) t->pan = pan;
}

void TrackManager::setTrackMute(int trackId, bool muted) {
    if (auto* t = getTrack(trackId)) t->muted = muted;
}

void TrackManager::setTrackSolo(int trackId, bool solo) {
    if (auto* t = getTrack(trackId)) t->solo = solo;
}

}  // namespace aidaw
