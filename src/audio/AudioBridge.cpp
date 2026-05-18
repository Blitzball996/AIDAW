#include "AudioBridge.hpp"

namespace aidaw {

#ifdef AIDAW_HAS_TRACKTION

AudioBridge::AudioBridge(te::Engine& engine, te::Edit& edit)
    : teEngine_(engine),
      edit_(edit),
      trackController_(engine, edit) {
    startTimer(30);  // 30ms metering refresh (~33Hz)
}

#else

AudioBridge::AudioBridge()
    : trackController_() {
    startTimer(30);
}

#endif

AudioBridge::~AudioBridge() {
    isShuttingDown_.store(true, std::memory_order_release);
    stopTimer();
}

// =============================================================================
// Clip Synchronization
// =============================================================================

void AudioBridge::syncClipToEngine(ClipId /*clipId*/) {
#ifdef AIDAW_HAS_TRACKTION
    // TODO: Implement clip-to-engine synchronization
#endif
}

void AudioBridge::removeClipFromEngine(ClipId /*clipId*/) {
#ifdef AIDAW_HAS_TRACKTION
    // TODO: Implement clip removal from engine
#endif
}

// =============================================================================
// Warp Markers
// =============================================================================

std::vector<WarpMarkerInfo> AudioBridge::getWarpMarkers(ClipId /*clipId*/) {
    // TODO: Delegate to warpMarkerManager_
    return {};
}

int AudioBridge::addWarpMarker(ClipId /*clipId*/, double /*sourceTime*/, double /*warpTime*/) {
    return -1;
}

double AudioBridge::moveWarpMarker(ClipId /*clipId*/, int /*index*/, double newWarpTime) {
    return newWarpTime;
}

void AudioBridge::removeWarpMarker(ClipId /*clipId*/, int /*index*/) {
}

// =============================================================================
// Track Mapping
// =============================================================================

#ifdef AIDAW_HAS_TRACKTION
te::AudioTrack* AudioBridge::getAudioTrack(TrackId trackId) const {
    return trackController_.getAudioTrack(trackId);
}

te::AudioTrack* AudioBridge::createAudioTrack(TrackId trackId, const juce::String& name) {
    return trackController_.createAudioTrack(trackId, name);
}

void AudioBridge::removeAudioTrack(TrackId trackId) {
    trackController_.removeAudioTrack(trackId);
}
#endif

// =============================================================================
// Mixer Controls
// =============================================================================

void AudioBridge::setTrackVolume(TrackId trackId, float volume) {
    trackController_.setTrackVolume(trackId, volume);
}

float AudioBridge::getTrackVolume(TrackId trackId) const {
    return trackController_.getTrackVolume(trackId);
}

void AudioBridge::setTrackPan(TrackId trackId, float pan) {
    trackController_.setTrackPan(trackId, pan);
}

float AudioBridge::getTrackPan(TrackId trackId) const {
    return trackController_.getTrackPan(trackId);
}

void AudioBridge::setMasterVolume(float volume) {
#ifdef AIDAW_HAS_TRACKTION
    if (auto* masterVol = edit_.getMasterVolumePlugin()) {
        float db = volume > 0.0f ? juce::Decibels::gainToDecibels(volume) : -100.0f;
        masterVol->setVolumeDb(db);
    }
#else
    juce::ignoreUnused(volume);
#endif
}

float AudioBridge::getMasterVolume() const {
#ifdef AIDAW_HAS_TRACKTION
    if (auto* masterVol = edit_.getMasterVolumePlugin())
        return juce::Decibels::decibelsToGain(masterVol->getVolumeDb());
#endif
    return 1.0f;
}

void AudioBridge::setMasterPan(float pan) {
#ifdef AIDAW_HAS_TRACKTION
    if (auto* masterVol = edit_.getMasterVolumePlugin())
        masterVol->setPan(pan);
#else
    juce::ignoreUnused(pan);
#endif
}

float AudioBridge::getMasterPan() const {
#ifdef AIDAW_HAS_TRACKTION
    if (auto* masterVol = edit_.getMasterVolumePlugin())
        return masterVol->getPan();
#endif
    return 0.0f;
}

// =============================================================================
// Transport State
// =============================================================================

void AudioBridge::updateTransportState(bool isPlaying, bool justStarted, bool justLooped) {
    transportState_.updateState(isPlaying, justStarted, justLooped);
}

// =============================================================================
// MIDI Activity
// =============================================================================

void AudioBridge::triggerMidiActivity(TrackId trackId) {
    juce::ScopedLock lock(midiActivityLock_);
    midiActivityCounters_[trackId].fetch_add(1, std::memory_order_relaxed);
}

uint32_t AudioBridge::getMidiActivityCounter(TrackId trackId) const {
    juce::ScopedLock lock(midiActivityLock_);
    auto it = midiActivityCounters_.find(trackId);
    if (it != midiActivityCounters_.end())
        return it->second.load(std::memory_order_relaxed);
    return 0;
}

// =============================================================================
// Audio Routing
// =============================================================================

void AudioBridge::setTrackAudioOutput(TrackId trackId, const juce::String& destination) {
    trackController_.setTrackAudioOutput(trackId, destination);
}

void AudioBridge::setTrackAudioInput(TrackId trackId, const juce::String& deviceId) {
    trackController_.setTrackAudioInput(trackId, deviceId);
}

juce::String AudioBridge::getTrackAudioOutput(TrackId trackId) const {
    return trackController_.getTrackAudioOutput(trackId);
}

juce::String AudioBridge::getTrackAudioInput(TrackId trackId) const {
    return trackController_.getTrackAudioInput(trackId);
}

// =============================================================================
// MIDI Routing
// =============================================================================

void AudioBridge::setTrackMidiInput(TrackId /*trackId*/, const juce::String& /*midiDeviceId*/) {
#ifdef AIDAW_HAS_TRACKTION
    // TODO: Delegate to MidiInputRouter
#endif
}

juce::String AudioBridge::getTrackMidiInput(TrackId /*trackId*/) const {
    return {};
}

void AudioBridge::enableAllMidiInputDevices() {
#ifdef AIDAW_HAS_TRACKTION
    // TODO: Enable all MIDI input devices in TE DeviceManager
#endif
}

// =============================================================================
// Synchronization
// =============================================================================

void AudioBridge::syncAll() {
#ifdef AIDAW_HAS_TRACKTION
    // TODO: Full sync of all tracks and devices
#endif
}

void AudioBridge::syncTrackPlugins(TrackId /*trackId*/) {
#ifdef AIDAW_HAS_TRACKTION
    // TODO: Sync single track's plugins
#endif
}

// =============================================================================
// Audio Callback Support
// =============================================================================

void AudioBridge::processParameterChanges() {
    // TODO: Process pending parameter changes from UI thread
}

void AudioBridge::updateMetering() {
    deviceMetering_.updateAllClients();
}

// =============================================================================
// Timer Callback
// =============================================================================

void AudioBridge::timerCallback() {
    if (isShuttingDown_.load(std::memory_order_acquire))
        return;

    updateMetering();
}

}  // namespace aidaw
