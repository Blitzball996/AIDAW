#include "VCAManager.hpp"

#include <algorithm>

namespace magda {

VCAManager::VCAManager() = default;
VCAManager::~VCAManager() = default;

VCAId VCAManager::createVCA(const juce::String& name) {
    VCAId id;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        id = nextId_++;
        VCAFader fader;
        fader.id = id;
        fader.name = name;
        vcas_.push_back(std::move(fader));
    }
    notifyListChanged();
    return id;
}

void VCAManager::deleteVCA(VCAId vcaId) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        vcas_.erase(std::remove_if(vcas_.begin(), vcas_.end(),
                                   [vcaId](const VCAFader& v) {
                                       return v.id == vcaId;
                                   }),
                    vcas_.end());
    }
    notifyListChanged();
}

std::vector<VCAFader> VCAManager::getAllVCAs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return vcas_;
}

VCAFader VCAManager::getVCA(VCAId vcaId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto* vca = findVCA(vcaId);
    return vca ? *vca : VCAFader{};
}

void VCAManager::renameVCA(VCAId vcaId, const juce::String& newName) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (auto* vca = findVCA(vcaId))
        vca->name = newName;
}

void VCAManager::setLevel(VCAId vcaId, float level) {
    float clamped = juce::jlimit(0.0f, 1.0f, level);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (auto* vca = findVCA(vcaId))
            vca->level = clamped;
        else
            return;
    }
    notifyLevelChanged(vcaId, clamped);
}

float VCAManager::getLevel(VCAId vcaId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto* vca = findVCA(vcaId);
    return vca ? vca->level : 1.0f;
}

void VCAManager::setMute(VCAId vcaId, bool mute) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (auto* vca = findVCA(vcaId))
            vca->mute = mute;
        else
            return;
    }
    notifyMuteOrSoloChanged(vcaId);
}

void VCAManager::setSolo(VCAId vcaId, bool solo) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (auto* vca = findVCA(vcaId))
            vca->solo = solo;
        else
            return;
    }
    notifyMuteOrSoloChanged(vcaId);
}

void VCAManager::assignTrack(VCAId vcaId, TrackId trackId) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto* vca = findVCA(vcaId);
        if (!vca) return;
        if (!vca->isAssigned(trackId))
            vca->assignedTracks.push_back(trackId);
        else
            return;
    }
    notifyAssignmentChanged(vcaId);
}

void VCAManager::unassignTrack(VCAId vcaId, TrackId trackId) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto* vca = findVCA(vcaId);
        if (!vca) return;
        auto& tracks = vca->assignedTracks;
        tracks.erase(std::remove(tracks.begin(), tracks.end(), trackId),
                     tracks.end());
    }
    notifyAssignmentChanged(vcaId);
}

void VCAManager::unassignTrackFromAll(TrackId trackId) {
    std::vector<VCAId> affected;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& vca : vcas_) {
            auto& tracks = vca.assignedTracks;
            auto it = std::find(tracks.begin(), tracks.end(), trackId);
            if (it != tracks.end()) {
                tracks.erase(it);
                affected.push_back(vca.id);
            }
        }
    }
    for (auto id : affected)
        notifyAssignmentChanged(id);
}

std::vector<VCAId> VCAManager::getVCAsForTrack(TrackId trackId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<VCAId> result;
    for (const auto& vca : vcas_)
        if (vca.isAssigned(trackId)) result.push_back(vca.id);
    return result;
}

float VCAManager::getEffectiveGain(TrackId trackId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    float gain = 1.0f;
    for (const auto& vca : vcas_) {
        if (vca.isAssigned(trackId) && !vca.mute)
            gain *= vca.level;
        else if (vca.isAssigned(trackId) && vca.mute)
            return 0.0f;
    }
    return gain;
}

bool VCAManager::isEffectivelyMuted(TrackId trackId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& vca : vcas_)
        if (vca.isAssigned(trackId) && vca.mute) return true;
    return false;
}

void VCAManager::addListener(VCAManagerListener* listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.push_back(listener);
}

void VCAManager::removeListener(VCAManagerListener* listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.erase(
        std::remove(listeners_.begin(), listeners_.end(), listener),
        listeners_.end());
}

// --- Private ---

VCAFader* VCAManager::findVCA(VCAId vcaId) {
    for (auto& vca : vcas_)
        if (vca.id == vcaId) return &vca;
    return nullptr;
}

const VCAFader* VCAManager::findVCA(VCAId vcaId) const {
    for (const auto& vca : vcas_)
        if (vca.id == vcaId) return &vca;
    return nullptr;
}

void VCAManager::notifyLevelChanged(VCAId vcaId, float level) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto* l : listeners_)
        l->vcaLevelChanged(vcaId, level);
}

void VCAManager::notifyMuteOrSoloChanged(VCAId vcaId) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto* l : listeners_)
        l->vcaMuteOrSoloChanged(vcaId);
}

void VCAManager::notifyAssignmentChanged(VCAId vcaId) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto* l : listeners_)
        l->vcaAssignmentChanged(vcaId);
}

void VCAManager::notifyListChanged() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto* l : listeners_)
        l->vcaListChanged();
}

}  // namespace magda
