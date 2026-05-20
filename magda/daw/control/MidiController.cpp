#include "MidiController.hpp"

#include <algorithm>

namespace magda {

MidiController::MidiController() = default;

MidiController::~MidiController()
{
    close();
}

bool MidiController::open()
{
    if (connected_)
        close();

    // Open MIDI input
    if (inputDeviceId_.isNotEmpty())
    {
        auto devices = juce::MidiInput::getAvailableDevices();
        for (const auto& device : devices)
        {
            if (device.identifier == inputDeviceId_ || device.name == inputDeviceId_)
            {
                midiInput_ = juce::MidiInput::openDevice(device.identifier, this);
                if (midiInput_)
                {
                    midiInput_->start();
                    connected_ = true;
                }
                break;
            }
        }
    }

    // Open MIDI output for feedback
    if (outputDeviceId_.isNotEmpty())
    {
        auto devices = juce::MidiOutput::getAvailableDevices();
        for (const auto& device : devices)
        {
            if (device.identifier == outputDeviceId_ || device.name == outputDeviceId_)
            {
                midiOutput_ = juce::MidiOutput::openDevice(device.identifier);
                break;
            }
        }
    }

    return connected_;
}

void MidiController::close()
{
    if (midiInput_)
    {
        midiInput_->stop();
        midiInput_.reset();
    }
    midiOutput_.reset();
    connected_ = false;
}

void MidiController::onFaderMove(int channel, float value)
{
    dispatchFader(channel, value);
}

void MidiController::onButtonPress(int id)
{
    dispatchButton(id);
}

void MidiController::onEncoderTurn(int id, int delta)
{
    dispatchEncoder(id, delta);
}

void MidiController::sendFeedback(int channel, float value)
{
    if (!midiOutput_)
        return;

    std::lock_guard<std::mutex> lock(mutex_);

    // Find the mapping for this channel and send CC feedback
    for (const auto& [key, mapping] : mappings_)
    {
        if (mapping.targetChannel == channel)
        {
            int ccValue = static_cast<int>(value * 127.0f);
            ccValue = std::clamp(ccValue, 0, 127);

            auto msg = juce::MidiMessage::controllerEvent(
                mapping.midiChannel + 1, mapping.ccNumber, ccValue);
            midiOutput_->sendMessageNow(msg);
            break;
        }
    }
}

void MidiController::sendLED(int id, bool state)
{
    if (!midiOutput_)
        return;

    // Send note-on with velocity 127 (on) or 0 (off) for LED feedback
    auto msg = juce::MidiMessage::noteOn(1, id, static_cast<juce::uint8>(state ? 127 : 0));
    midiOutput_->sendMessageNow(msg);
}

void MidiController::setInputDevice(const juce::String& deviceIdentifier)
{
    inputDeviceId_ = deviceIdentifier;
}

void MidiController::setOutputDevice(const juce::String& deviceIdentifier)
{
    outputDeviceId_ = deviceIdentifier;
}

juce::StringArray MidiController::getAvailableInputDevices()
{
    juce::StringArray result;
    for (const auto& device : juce::MidiInput::getAvailableDevices())
        result.add(device.name);
    return result;
}

juce::StringArray MidiController::getAvailableOutputDevices()
{
    juce::StringArray result;
    for (const auto& device : juce::MidiOutput::getAvailableDevices())
        result.add(device.name);
    return result;
}

void MidiController::addMapping(const MidiCCMapping& mapping)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int key = (mapping.ccNumber << 4) | (mapping.midiChannel & 0x0F);
    mappings_[key] = mapping;
}

void MidiController::removeMapping(int ccNumber, int midiChannel)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int key = (ccNumber << 4) | (midiChannel & 0x0F);
    mappings_.erase(key);
}

void MidiController::clearMappings()
{
    std::lock_guard<std::mutex> lock(mutex_);
    mappings_.clear();
}

std::vector<MidiCCMapping> MidiController::getMappings() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MidiCCMapping> result;
    result.reserve(mappings_.size());
    for (const auto& [key, mapping] : mappings_)
        result.push_back(mapping);
    return result;
}

void MidiController::listenForNextCC(int targetChannel,
                                     std::function<void(int, int)> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    learning_ = true;
    learnTargetChannel_ = targetChannel;
    learnCallback_ = std::move(callback);
}

void MidiController::cancelLearn()
{
    std::lock_guard<std::mutex> lock(mutex_);
    learning_ = false;
    learnCallback_ = nullptr;
}

