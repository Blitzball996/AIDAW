#pragma once

#ifdef AIDAW_HAS_TRACKTION
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion;
#endif

namespace aidaw {

class Engine;

class TransportControl {
public:
#ifdef AIDAW_HAS_TRACKTION
    explicit TransportControl(Engine& engine);
#else
    TransportControl() = default;
#endif

    void play();
    void stop();
    void record();
    void setTempo(double bpm);
    void setPosition(double seconds);

    bool isPlaying() const;
    bool isRecording() const;
    double getTempo() const;
    double getPosition() const;

    void setLoopRange(double startSeconds, double endSeconds);
    void setLooping(bool enabled);

private:
#ifdef AIDAW_HAS_TRACKTION
    Engine& engineRef;
    te::TransportControl* getTransport() const;
#else
    bool playing = false;
    bool recording = false;
    double tempo = 120.0;
    double position = 0.0;
#endif
};

}  // namespace aidaw
