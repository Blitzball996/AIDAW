#pragma once

namespace aidaw {

/**
 * @brief Interface for controlling DAW transport (playback control)
 *
 * The TransportInterface provides methods for controlling playback,
 * recording, and position within the DAW timeline.
 */
class TransportInterface {
  public:
    virtual ~TransportInterface() = default;

    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void record() = 0;

    /** Set playback position in seconds */
    virtual void locate(double position_seconds) = 0;

    /** Set playback position in bars/beats */
    virtual void locateMusical(int bar, int beat, int tick = 0) = 0;

    virtual double getCurrentPosition() const = 0;
    virtual void getCurrentMusicalPosition(int& bar, int& beat, int& tick) const = 0;

    virtual bool isPlaying() const = 0;
    virtual bool isRecording() const = 0;

    virtual void setTempo(double bpm) = 0;
    virtual double getTempo() const = 0;

    virtual void setTimeSignature(int numerator, int denominator) = 0;
    virtual void getTimeSignature(int& numerator, int& denominator) const = 0;

    virtual void setLooping(bool enabled) = 0;
    virtual void setLoopRegion(double start_seconds, double end_seconds) = 0;
    virtual bool isLooping() const = 0;

    /**
     * @brief Check if transport just started playing (true for one frame after play starts)
     * Used for LFO trigger mode to reset phase on transport start
     */
    virtual bool justStarted() const = 0;

    /**
     * @brief Check if transport just looped back (true for one frame after loop point)
     * Used for LFO trigger mode to reset phase on loop
     */
    virtual bool justLooped() const = 0;
};

}  // namespace aidaw
