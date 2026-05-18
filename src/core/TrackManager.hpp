#pragma once

#include <string>
#include <vector>

namespace aidaw {

struct TrackInfo {
    int id;
    std::string name;
    std::string instrument;
    float volume = 1.0f;
    float pan = 0.0f;
    bool muted = false;
    bool solo = false;
};

class TrackManager {
public:
    int createTrack(const std::string& name, const std::string& instrument = "");
    void deleteTrack(int trackId);
    TrackInfo* getTrack(int trackId);
    const std::vector<TrackInfo>& getAllTracks() const { return tracks; }

    void setTrackVolume(int trackId, float volume);
    void setTrackPan(int trackId, float pan);
    void setTrackMute(int trackId, bool muted);
    void setTrackSolo(int trackId, bool solo);

private:
    std::vector<TrackInfo> tracks;
    int nextId = 1;
};

}  // namespace aidaw
