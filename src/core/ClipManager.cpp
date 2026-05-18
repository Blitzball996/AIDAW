#include "ClipManager.hpp"
#include <algorithm>

namespace aidaw {

int ClipManager::createClip(int trackId, const std::string& name,
                             double startBeat, double lengthBeats) {
    ClipInfo clip;
    clip.id = nextId++;
    clip.trackId = trackId;
    clip.name = name;
    clip.startBeat = startBeat;
    clip.lengthBeats = lengthBeats;
    clips.push_back(clip);
    return clip.id;
}

void ClipManager::deleteClip(int clipId) {
    clips.erase(
        std::remove_if(clips.begin(), clips.end(),
            [clipId](const ClipInfo& c) { return c.id == clipId; }),
        clips.end());
}

ClipInfo* ClipManager::getClip(int clipId) {
    for (auto& c : clips) {
        if (c.id == clipId) return &c;
    }
    return nullptr;
}

std::vector<ClipInfo> ClipManager::getClipsForTrack(int trackId) const {
    std::vector<ClipInfo> result;
    for (const auto& c : clips) {
        if (c.trackId == trackId) result.push_back(c);
    }
    return result;
}

}  // namespace aidaw
