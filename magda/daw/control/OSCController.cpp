#include "OSCController.hpp"

#include <regex>

namespace magda {

OSCController::OSCController(int listenPort)
    : listenPort_(listenPort)
{
}

OSCController::~OSCController()
{
    close();
}

bool OSCController::open()
{
    if (connected_)
        close();

    if (!receiver_.connect(listenPort_))
        return false;

    receiver_.addListener(this);
    connected_ = true;

    // Connect sender for feedback
    if (feedbackEnabled_)
        sender_.connect(feedbackHost_, feedbackPort_);

    return true;
}

void OSCController::close()
{
    if (!connected_)
        return;

    receiver_.removeListener(this);
    receiver_.disconnect();
    sender_.disconnect();
    connected_ = false;
}

void OSCController::onFaderMove(int channel, float value)
{
    dispatchFader(channel, value);
}

void OSCController::onButtonPress(int id)
{
    dispatchButton(id);
}

void OSCController::onEncoderTurn(int id, int delta)
{
    dispatchEncoder(id, delta);
}

void OSCController::sendFeedback(int channel, float value)
{
    if (!feedbackEnabled_ || !connected_)
        return;

    juce::String address = "/track/" + juce::String(channel + 1) + "/volume";
    sender_.send(address, value);
}

void OSCController::sendLED(int id, bool state)
{
    if (!feedbackEnabled_ || !connected_)
        return;

    juce::String address = "/led/" + juce::String(id);
    sender_.send(address, state ? 1 : 0);
}

void OSCController::setFeedbackTarget(const juce::String& host, int port)
{
    feedbackHost_ = host;
    feedbackPort_ = port;
    feedbackEnabled_ = true;

    if (connected_)
    {
        sender_.disconnect();
        sender_.connect(feedbackHost_, feedbackPort_);
    }
}

void OSCController::addMapping(const juce::String& address, int channel)
{
    std::lock_guard<std::mutex> lock(mutex_);
    customMappings_[address] = channel;
}

void OSCController::removeMapping(const juce::String& address)
{
    std::lock_guard<std::mutex> lock(mutex_);
    customMappings_.erase(address);
}

void OSCController::oscMessageReceived(const juce::OSCMessage& message)
{
    const juce::String address = message.getAddressPattern().toString();

    // Check custom mappings first
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = customMappings_.find(address);
        if (it != customMappings_.end() && message.size() > 0 && message[0].isFloat32())
        {
            onFaderMove(it->second, message[0].getFloat32());
            return;
        }
    }

    // Handle standard address patterns
    if (address.startsWith("/track/"))
        handleTrackMessage(address, message);
    else if (address.startsWith("/transport/"))
        handleTransportMessage(address, message);
    else if (address == "/master/volume" && message.size() > 0 && message[0].isFloat32())
        onFaderMove(-1, message[0].getFloat32());  // -1 = master
}

void OSCController::oscBundleReceived(const juce::OSCBundle& bundle)
{
    for (const auto& element : bundle)
    {
        if (element.isMessage())
            oscMessageReceived(element.getMessage());
        else if (element.isBundle())
            oscBundleReceived(element.getBundle());
    }
}

void OSCController::handleTrackMessage(const juce::String& address,
                                       const juce::OSCMessage& message)
{
    // Parse /track/N/param
    // Extract track number
    auto parts = juce::StringArray::fromTokens(address, "/", "");
    // parts: ["", "track", "N", "param"]
    if (parts.size() < 4)
        return;

    int trackNum = parts[2].getIntValue() - 1;  // Convert to 0-based
    if (trackNum < 0)
        return;

    const juce::String& param = parts[3];

    if (message.size() == 0)
        return;

    if (param == "volume" && message[0].isFloat32())
    {
        onFaderMove(trackNum, message[0].getFloat32());
    }
    else if (param == "pan" && message[0].isFloat32())
    {
        // Pan uses encoder callback with a mapped value
        int delta = message[0].getFloat32() > 0.5f ? 1 : -1;
        onEncoderTurn(trackNum, delta);
    }
    else if (param == "mute" && message[0].isInt32())
    {
        // Button IDs: mute = trackNum * 10 + 0
        if (message[0].getInt32() != 0)
            onButtonPress(trackNum * 10);
    }
    else if (param == "solo" && message[0].isInt32())
    {
        // Button IDs: solo = trackNum * 10 + 1
        if (message[0].getInt32() != 0)
            onButtonPress(trackNum * 10 + 1);
    }
}

void OSCController::handleTransportMessage(const juce::String& address,
                                           const juce::OSCMessage& /*message*/)
{
    // Transport button IDs (negative to distinguish from track buttons)
    if (address == "/transport/play")
        onButtonPress(-1);
    else if (address == "/transport/stop")
        onButtonPress(-2);
    else if (address == "/transport/record")
        onButtonPress(-3);
    else if (address == "/transport/rewind")
        onButtonPress(-4);
    else if (address == "/transport/forward")
        onButtonPress(-5);
    else if (address == "/transport/loop")
        onButtonPress(-6);
}

}  // namespace magda
