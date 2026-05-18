#pragma once

#include <string>
#include <vector>

#ifdef AIDAW_HAS_TRACKTION
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion;
#endif

namespace aidaw {

class Engine;

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
#ifdef AIDAW_HAS_TRACKTION
    explicit TrackManager(Engine& engine);
#else
    TrackManager() = default;
#endif

    int createTrack(const std::string& name, const std::string& instrument = "");
    void deleteTrack(int trackId);
    TrackInfo* getTrack(int trackId);
    std::vector<TrackInfo> getAllTracks() const;

    void setTrackVolume(int trackId, float volume);
    void setTrackPan(int trackId, float pan);
    void setTrackMute(int trackId, bool muted);
    void setTrackSolo(int trackId, bool solo);

private:
#ifdef AIDAW_HAS_TRACKTION
    Engine& engineRef;

    te::Edit* getEdit() const;
    te::AudioTrack* findTrackById(int trackId) const;
    TrackInfo buildTrackInfo(te::AudioTrack* track) const;
#else
    std::vector<TrackInfo> tracks;
    int nextId = 1;
#endif
};

}  // namespace aidaw
