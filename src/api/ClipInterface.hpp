#pragma once

#include <string>
#include <vector>

namespace aidaw {

struct MidiNote;

/**
 * @brief Interface for managing clips (MIDI and audio segments)
 *
 * The ClipInterface provides methods for creating, editing, and manipulating
 * clips within tracks. Clips are timed segments that contain MIDI data or
 * reference audio files.
 */
class ClipInterface {
  public:
    virtual ~ClipInterface() = default;

    /**
     * @brief Add a MIDI clip to a track
     * @param track_id The target track ID
     * @param start_time Start time in seconds
     * @param length Length in seconds
     * @param notes Vector of MIDI notes
     * @return Clip ID
     */
    virtual std::string addMidiClip(const std::string& track_id, double start_time, double length,
                                    const std::vector<MidiNote>& notes) = 0;

    /**
     * @brief Add an audio clip to a track
     * @param track_id The target track ID
     * @param start_time Start time in seconds
     * @param audio_file_path Path to the audio file
     * @return Clip ID
     */
    virtual std::string addAudioClip(const std::string& track_id, double start_time,
                                     const std::string& audio_file_path) = 0;

    virtual void deleteClip(const std::string& clip_id) = 0;
    virtual void moveClip(const std::string& clip_id, double new_start_time) = 0;
    virtual void resizeClip(const std::string& clip_id, double new_length) = 0;

    virtual double getClipStartTime(const std::string& clip_id) const = 0;
    virtual double getClipLength(const std::string& clip_id) const = 0;

    virtual void addNoteToMidiClip(const std::string& clip_id, const MidiNote& note) = 0;
    virtual void removeNotesFromMidiClip(const std::string& clip_id, double start_time,
                                         double end_time) = 0;
    virtual std::vector<MidiNote> getMidiClipNotes(const std::string& clip_id) const = 0;

    virtual std::vector<std::string> getTrackClips(const std::string& track_id) const = 0;
    virtual bool clipExists(const std::string& clip_id) const = 0;
};

}  // namespace aidaw
