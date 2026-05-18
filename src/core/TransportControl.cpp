#include "TransportControl.hpp"
#include "Engine.hpp"

namespace aidaw {

#ifdef AIDAW_HAS_TRACKTION

TransportControl::TransportControl(Engine& engine)
    : engineRef(engine) {}

te::TransportControl* TransportControl::getTransport() const {
    auto* edit = engineRef.getEdit();
    return edit ? &edit->getTransport() : nullptr;
}

void TransportControl::play() {
    if (auto* tc = getTransport())
        tc->play(false);
}

void TransportControl::stop() {
    if (auto* tc = getTransport())
        tc->stop(false, false);
}

void TransportControl::record() {
    if (auto* tc = getTransport())
        tc->record(false);
}

void TransportControl::setTempo(double bpm) {
    auto* edit = engineRef.getEdit();
    if (!edit) return;

    if (bpm > 0.0 && bpm < 999.0) {
        edit->tempoSequence.getTempo(0)->setBpm(bpm);
    }
}

void TransportControl::setPosition(double seconds) {
    if (auto* tc = getTransport())
        tc->setPosition(te::TimePosition::fromSeconds(seconds));
}

bool TransportControl::isPlaying() const {
    if (auto* tc = getTransport())
        return tc->isPlaying();
    return false;
}

bool TransportControl::isRecording() const {
    if (auto* tc = getTransport())
        return tc->isRecording();
    return false;
}

double TransportControl::getTempo() const {
    auto* edit = engineRef.getEdit();
    if (!edit) return 120.0;
    return edit->tempoSequence.getTempo(0)->getBpm();
}

double TransportControl::getPosition() const {
    if (auto* tc = getTransport())
        return tc->getPosition().inSeconds();
    return 0.0;
}

void TransportControl::setLoopRange(double startSeconds, double endSeconds) {
    if (auto* tc = getTransport()) {
        tc->setLoopRange(te::TimeRange {
            te::TimePosition::fromSeconds(startSeconds),
            te::TimePosition::fromSeconds(endSeconds)
        });
    }
}

void TransportControl::setLooping(bool enabled) {
    if (auto* tc = getTransport())
        tc->looping.setValue(enabled, nullptr);
}

#else  // Fallback placeholder implementation

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

void TransportControl::setPosition(double seconds) {
    if (seconds >= 0.0) position = seconds;
}

bool TransportControl::isPlaying() const { return playing; }
bool TransportControl::isRecording() const { return recording; }
double TransportControl::getTempo() const { return tempo; }
double TransportControl::getPosition() const { return position; }

void TransportControl::setLoopRange(double /*startSeconds*/, double /*endSeconds*/) {}
void TransportControl::setLooping(bool /*enabled*/) {}

#endif

}  // namespace aidaw
