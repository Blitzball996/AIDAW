#pragma once

#include <juce_core/juce_core.h>

#include "../core/TypeIds.hpp"

namespace aidaw {

/// Play state of a session clip, derived from the scheduler
enum class SessionClipPlayState { Stopped, Queued, Playing };

/**
 * Abstract view onto session-view playback — clip launching, stopping,
 * scene-launch helpers.
 *
 * Behaviour matches the UI's "trigger" buttons — calls flow through
 * the same clip-playback-requested pipeline. Launch quantization,
 * toggle vs trigger mode, and scene logic are applied identically
 * to manual UI interaction.
 */
class SessionApi {
  public:
    virtual ~SessionApi() = default;

    /// Trigger a session-view clip. No-op if clipId is invalid or the
    /// clip is in arrangement view.
    virtual void launchClip(ClipId clipId) = 0;

    /// Stop a session clip (no-op if not active).
    virtual void stopClip(ClipId clipId) = 0;

    /// Stop the active session clip on a track, if any.
    virtual void stopTrack(TrackId trackId) = 0;

    /// Stop every active session clip.
    virtual void stopAll() = 0;

    /// Launch a whole scene row: triggers every clip in that scene and
    /// stops the active clip on tracks whose slot is empty (matching the
    /// UI's scene-button behaviour).
    virtual void launchScene(int sceneIndex) = 0;

    /// Return the active session clip on the given track, or
    /// INVALID_CLIP_ID if nothing is currently launched there.
    virtual ClipId getActiveClipOnTrack(TrackId trackId) const = 0;

    /// Return the session clip at (track, sceneIndex), or INVALID_CLIP_ID
    /// if that slot is empty. sceneIndex is 0-based.
    virtual ClipId getClipInSlot(TrackId trackId, int sceneIndex) const = 0;

    /// Return the playback state of a session clip. Returns Stopped if
    /// the id is invalid, the clip isn't a session clip, or the engine
    /// is unavailable.
    virtual SessionClipPlayState getClipPlayState(ClipId clipId) const = 0;
};

}  // namespace aidaw
