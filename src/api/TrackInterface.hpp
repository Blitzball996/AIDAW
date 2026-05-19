#pragma once

#include <string>
#include <vector>

namespace aidaw {

/**
 * @brief Interface for managing DAW tracks
 *
 * The TrackInterface provides methods for creating, deleting, and configuring
 * audio and MIDI tracks within the DAW.
 */
class TrackInterface {
  public:
    virtual ~TrackInterface() = default;

    virtual std::string createAudioTrack(const std::string& name) = 0;
    virtual std::string createMidiTrack(const std::string& name) = 0;
    virtual void deleteTrack(const std::string& track_id) = 0;

    virtual void setTrackName(const std::string& track_id, const std::string& name) = 0;
    virtual std::string getTrackName(const std::string& track_id) const = 0;

    virtual void setTrackMuted(const std::string& track_id, bool muted) = 0;
    virtual bool isTrackMuted(const std::string& track_id) const = 0;

    virtual void setTrackSolo(const std::string& track_id, bool solo) = 0;
    virtual bool isTrackSolo(const std::string& track_id) const = 0;

    virtual void setTrackArmed(const std::string& track_id, bool armed) = 0;
    virtual bool isTrackArmed(const std::string& track_id) const = 0;

    virtual void setTrackColor(const std::string& track_id, int r, int g, int b) = 0;

    virtual std::vector<std::string> getAllTrackIds() const = 0;
    virtual bool trackExists(const std::string& track_id) const = 0;
};

}  // namespace aidaw
