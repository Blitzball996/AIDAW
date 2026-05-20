#pragma once

#include <juce_core/juce_core.h>

#include <mutex>
#include <unordered_map>
#include <vector>

#include "../../core/TypeIds.hpp"

namespace magda {

/**
 * @brief Unique identifier for VCA faders.
 */
using VCAId = int;
constexpr VCAId INVALID_VCA_ID = -1;

/**
 * @brief A VCA (Voltage Controlled Amplifier) fader that controls
 *        the level of multiple assigned tracks proportionally.
 */
struct VCAFader {
    VCAId id = INVALID_VCA_ID;
    juce::String name;
    float level = 1.0f;          ///< 0.0 to 1.0 (unity = 1.0)
    bool mute = false;
    bool solo = false;
    std::vector<TrackId> assignedTracks;

    bool isAssigned(TrackId trackId) const {
        return std::find(assignedTracks.begin(), assignedTracks.end(), trackId) !=
               assignedTracks.end();
    }
};

/**
 * @brief Listener interface for VCA fader changes.
 */
class VCAManagerListener {
  public:
    virtual ~VCAManagerListener() = default;

    /** Called when a VCA fader's level changes. */
    virtual void vcaLevelChanged(VCAId vcaId, float newLevel) = 0;

    /** Called when a VCA fader's mute/solo state changes. */
    virtual void vcaMuteOrSoloChanged(VCAId vcaId) = 0;

    /** Called when tracks are assigned/unassigned from a VCA. */
    virtual void vcaAssignmentChanged(VCAId vcaId) = 0;

    /** Called when a VCA fader is created or deleted. */
    virtual void vcaListChanged() = 0;
};

/**
 * @brief Manages VCA (Voltage Controlled Amplifier) faders.
 *
 * VCA faders provide proportional level control over groups of tracks.
 * When a VCA level changes, all assigned tracks scale their output
 * proportionally (multiplicative, not additive).
 *
 * Thread-safe for message-thread access.
 */
class VCAManager {
  public:
    VCAManager();
    ~VCAManager();

    // --- VCA lifecycle ---

    /** Create a new VCA fader with the given name. Returns its ID. */
    VCAId createVCA(const juce::String& name);

    /** Delete a VCA fader. Tracks are unassigned automatically. */
    void deleteVCA(VCAId vcaId);

    /** Get all VCA faders. */
    std::vector<VCAFader> getAllVCAs() const;

    /** Get a specific VCA fader by ID. */
    VCAFader getVCA(VCAId vcaId) const;

    /** Rename a VCA fader. */
    void renameVCA(VCAId vcaId, const juce::String& newName);

    // --- Level control ---

    /** Set the VCA fader level (0.0 to 1.0). Notifies listeners. */
    void setLevel(VCAId vcaId, float level);

    /** Get the current VCA level. */
    float getLevel(VCAId vcaId) const;

    /** Set mute state. */
    void setMute(VCAId vcaId, bool mute);

    /** Set solo state. */
    void setSolo(VCAId vcaId, bool solo);

    // --- Track assignment ---

    /** Assign a track to a VCA fader. */
    void assignTrack(VCAId vcaId, TrackId trackId);

    /** Unassign a track from a VCA fader. */
    void unassignTrack(VCAId vcaId, TrackId trackId);

    /** Unassign a track from all VCA faders. */
    void unassignTrackFromAll(TrackId trackId);

    /** Get all VCA faders that a track is assigned to. */
    std::vector<VCAId> getVCAsForTrack(TrackId trackId) const;

    /** Get the effective gain multiplier for a track from all assigned VCAs.
     *  This is the product of all assigned VCA levels. */
    float getEffectiveGain(TrackId trackId) const;

    /** Check if a track is effectively muted by any assigned VCA. */
    bool isEffectivelyMuted(TrackId trackId) const;

    // --- Listeners ---

    void addListener(VCAManagerListener* listener);
    void removeListener(VCAManagerListener* listener);

  private:
    VCAFader* findVCA(VCAId vcaId);
    const VCAFader* findVCA(VCAId vcaId) const;
    void notifyLevelChanged(VCAId vcaId, float level);
    void notifyMuteOrSoloChanged(VCAId vcaId);
    void notifyAssignmentChanged(VCAId vcaId);
    void notifyListChanged();

    mutable std::mutex mutex_;
    std::vector<VCAFader> vcas_;
    std::vector<VCAManagerListener*> listeners_;
    VCAId nextId_ = 1;
};

}  // namespace magda
