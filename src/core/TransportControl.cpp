#include "TransportControl.hpp"

namespace aidaw {

void TransportControl::play() {
    playing = true;
    recording = false;
}

void TransportControl::stop() {
    playing = false;
    recording = false;
}

void TransportControl::record() {
    playing = true;
    recording = true;
}

void TransportControl::setTempo(double bpm) {
    if (bpm > 0.0 && bpm < 999.0) tempo = bpm;
}

void TransportControl::setPosition(double beats) {
    if (beats >= 0.0) position = beats;
}

}  // namespace aidaw
