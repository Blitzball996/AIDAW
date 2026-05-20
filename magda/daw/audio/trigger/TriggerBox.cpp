#include "TriggerBox.hpp"

#include <algorithm>
#include <random>

namespace magda {

TriggerBox::TriggerBox() = default;
TriggerBox::~TriggerBox() = default;

TriggerSlot TriggerBox::getSlot(TrackId trackId, int slotIndex) const {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto* ts = findTrackSlots(trackId);
    if (ts == nullptr || slotIndex < 0 ||
        slotIndex >= static_cast<int>(ts->slots.size()))
        return {};
    return ts->slots[static_cast<size_t>(slotIndex)];
}

void TriggerBox::setSlotClip(TrackId trackId, int slotIndex, ClipId clipId) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& ts = getOrCreateTrackSlots(trackId);
        if (slotIndex < 0 || slotIndex >= kMaxSlotsPerTrack) return;
        while (static_cast<int>(ts.slots.size()) <= slotIndex)
            ts.slots.emplace_back();
        ts.slots[static_cast<size_t>(slotIndex)].clipId = clipId;
    }
    notifyClipChanged(trackId, slotIndex, clipId);
}

void TriggerBox::clearSlot(TrackId trackId, int slotIndex) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto* ts = findTrackSlots(trackId) ? &trackSlots_[trackId] : nullptr;
        if (ts == nullptr || slotIndex < 0 ||
            slotIndex >= static_cast<int>(ts->slots.size()))
            return;
        ts->slots[static_cast<size_t>(slotIndex)] = TriggerSlot{};
    }
    notifyClipChanged(trackId, slotIndex, INVALID_CLIP_ID);
}

void TriggerBox::setSlotGain(TrackId trackId, int slotIndex, float gain) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto& ts = getOrCreateTrackSlots(trackId);
    if (slotIndex < 0 || slotIndex >= static_cast<int>(ts.slots.size())) return;
    ts.slots[static_cast<size_t>(slotIndex)].gain = juce::jlimit(0.0f, 2.0f, gain);
}

void TriggerBox::setSlotLooping(TrackId trackId, int slotIndex, bool looping) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto& ts = getOrCreateTrackSlots(trackId);
    if (slotIndex < 0 || slotIndex >= static_cast<int>(ts.slots.size())) return;
    ts.slots[static_cast<size_t>(slotIndex)].looping = looping;
}

void TriggerBox::setSlotFollowAction(TrackId trackId, int slotIndex,
                                     FollowAction action) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto& ts = getOrCreateTrackSlots(trackId);
    if (slotIndex < 0 || slotIndex >= static_cast<int>(ts.slots.size())) return;
    ts.slots[static_cast<size_t>(slotIndex)].followAction = action;
}

void TriggerBox::launchSlot(TrackId trackId, int slotIndex,
                            LaunchQuantize quantize) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& ts = getOrCreateTrackSlots(trackId);
        if (slotIndex < 0 || slotIndex >= static_cast<int>(ts.slots.size()))
            return;
        auto& slot = ts.slots[static_cast<size_t>(slotIndex)];
        if (slot.isEmpty()) return;

        // Stop currently active slot
        if (ts.activeSlotIndex >= 0 && ts.activeSlotIndex != slotIndex) {
            ts.slots[static_cast<size_t>(ts.activeSlotIndex)].state =
                TriggerState::Stopped;
        }

        if (quantize == LaunchQuantize::None) {
            slot.state = TriggerState::Playing;
            ts.activeSlotIndex = slotIndex;
        } else {
            slot.state = TriggerState::Queued;
            // Actual launch happens at quantize boundary via external timer
        }
    }

    auto state = getSlot(trackId, slotIndex).state;
    notifyStateChanged(trackId, slotIndex, state);
}

void TriggerBox::stopSlot(TrackId trackId, LaunchQuantize /*quantize*/) {
    int prevActive = -1;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = trackSlots_.find(trackId);
        if (it == trackSlots_.end()) return;
        auto& ts = it->second;
        if (ts.activeSlotIndex < 0) return;
        prevActive = ts.activeSlotIndex;
        ts.slots[static_cast<size_t>(ts.activeSlotIndex)].state =
            TriggerState::Stopped;
        ts.activeSlotIndex = -1;
    }
    if (prevActive >= 0)
        notifyStateChanged(trackId, prevActive, TriggerState::Stopped);
}

