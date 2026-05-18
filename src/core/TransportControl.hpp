#pragma once

namespace aidaw {

class TransportControl {
public:
    void play();
    void stop();
    void record();
    void setTempo(double bpm);
    void setPosition(double beats);

    bool isPlaying() const { return playing; }
    bool isRecording() const { return recording; }
    double getTempo() const { return tempo; }
    double getPosition() const { return position; }

private:
    bool playing = false;
    bool recording = false;
    double tempo = 120.0;
    double position = 0.0;
};

}  // namespace aidaw