void MidiController::loadPreset(const MidiControllerPreset& preset)
{
    std::lock_guard<std::mutex> lock(mutex_);
    mappings_.clear();
    name_ = preset.name;

    for (const auto& mapping : preset.mappings)
    {
        int key = (mapping.ccNumber << 4) | (mapping.midiChannel & 0x0F);
        mappings_[key] = mapping;
    }
}

std::vector<MidiControllerPreset> MidiController::getBuiltInPresets()
{
    std::vector<MidiControllerPreset> presets;

    // Generic 8-fader controller
    {
        MidiControllerPreset preset;
        preset.name = "Generic 8-Fader";
        preset.manufacturer = "Generic";
        for (int i = 0; i < 8; ++i)
        {
            MidiCCMapping m;
            m.ccNumber = i + 1;  // CC1-CC8
            m.midiChannel = 0;
            m.targetChannel = i;
            preset.mappings.push_back(m);
        }
        presets.push_back(std::move(preset));
    }

    // Korg nanoKONTROL2
    {
        MidiControllerPreset preset;
        preset.name = "Korg nanoKONTROL2";
        preset.manufacturer = "Korg";
        // Faders (CC0-CC7)
        for (int i = 0; i < 8; ++i)
        {
            MidiCCMapping m;
            m.ccNumber = i;
            m.midiChannel = 0;
            m.targetChannel = i;
            preset.mappings.push_back(m);
        }
        // Knobs (CC16-CC23)
        for (int i = 0; i < 8; ++i)
        {
            MidiCCMapping m;
            m.ccNumber = 16 + i;
            m.midiChannel = 0;
            m.targetChannel = i;
            m.isRelative = true;
            preset.mappings.push_back(m);
        }
        presets.push_back(std::move(preset));
    }

    // Akai APC40
    {
        MidiControllerPreset preset;
        preset.name = "Akai APC40";
        preset.manufacturer = "Akai";
        // Track faders (CC7, channels 1-8)
        for (int i = 0; i < 8; ++i)
        {
            MidiCCMapping m;
            m.ccNumber = 7;
            m.midiChannel = i;
            m.targetChannel = i;
            preset.mappings.push_back(m);
        }
        presets.push_back(std::move(preset));
    }

    return presets;
}

void MidiController::handleIncomingMidiMessage(juce::MidiInput* /*source*/,
                                               const juce::MidiMessage& message)
{
    if (message.isController())
    {
        handleCC(message.getControllerNumber(),
                 message.getControllerValue(),
                 message.getChannel() - 1);  // Convert to 0-based
    }
    else if (message.isNoteOn())
    {
        handleNoteOn(message.getNoteNumber(),
                     message.getVelocity(),
                     message.getChannel() - 1);
    }
}

void MidiController::handleCC(int cc, int value, int midiChannel)
{
    std::function<void(int, int)> callbackToInvoke;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        // Learn mode: capture this CC
        if (learning_)
        {
            MidiCCMapping newMapping;
            newMapping.ccNumber = cc;
            newMapping.midiChannel = midiChannel;
            newMapping.targetChannel = learnTargetChannel_;

            int key = (cc << 4) | (midiChannel & 0x0F);
            mappings_[key] = newMapping;

            learning_ = false;

            if (learnCallback_)
            {
                callbackToInvoke = std::move(learnCallback_);
                learnCallback_ = nullptr;
            }
            // Fall through to invoke callback outside lock
        }
        else
        {
            // Normal mode: look up mapping
            int key = (cc << 4) | (midiChannel & 0x0F);
            auto it = mappings_.find(key);
            if (it == mappings_.end())
            {
                // Try omni (channel 0)
                key = (cc << 4);
                it = mappings_.find(key);
                if (it == mappings_.end())
                    return;
            }

            const auto& mapping = it->second;

            if (mapping.isRelative)
            {
                // Relative encoder: value 1-63 = CW, 65-127 = CCW
                int delta = (value < 64) ? value : (value - 128);
                onEncoderTurn(mapping.targetChannel, delta);
            }
            else
            {
                float floatValue = ccToFloat(value, mapping);
                onFaderMove(mapping.targetChannel, floatValue);
            }
            return;
        }
    }

    // Invoke learn callback outside the lock
    if (callbackToInvoke)
        callbackToInvoke(cc, midiChannel);
}

void MidiController::handleNoteOn(int note, int /*velocity*/, int /*midiChannel*/)
{
    // Note-on triggers button press
    onButtonPress(note);
}

float MidiController::ccToFloat(int ccValue, const MidiCCMapping& mapping) const
{
    float normalised = static_cast<float>(ccValue) / 127.0f;
    return mapping.minValue + normalised * (mapping.maxValue - mapping.minValue);
}

}  // namespace magda
