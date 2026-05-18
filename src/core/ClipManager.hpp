#pragma once

#include <string>
#include <vector>

namespace aidaw {

struct ClipInfo {
    int id;
    int trackId;
    std::string name;
    double startBeat;
    double lengthBeats;
};

class ClipManager {
public:
    int createClip(int trackId, const std::string& name, double startBeat, double lengthBeats);
    void deleteClip(int clipId);
    ClipInfo* getClip(int clipId);
    std::vector<ClipInfo> getClipsForTrack(int trackId) const;

private:
    std::vector<ClipInfo> clips;
    int nextId = 1;
};

}  // namespace aidaw
