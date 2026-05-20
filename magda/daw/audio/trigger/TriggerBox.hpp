#pragma once

#include <juce_core/juce_core.h>

#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "../../core/ClipTypes.hpp"
#include "../../core/TypeIds.hpp"
#include "core/ClipTypes.hpp"

namespace magda {

/**
 * @brief State of a trigger slot.
 */
enum class TriggerState {
    Stopped,    ///< Slot is idle
    Playing,    ///< Slot is actively playing
    Queued,     ///< Slot is waiting for quantize boundary to start
    Recording   ///< Slot is recording new content
};

/**
 * @brief A single trigger slot holding a clip reference and playback settings.
 */
struct TriggerSlot {
    ClipId clipId = INVALID_CLIP_ID;
    TriggerState state = TriggerState::Stopped;
    float gain = 1.0f;
    bool looping = true;
    FollowAction followAction = FollowAction::None;

    bool isEmpty() const { return clipId == INVALID_CLIP_ID; }
};

/**
 * @brief Listener interface for TriggerBox state changes.
 */
class TriggerBoxListener {
  public:
    virtual ~TriggerBoxListener() = default;

    /** Called when a slot's state changes (play/stop/queue). */
    virtual void triggerSlotStateChanged(TrackId trackId, int slotIndex,
                                         TriggerState newState) = 0;

    /** Called when a slot's clip assignment changes. */
    virtual void triggerSlotClipChanged(TrackId trackId, int slotIndex,
                                        ClipId newClipId) = 0;
};

/**
 * @brief Per-track trigger slot system for session/clip-launch workflows.
 *
 * Manages a grid of trigger slots per track, handling quantized launch/stop
 * and follow actions. Similar to Ableton's clip slots or Ardour's triggerbox.
 *
 * All public methods are thread-safe and intended for message-thread use.
 */
class TriggerBox {
  public:
    static constexpr int kMaxSlotsPerTrack = 16;

    TriggerBox();
    ~TriggerBox();

    // --- Slot management ---

    /** Get the trigger slot for a track at a given index. */
    TriggerSlot getSlot(TrackId trackId, int slotIndex) const;

    /** Assign a clip to a slot. */
    void setSlotClip(TrackId trackId, int slotIndex, ClipId clipId);

    /** Clear a slot (remove clip assignment). */
    void clearSlot(TrackId trackId, int slotIndex);

    /** Set the gain for a slot. */
    void setSlotGain(TrackId trackId, int slotIndex, float gain);

    /** Set looping state for a slot. */
    void setSlotLooping(TrackId trackId, int slotIndex, bool looping);

    /** Set the follow action for a slot. */
    void setSlotFollowAction(TrackId trackId, int slotIndex, FollowAction action);

    // --- Playback control ---

    /** Launch a slot with optional quantization.
     *  @param trackId   The track containing the slot.
     *  @param slotIndex The slot index (0-based).
     *  @param quantize  Launch quantization setting.
     */
    void launchSlot(TrackId trackId, int slotIndex, LaunchQuantize quantize);

    /** Stop the currently playing slot on a track. */
    void stopSlot(TrackId trackId, LaunchQuantize quantize);

    /** Stop all playing slots across all tracks. */
    void stopAll();

    // --- Query ---

    /** Get the currently playing slot index for a track, or -1 if none. */
    int getActiveSlotIndex(TrackId trackId) const;

    /** Get the number of configured slots for a track. */
    int getSlotCount(TrackId trackId) const;

    // --- Follow action processing ---

    /** Called when a clip finishes playing to evaluate follow actions.
     *  Typically invoked from the audio callback or a timer. */
    void processFollowAction(TrackId trackId, int slotIndex);

    // --- Listeners ---

    void addListener(TriggerBoxListener* listener);
    void removeListener(TriggerBoxListener* listener);

  private:
    struct TrackSlots {
        std::vector<TriggerSlot> slots;
        int activeSlotIndex = -1;
    };

    TrackSlots& getOrCreateTrackSlots(TrackId trackId);
    const TrackSlots* findTrackSlots(TrackId trackId) const;
    void notifyStateChanged(TrackId trackId, int slotIndex, TriggerState state);
    void notifyClipChanged(TrackId trackId, int slotIndex, ClipId clipId);
    int resolveFollowAction(FollowAction action, int currentIndex, int slotCount) const;

    mutable std::mutex mutex_;
    std::unordered_map<TrackId, TrackSlots> trackSlots_;
    std::vector<TriggerBoxListener*> listeners_;
};

}  // namespace magda
