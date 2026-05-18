#include "TrackManager.hpp"
#include "Engine.hpp"
#include <algorithm>

namespace aidaw {

#ifdef AIDAW_HAS_TRACKTION

TrackManager::TrackManager(Engine& engine)
    : engineRef(engine) {}

te::Edit* TrackManager::getEdit() const {
    return engineRef.getEdit();
}

te::AudioTrack* TrackManager::findTrackById(int trackId) const {
    auto* edit = getEdit();
    if (!edit) return nullptr;

    int index = 0;
    for (auto track : te::getAudioTracks(*edit)) {
        if (index == trackId)
            return track;
        index++;
    }
    return nullptr;
}

TrackInfo TrackManager::buildTrackInfo(te::AudioTrack* track) const {
    TrackInfo info;
    info.id = track->getIndexInEditTrackList();
    info.name = track->getName().toStdString();
    info.muted = track->isMuted(false);
    info.solo = track->isSolo(false);

    if (auto* vol = track->getVolumePlugin()) {
        info.volume = vol->getSliderPos();
        info.pan = vol->getPan();
    }

    return info;
}

int TrackManager::createTrack(const std::string& name, const std::string& /*instrument*/) {
    auto* edit = getEdit();
    if (!edit) return -1;

    auto track = edit->insertNewAudioTrack(
        te::TrackInsertPoint::getEndOfTracks(*edit), nullptr, true);

    if (!track) return -1;

    track->setName(juce::String(name));
    return track->getIndexInEditTrackList();
}

void TrackManager::deleteTrack(int trackId) {
    auto* track = findTrackById(trackId);
    if (!track) return;

    auto* edit = getEdit();
    if (edit)
        edit->deleteTrack(track);
}

TrackInfo* TrackManager::getTrack(int trackId) {
    // Note: returns nullptr because Tracktion owns the track data.
    // Use getAllTracks() or buildTrackInfo() for snapshots.
    (void)trackId;
    return nullptr;
}

std::vector<TrackInfo> TrackManager::getAllTracks() const {
    std::vector<TrackInfo> result;
    auto* edit = getEdit();
    if (!edit) return result;

    for (auto* track : te::getAudioTracks(*edit))
        result.push_back(buildTrackInfo(track));

    return result;
}

void TrackManager::setTrackVolume(int trackId, float volume) {
    if (auto* track = findTrackById(trackId)) {
        if (auto* vol = track->getVolumePlugin())
            vol->setSliderPos(volume);
    }
}

void TrackManager::setTrackPan(int trackId, float pan) {
    if (auto* track = findTrackById(trackId)) {
        if (auto* vol = track->getVolumePlugin())
            vol->setPan(pan);
    }
}

void TrackManager::setTrackMute(int trackId, bool muted) {
    if (auto* track = findTrackById(trackId))
        track->setMute(muted);
}

void TrackManager::setTrackSolo(int trackId, bool solo) {
    if (auto* track = findTrackById(trackId))
        track->setSolo(solo);
}

#else  // Fallback placeholder implementation

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

std::vector<TrackInfo> TrackManager::getAllTracks() const {
    return tracks;
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

#endif

}  // namespace aidaw
