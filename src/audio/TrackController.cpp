#include "TrackController.hpp"

namespace aidaw {

#ifdef AIDAW_HAS_TRACKTION

namespace {

te::VolumeAndPanPlugin* getFaderPlugin(te::AudioTrack* track) {
    if (!track)
        return nullptr;
    auto& plugins = track->pluginList;
    for (int i = plugins.size() - 1; i >= 0; --i) {
        if (auto* vp = dynamic_cast<te::VolumeAndPanPlugin*>(plugins[i])) {
            bool onlyMetersAfter = true;
            for (int j = i + 1; j < plugins.size(); ++j) {
                if (!dynamic_cast<te::LevelMeterPlugin*>(plugins[j])) {
                    onlyMetersAfter = false;
                    break;
                }
            }
            if (onlyMetersAfter)
                return vp;
        }
    }
    return track->getVolumePlugin();
}

}  // namespace

TrackController::TrackController(te::Engine& engine, te::Edit& edit)
    : engine_(engine), edit_(edit) {}

// =============================================================================
// Core Track Lifecycle
// =============================================================================

te::AudioTrack* TrackController::getAudioTrack(TrackId trackId) const {
    juce::ScopedLock lock(trackLock_);
    auto it = trackMapping_.find(trackId);
    return it != trackMapping_.end() ? it->second : nullptr;
}

te::AudioTrack* TrackController::createAudioTrack(TrackId trackId, const juce::String& name) {
    juce::ScopedLock lock(trackLock_);

    auto it = trackMapping_.find(trackId);
    if (it != trackMapping_.end() && it->second != nullptr) {
        return it->second;
    }

    auto insertPoint = te::TrackInsertPoint(nullptr, nullptr);
    auto trackPtr = edit_.insertNewAudioTrack(insertPoint, nullptr);

    te::AudioTrack* track = trackPtr.get();
    if (track) {
        track->setName(name);
        track->getOutput().setOutputToDefaultDevice(false);
        trackMapping_[trackId] = track;
    }

    return track;
}

void TrackController::removeAudioTrack(TrackId trackId) {
    te::AudioTrack* track = nullptr;

    {
        juce::ScopedLock lock(trackLock_);
        auto it = trackMapping_.find(trackId);
        if (it != trackMapping_.end()) {
            track = it->second;

            auto clientIt = meterClients_.find(trackId);
            if (clientIt != meterClients_.end()) {
                if (clientIt->second.measurer)
                    clientIt->second.measurer->removeClient(clientIt->second.client);
                meterClients_.erase(clientIt);
            }

            trackMapping_.erase(it);
        }
    }

    if (track) {
        edit_.deleteTrack(track);
    }
}

te::AudioTrack* TrackController::ensureTrackMapping(TrackId trackId, const juce::String& name) {
    auto* track = getAudioTrack(trackId);
    if (!track) {
        track = createAudioTrack(trackId, name);
    }
    return track;
}

// =============================================================================
// Mixer Controls
// =============================================================================

void TrackController::setTrackVolume(TrackId trackId, float volume) {
    auto* track = getAudioTrack(trackId);
    if (!track)
        return;

    if (auto* volPan = getFaderPlugin(track)) {
        float db = volume > 0.0f ? juce::Decibels::gainToDecibels(volume) : -100.0f;
        volPan->setVolumeDb(db);
    }
}

float TrackController::getTrackVolume(TrackId trackId) const {
    auto* track = getAudioTrack(trackId);
    if (!track)
        return 1.0f;

    if (auto* volPan = getFaderPlugin(track))
        return juce::Decibels::decibelsToGain(volPan->getVolumeDb());
    return 1.0f;
}

void TrackController::setTrackPan(TrackId trackId, float pan) {
    auto* track = getAudioTrack(trackId);
    if (!track)
        return;

    if (auto* volPan = getFaderPlugin(track))
        volPan->setPan(pan);
}

float TrackController::getTrackPan(TrackId trackId) const {
    auto* track = getAudioTrack(trackId);
    if (!track)
        return 0.0f;

    if (auto* volPan = getFaderPlugin(track))
        return volPan->getPan();
    return 0.0f;
}

// =============================================================================
// Audio Routing
// =============================================================================

void TrackController::setTrackAudioOutput(TrackId trackId, const juce::String& destination) {
    auto* track = getAudioTrack(trackId);
    if (!track)
        return;

    if (destination.isEmpty()) {
        track->getOutput().setOutputToDeviceID({});
    } else if (destination == "master") {
        if (track->getOutput().usesDefaultAudioOut())
            return;
        track->getOutput().setOutputToDefaultDevice(false);
    } else if (destination.startsWith("track:")) {
        TrackId targetId = destination.fromFirstOccurrenceOf("track:", false, false).getIntValue();
        auto* targetTrack = getAudioTrack(targetId);
        if (targetTrack) {
            track->getOutput().setOutputToDeviceID({});
            track->getOutput().setOutputToTrack(targetTrack);
        } else {
            track->getOutput().setOutputToDefaultDevice(false);
        }
    } else {
        track->getOutput().setOutputToDeviceID(destination);
    }
}

juce::String TrackController::getTrackAudioOutput(TrackId trackId) const {
    auto* track = getAudioTrack(trackId);
    if (!track)
        return {};

    auto& output = track->getOutput();
    if (output.usesDefaultAudioOut())
        return "master";

    if (auto* destTrack = output.getDestinationTrack()) {
        juce::ScopedLock lock(trackLock_);
        for (const auto& [id, teTrack] : trackMapping_) {
            if (teTrack == destTrack)
                return "track:" + juce::String(id);
        }
    }

    return output.getOutputName();
}

void TrackController::setTrackAudioInput(TrackId trackId, const juce::String& deviceId) {
    auto* track = getAudioTrack(trackId);
    if (!track)
        return;

    if (deviceId.isEmpty()) {
        auto* playbackContext = edit_.getCurrentPlaybackContext();
        if (playbackContext) {
            for (auto* inputDeviceInstance : playbackContext->getAllInputs()) {
                inputDeviceInstance->removeTarget(track->itemID, nullptr);
            }
        }
    } else {
        auto* playbackContext = edit_.getCurrentPlaybackContext();
        if (playbackContext) {
            auto allInputs = playbackContext->getAllInputs();

            if (deviceId == "default") {
                for (auto* input : allInputs) {
                    if (dynamic_cast<te::MidiInputDevice*>(&input->owner))
                        continue;
                    auto result = input->setTarget(track->itemID, false, nullptr);
                    if (result.has_value()) {
                        (*result)->recordEnabled = false;
                        break;
                    }
                }
            } else {
                auto resolvedName = deviceId.startsWith("stereo:")
                                        ? deviceId.fromFirstOccurrenceOf("stereo:", false, false)
                                        : deviceId;
                for (auto* inputDeviceInstance : allInputs) {
                    if (inputDeviceInstance->owner.getName() == resolvedName) {
                        auto result = inputDeviceInstance->setTarget(track->itemID, false, nullptr);
                        if (result.has_value())
                            (*result)->recordEnabled = false;
                        break;
                    }
                }
            }
        }
    }
}

juce::String TrackController::getTrackAudioInput(TrackId trackId) const {
    auto* track = getAudioTrack(trackId);
    if (!track)
        return {};

    auto* playbackContext = edit_.getCurrentPlaybackContext();
    if (playbackContext) {
        auto allInputs = playbackContext->getAllInputs();
        for (int i = 0; i < allInputs.size(); ++i) {
            auto* inputDeviceInstance = allInputs[i];
            auto targets = inputDeviceInstance->getTargets();
            for (auto targetID : targets) {
                if (targetID == track->itemID) {
                    if (i == 0)
                        return "default";
                    return inputDeviceInstance->owner.getName();
                }
            }
        }
    }

    return {};
}

// =============================================================================
// Utilities
// =============================================================================

std::vector<TrackId> TrackController::getAllTrackIds() const {
    juce::ScopedLock lock(trackLock_);
    std::vector<TrackId> trackIds;
    trackIds.reserve(trackMapping_.size());
    for (const auto& [trackId, track] : trackMapping_) {
        trackIds.push_back(trackId);
    }
    return trackIds;
}

void TrackController::clearAllMappings() {
    juce::ScopedLock lock(trackLock_);
    trackMapping_.clear();
    meterClients_.clear();
}

void TrackController::withTrackMapping(
    std::function<void(const std::map<TrackId, te::AudioTrack*>&)> callback) const {
    juce::ScopedLock lock(trackLock_);
    callback(trackMapping_);
}

// =============================================================================
// Metering Coordination
// =============================================================================

void TrackController::addMeterClient(TrackId trackId, te::LevelMeterPlugin* levelMeter) {
    if (!levelMeter)
        return;

    auto* measurer = &levelMeter->measurer;

    juce::ScopedLock lock(trackLock_);
    auto [it, inserted] = meterClients_.try_emplace(trackId);

    if (inserted) {
        it->second.measurer = measurer;
        measurer->addClient(it->second.client);
    } else if (it->second.measurer != measurer) {
        if (it->second.measurer)
            it->second.measurer->removeClient(it->second.client);
        it->second.measurer = measurer;
        measurer->addClient(it->second.client);
    }
}

void TrackController::removeMeterClient(TrackId trackId) {
    juce::ScopedLock lock(trackLock_);
    auto it = meterClients_.find(trackId);
    if (it != meterClients_.end()) {
        if (it->second.measurer)
            it->second.measurer->removeClient(it->second.client);
        meterClients_.erase(it);
    }
}

void TrackController::withMeterClients(
    std::function<void(std::map<TrackId, MeterClientEntry>&)> callback) {
    juce::ScopedLock lock(trackLock_);
    callback(meterClients_);
}

#else  // !AIDAW_HAS_TRACKTION

TrackController::TrackController() {}

void TrackController::setTrackVolume(TrackId, float) {}
float TrackController::getTrackVolume(TrackId) const { return 1.0f; }
void TrackController::setTrackPan(TrackId, float) {}
float TrackController::getTrackPan(TrackId) const { return 0.0f; }
void TrackController::setTrackAudioOutput(TrackId, const juce::String&) {}
juce::String TrackController::getTrackAudioOutput(TrackId) const { return {}; }
void TrackController::setTrackAudioInput(TrackId, const juce::String&) {}
juce::String TrackController::getTrackAudioInput(TrackId) const { return {}; }
std::vector<TrackId> TrackController::getAllTrackIds() const { return {}; }
void TrackController::clearAllMappings() {}

#endif  // AIDAW_HAS_TRACKTION

}  // namespace aidaw
