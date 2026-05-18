#include "MidiBridge.hpp"
#include "AudioBridge.hpp"

namespace aidaw {

#ifdef AIDAW_HAS_TRACKTION
MidiBridge::MidiBridge(te::Engine& engine) : engine_(engine) {}
#else
MidiBridge::MidiBridge() {}
#endif

MidiBridge::~MidiBridge() {
    stopAllInputs();

    juce::ScopedLock lock(routingLock_);
    activeMidiOutputs_.clear();
}

void MidiBridge::setAudioBridge(AudioBridge* audioBridge) {
    audioBridge_ = audioBridge;
}

void MidiBridge::stopAllInputs() {
    isShuttingDown_.store(true, std::memory_order_release);

    std::unordered_map<juce::String, std::unique_ptr<juce::MidiInput>> inputsToDestroy;

    {
        juce::ScopedLock lock(routingLock_);
        inputsToDestroy = std::move(activeMidiInputs_);
        activeMidiInputs_.clear();
        trackMidiInputs_.clear();
        monitoredTracks_.clear();
    }

    for (auto& [deviceId, midiInput] : inputsToDestroy) {
        if (midiInput) {
            midiInput->stop();
            midiInput->removeCallback(*this);
        }
    }
    inputsToDestroy.clear();

    while (activeCallbacks_.load(std::memory_order_acquire) > 0)
        juce::Thread::sleep(1);
}

std::vector<MidiDeviceInfo> MidiBridge::getAvailableMidiInputs() const {
    std::vector<MidiDeviceInfo> devices;

    auto midiInputs = juce::MidiInput::getAvailableDevices();
    for (const auto& device : midiInputs) {
        MidiDeviceInfo info;
        info.id = device.identifier;
        info.name = device.name;
        info.isEnabled = false;
        info.isAvailable = true;

        {
            juce::ScopedLock lock(routingLock_);
            info.isEnabled = activeMidiInputs_.count(device.identifier) > 0;
        }

        devices.push_back(info);
    }

    return devices;
}

std::vector<MidiDeviceInfo> MidiBridge::getAvailableMidiOutputs() const {
    std::vector<MidiDeviceInfo> devices;

    auto midiOutputs = juce::MidiOutput::getAvailableDevices();
    for (const auto& device : midiOutputs) {
        MidiDeviceInfo info;
        info.id = device.identifier;
        info.name = device.name;
        info.isEnabled = false;
        info.isAvailable = true;
        devices.push_back(info);
    }

    return devices;
}

void MidiBridge::enableMidiInput(const juce::String& deviceId) {
    juce::ScopedLock lock(routingLock_);

    if (activeMidiInputs_.count(deviceId) > 0)
        return;

    auto devices = juce::MidiInput::getAvailableDevices();
    for (const auto& device : devices) {
        if (device.identifier == deviceId) {
            auto midiInput = juce::MidiInput::openDevice(device.identifier, this);
            if (midiInput) {
                midiInput->start();
                activeMidiInputs_[deviceId] = std::move(midiInput);
            }
            break;
        }
    }
}

void MidiBridge::disableMidiInput(const juce::String& deviceId) {
    juce::ScopedLock lock(routingLock_);

    auto it = activeMidiInputs_.find(deviceId);
    if (it != activeMidiInputs_.end()) {
        if (it->second) {
            it->second->stop();
            it->second->removeCallback(*this);
        }
        activeMidiInputs_.erase(it);
    }
}

bool MidiBridge::isMidiInputEnabled(const juce::String& deviceId) const {
    juce::ScopedLock lock(routingLock_);
    return activeMidiInputs_.count(deviceId) > 0;
}

void MidiBridge::setTrackMidiInput(TrackId trackId, const juce::String& midiDeviceId) {
    juce::ScopedLock lock(routingLock_);
    if (midiDeviceId.isEmpty())
        trackMidiInputs_.erase(trackId);
    else
        trackMidiInputs_[trackId] = midiDeviceId;
}

juce::String MidiBridge::getTrackMidiInput(TrackId trackId) const {
    juce::ScopedLock lock(routingLock_);
    auto it = trackMidiInputs_.find(trackId);
    return it != trackMidiInputs_.end() ? it->second : juce::String{};
}

void MidiBridge::clearTrackMidiInput(TrackId trackId) {
    juce::ScopedLock lock(routingLock_);
    trackMidiInputs_.erase(trackId);
}

void MidiBridge::startMonitoring(TrackId trackId) {
    juce::ScopedLock lock(routingLock_);
    monitoredTracks_.insert(trackId);
}

void MidiBridge::stopMonitoring(TrackId trackId) {
    juce::ScopedLock lock(routingLock_);
    monitoredTracks_.erase(trackId);
}

bool MidiBridge::isMonitoring(TrackId trackId) const {
    juce::ScopedLock lock(routingLock_);
    return monitoredTracks_.count(trackId) > 0;
}

bool MidiBridge::sendMidi(const juce::String& deviceNameOrId, const juce::MidiMessage& msg) {
    juce::ScopedLock lock(routingLock_);

    auto it = activeMidiOutputs_.find(deviceNameOrId);
    if (it != activeMidiOutputs_.end() && it->second) {
        it->second->sendMessageNow(msg);
        return true;
    }

    // Try to open the device lazily
    auto devices = juce::MidiOutput::getAvailableDevices();
    for (const auto& device : devices) {
        if (device.identifier == deviceNameOrId || device.name == deviceNameOrId) {
            auto output = juce::MidiOutput::openDevice(device.identifier);
            if (output) {
                output->sendMessageNow(msg);
                activeMidiOutputs_[deviceNameOrId] = std::move(output);
                return true;
            }
        }
    }

    return false;
}

bool MidiBridge::sendSysEx(const juce::String& deviceNameOrId, const juce::uint8* data,
                           size_t numBytes) {
    auto msg = juce::MidiMessage::createSysExMessage(data, static_cast<int>(numBytes));
    return sendMidi(deviceNameOrId, msg);
}

void MidiBridge::addRawMidiListener(RawMidiListener* listener) {
    juce::ScopedLock lock(rawMidiListenersLock_);
    rawMidiListeners_.addIfNotAlreadyThere(listener);
}

void MidiBridge::removeRawMidiListener(RawMidiListener* listener) {
    juce::ScopedLock lock(rawMidiListenersLock_);
    rawMidiListeners_.removeFirstMatchingValue(listener);
}

void MidiBridge::broadcastSynthesizedNote(const juce::String& /*sourceDeviceId*/,
                                          int noteNumber, int velocity, bool isNoteOn) {
    if (!audioBridge_)
        return;

    // Broadcast to all monitored tracks
    juce::ScopedLock lock(routingLock_);
    for (auto trackId : monitoredTracks_) {
        if (onNoteEvent) {
            MidiNoteEvent event;
            event.noteNumber = noteNumber;
            event.velocity = velocity;
            event.isNoteOn = isNoteOn;
            onNoteEvent(trackId, event);
        }
        audioBridge_->triggerMidiActivity(trackId);
    }
}

void MidiBridge::handleIncomingMidiMessage(juce::MidiInput* source,
                                           const juce::MidiMessage& message) {
    if (isShuttingDown_.load(std::memory_order_acquire))
        return;

    activeCallbacks_.fetch_add(1, std::memory_order_acquire);

    // Notify raw listeners
    {
        juce::ScopedLock lock(rawMidiListenersLock_);
        auto deviceId = source ? source->getIdentifier() : juce::String{};
        auto deviceName = source ? source->getName() : juce::String{};
        for (auto* listener : rawMidiListeners_) {
            listener->onRawMidi(deviceId, deviceName, message);
        }
    }

    // Route to tracks
    if (audioBridge_ && forwardMidiToPlugins_) {
        juce::ScopedLock lock(routingLock_);
        auto sourceId = source ? source->getIdentifier() : juce::String{};

        for (const auto& [trackId, midiDeviceId] : trackMidiInputs_) {
            if (midiDeviceId == sourceId || midiDeviceId == "all") {
                audioBridge_->triggerMidiActivity(trackId);

                if (monitoredTracks_.count(trackId) > 0) {
                    if (message.isNoteOnOrOff() && onNoteEvent) {
                        MidiNoteEvent event;
                        event.noteNumber = message.getNoteNumber();
                        event.velocity = message.getVelocity();
                        event.isNoteOn = message.isNoteOn();
                        event.channel = message.getChannel();
                        onNoteEvent(trackId, event);
                    } else if (message.isController() && onCCEvent) {
                        MidiCCEvent event;
                        event.ccNumber = message.getControllerNumber();
                        event.value = message.getControllerValue();
                        event.channel = message.getChannel();
                        onCCEvent(trackId, event);
                    }
                }
            }
        }
    }

    activeCallbacks_.fetch_sub(1, std::memory_order_release);
}

}  // namespace aidaw