void TriggerBox::stopAll() {
    std::vector<std::pair<TrackId, int>> stoppedSlots;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [trackId, ts] : trackSlots_) {
            if (ts.activeSlotIndex >= 0) {
                ts.slots[static_cast<size_t>(ts.activeSlotIndex)].state =
                    TriggerState::Stopped;
                stoppedSlots.emplace_back(trackId, ts.activeSlotIndex);
                ts.activeSlotIndex = -1;
            }
        }
    }
    for (auto& [trackId, slotIdx] : stoppedSlots)
        notifyStateChanged(trackId, slotIdx, TriggerState::Stopped);
}

int TriggerBox::getActiveSlotIndex(TrackId trackId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto* ts = findTrackSlots(trackId);
    return ts ? ts->activeSlotIndex : -1;
}

int TriggerBox::getSlotCount(TrackId trackId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto* ts = findTrackSlots(trackId);
    return ts ? static_cast<int>(ts->slots.size()) : 0;
}

void TriggerBox::processFollowAction(TrackId trackId, int slotIndex) {
    FollowAction action;
    int slotCount = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto* ts = findTrackSlots(trackId) ? &trackSlots_[trackId] : nullptr;
        if (ts == nullptr || slotIndex < 0 ||
            slotIndex >= static_cast<int>(ts->slots.size()))
            return;
        action = ts->slots[static_cast<size_t>(slotIndex)].followAction;
        slotCount = static_cast<int>(ts->slots.size());
    }

    if (action == FollowAction::None) return;
    if (action == FollowAction::Stop) {
        stopSlot(trackId, LaunchQuantize::None);
        return;
    }

    int nextSlot = resolveFollowAction(action, slotIndex, slotCount);
    if (nextSlot >= 0 && nextSlot < slotCount)
        launchSlot(trackId, nextSlot, LaunchQuantize::None);
}

void TriggerBox::addListener(TriggerBoxListener* listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.push_back(listener);
}

void TriggerBox::removeListener(TriggerBoxListener* listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.erase(std::remove(listeners_.begin(), listeners_.end(), listener),
                     listeners_.end());
}

// --- Private ---

TriggerBox::TrackSlots& TriggerBox::getOrCreateTrackSlots(TrackId trackId) {
    auto it = trackSlots_.find(trackId);
    if (it == trackSlots_.end()) {
        auto& ts = trackSlots_[trackId];
        ts.slots.resize(kMaxSlotsPerTrack);
        return ts;
    }
    return it->second;
}

const TriggerBox::TrackSlots* TriggerBox::findTrackSlots(TrackId trackId) const {
    auto it = trackSlots_.find(trackId);
    return it != trackSlots_.end() ? &it->second : nullptr;
}

void TriggerBox::notifyStateChanged(TrackId trackId, int slotIndex,
                                    TriggerState state) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto* l : listeners_)
        l->triggerSlotStateChanged(trackId, slotIndex, state);
}

void TriggerBox::notifyClipChanged(TrackId trackId, int slotIndex, ClipId clipId) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto* l : listeners_)
        l->triggerSlotClipChanged(trackId, slotIndex, clipId);
}

int TriggerBox::resolveFollowAction(FollowAction action, int currentIndex,
                                    int slotCount) const {
    if (slotCount <= 0) return -1;

    switch (action) {
        case FollowAction::None:
        case FollowAction::Stop:
            return -1;
        case FollowAction::PlayNext:
            return (currentIndex + 1) % slotCount;
        case FollowAction::PlayPrevious:
            return (currentIndex - 1 + slotCount) % slotCount;
        case FollowAction::PlayAgain:
            return 0;
            return currentIndex; // PlayAgain = replay current
        case FollowAction::PlayRandom: {
            if (slotCount <= 1) return 0;
            static std::mt19937 rng(std::random_device{}());
            std::uniform_int_distribution<int> dist(0, slotCount - 1);
            int next = dist(rng);
            // Avoid playing the same slot
            while (next == currentIndex && slotCount > 1) next = dist(rng);
            return next;
        }
    }
    return -1;
}

}  // namespace magda
