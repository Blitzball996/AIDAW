#include "DeviceMeteringManager.hpp"

namespace aidaw {

#ifdef AIDAW_HAS_TRACKTION
std::map<te::Edit*, DeviceMeteringManager*> DeviceMeteringManager::editMap_;
juce::CriticalSection DeviceMeteringManager::editMapLock_;

te::LevelMeasurer& DeviceMeteringManager::getOrCreateMeasurer(DeviceId deviceId) {
    juce::ScopedLock sl(lock_);
    auto it = entries_.find(deviceId);
    if (it != entries_.end()) {
        if (!it->second->clientRegistered) {
            it->second->measurer.addClient(it->second->client);
            it->second->clientRegistered = true;
        }
        return it->second->measurer;
    }

    auto entry = std::make_unique<Entry>();
    entry->measurer.addClient(entry->client);
    entry->clientRegistered = true;
    auto& measurer = entry->measurer;
    entries_[deviceId] = std::move(entry);
    return measurer;
}

void DeviceMeteringManager::removeMeasurer(DeviceId deviceId) {
    juce::ScopedLock sl(lock_);
    auto it = entries_.find(deviceId);
    if (it != entries_.end()) {
        if (it->second->clientRegistered)
            it->second->measurer.removeClient(it->second->client);
        entries_.erase(it);
    }
}

DeviceMeteringManager* DeviceMeteringManager::getInstanceForEdit(te::Edit& edit) {
    juce::ScopedLock sl(editMapLock_);
    auto it = editMap_.find(&edit);
    return it != editMap_.end() ? it->second : nullptr;
}

void DeviceMeteringManager::registerForEdit(te::Edit& edit, DeviceMeteringManager* mgr) {
    juce::ScopedLock sl(editMapLock_);
    editMap_[&edit] = mgr;
}

void DeviceMeteringManager::unregisterForEdit(te::Edit& edit) {
    juce::ScopedLock sl(editMapLock_);
    editMap_.erase(&edit);
}
#endif  // AIDAW_HAS_TRACKTION

void DeviceMeteringManager::updateAllClients() {
    juce::ScopedLock sl(lock_);
    for (auto& [deviceId, entry] : entries_) {
#ifdef AIDAW_HAS_TRACKTION
        if (!entry->clientRegistered) {
            entry->measurer.addClient(entry->client);
            entry->clientRegistered = true;
        }

        auto levelL = entry->client.getAndClearAudioLevel(0);
        auto levelR = entry->client.getAndClearAudioLevel(1);

        float peakL = juce::Decibels::decibelsToGain(levelL.dB);
        float peakR = juce::Decibels::decibelsToGain(levelR.dB);
#else
        float peakL = 0.0f;
        float peakR = 0.0f;
#endif

        // Merge with realtime tap if present
        if (entry->realtimeTap) {
            float tapL = entry->realtimeTap->peakL.exchange(0.0f, std::memory_order_relaxed);
            float tapR = entry->realtimeTap->peakR.exchange(0.0f, std::memory_order_relaxed);
            peakL = std::max(peakL, tapL);
            peakR = std::max(peakR, tapR);
        }

        entry->peakL.store(peakL, std::memory_order_relaxed);
        entry->peakR.store(peakR, std::memory_order_relaxed);
    }
}

bool DeviceMeteringManager::getLatestLevels(DeviceId deviceId, DeviceMeterData& out) const {
    juce::ScopedLock sl(lock_);
    auto it = entries_.find(deviceId);
    if (it == entries_.end())
        return false;

    out.peakL = it->second->peakL.load(std::memory_order_relaxed);
    out.peakR = it->second->peakR.load(std::memory_order_relaxed);
    return true;
}

void DeviceMeteringManager::setGain(DeviceId deviceId, float gainLinear) {
    juce::ScopedLock sl(lock_);
    auto it = entries_.find(deviceId);
    if (it != entries_.end())
        it->second->gainLinear.store(gainLinear, std::memory_order_relaxed);
}

std::atomic<float>* DeviceMeteringManager::getGainAtomic(DeviceId deviceId) {
    juce::ScopedLock sl(lock_);
    auto it = entries_.find(deviceId);
    return it != entries_.end() ? &it->second->gainLinear : nullptr;
}

void DeviceMeteringManager::setDirectLevels(DeviceId deviceId, float peakL, float peakR) {
    juce::ScopedLock sl(lock_);
    auto it = entries_.find(deviceId);
    if (it != entries_.end()) {
        it->second->peakL.store(peakL, std::memory_order_relaxed);
        it->second->peakR.store(peakR, std::memory_order_relaxed);
    }
}

void DeviceMeteringManager::ensureEntry(DeviceId deviceId) {
    juce::ScopedLock sl(lock_);
    if (entries_.find(deviceId) == entries_.end())
        entries_[deviceId] = std::make_unique<Entry>();
}

DeviceMeteringManager::RealtimeTap DeviceMeteringManager::getRealtimeTap(DeviceId deviceId) {
    juce::ScopedLock sl(lock_);
    auto it = entries_.find(deviceId);
    if (it == entries_.end()) {
        entries_[deviceId] = std::make_unique<Entry>();
        it = entries_.find(deviceId);
    }

    if (!it->second->realtimeTap)
        it->second->realtimeTap = std::make_shared<RealtimeTapStorage>();

    RealtimeTap tap;
    tap.storage = it->second->realtimeTap;
    tap.peakL = &tap.storage->peakL;
    tap.peakR = &tap.storage->peakR;
    tap.gainLinear = &tap.storage->gainLinear;
    return tap;
}

void DeviceMeteringManager::setRackDirectLevels(RackId rackId, float peakL, float peakR) {
    juce::ScopedLock sl(lock_);
    auto it = rackEntries_.find(rackId);
    if (it != rackEntries_.end()) {
        it->second->peakL.store(peakL, std::memory_order_relaxed);
        it->second->peakR.store(peakR, std::memory_order_relaxed);
    }
}

void DeviceMeteringManager::ensureRackEntry(RackId rackId) {
    juce::ScopedLock sl(lock_);
    if (rackEntries_.find(rackId) == rackEntries_.end())
        rackEntries_[rackId] = std::make_unique<SimpleEntry>();
}

bool DeviceMeteringManager::getRackLatestLevels(RackId rackId, DeviceMeterData& out) const {
    juce::ScopedLock sl(lock_);
    auto it = rackEntries_.find(rackId);
    if (it == rackEntries_.end())
        return false;

    out.peakL = it->second->peakL.load(std::memory_order_relaxed);
    out.peakR = it->second->peakR.load(std::memory_order_relaxed);
    return true;
}

void DeviceMeteringManager::clear() {
    juce::ScopedLock sl(lock_);
#ifdef AIDAW_HAS_TRACKTION
    for (auto& [id, entry] : entries_) {
        if (entry->clientRegistered)
            entry->measurer.removeClient(entry->client);
    }
#endif
    entries_.clear();
    rackEntries_.clear();
}

}  // namespace aidaw
