#include "SendReturn.hpp"

namespace magda
{

//==============================================================================
// SendReturn
//==============================================================================

SendReturn::SendReturn (const juce::String& id,
                        const juce::String& sourceTrackId,
                        const juce::String& destBusId)
    : id_ (id), sourceTrackId_ (sourceTrackId), destBusId_ (destBusId)
{
}

SendReturn::~SendReturn() = default;

const juce::String& SendReturn::getId() const noexcept            { return id_; }
const juce::String& SendReturn::getSourceTrackId() const noexcept { return sourceTrackId_; }
const juce::String& SendReturn::getDestBusId() const noexcept     { return destBusId_; }

float SendReturn::getLevel() const noexcept { return level_; }

void SendReturn::setLevel (float level)
{
    level_ = juce::jlimit (0.0f, 1.0f, level);
}

bool SendReturn::isPreFader() const noexcept { return preFader_; }
void SendReturn::setPreFader (bool preFader) { preFader_ = preFader; }

SendInfo SendReturn::getInfo() const
{
    return { id_, sourceTrackId_, destBusId_, level_, preFader_ };
}

//==============================================================================
// SendReturnManager
//==============================================================================

SendReturnManager::SendReturnManager() = default;
SendReturnManager::~SendReturnManager() = default;

//==============================================================================
juce::String SendReturnManager::createReturnBus (const juce::String& name)
{
    ReturnBusInfo bus;
    bus.id   = "bus_" + juce::String (nextBusId_++);
    bus.name = name;
    returnBuses_.push_back (bus);
    listeners_.call (&SendReturnListener::returnBusCreated, bus);
    return bus.id;
}

void SendReturnManager::deleteReturnBus (const juce::String& busId)
{
    // Remove all sends routed to this bus
    sends_.erase (
        std::remove_if (sends_.begin(), sends_.end(),
                        [&] (const auto& s) { return s->getDestBusId() == busId; }),
        sends_.end());

    returnBuses_.erase (
        std::remove_if (returnBuses_.begin(), returnBuses_.end(),
                        [&] (const auto& b) { return b.id == busId; }),
        returnBuses_.end());

    listeners_.call (&SendReturnListener::returnBusDeleted, busId);
}

const ReturnBusInfo* SendReturnManager::getReturnBus (const juce::String& busId) const
{
    for (auto& bus : returnBuses_)
        if (bus.id == busId)
            return &bus;
    return nullptr;
}

const std::vector<ReturnBusInfo>& SendReturnManager::getReturnBuses() const noexcept
{
    return returnBuses_;
}

void SendReturnManager::setReturnBusGain (const juce::String& busId, float gain)
{
    for (auto& bus : returnBuses_)
    {
        if (bus.id == busId)
        {
            bus.gain = juce::jlimit (0.0f, 2.0f, gain);
            return;
        }
    }
}

void SendReturnManager::setReturnBusPan (const juce::String& busId, float pan)
{
    for (auto& bus : returnBuses_)
    {
        if (bus.id == busId)
        {
            bus.pan = juce::jlimit (-1.0f, 1.0f, pan);
            return;
        }
    }
}

//==============================================================================
juce::String SendReturnManager::createSend (const juce::String& sourceTrackId,
                                            const juce::String& destBusId)
{
    auto id = "send_" + juce::String (nextSendId_++);
    auto send = std::make_unique<SendReturn> (id, sourceTrackId, destBusId);
    auto info = send->getInfo();
    sends_.push_back (std::move (send));
    listeners_.call (&SendReturnListener::sendCreated, info);
    return id;
}

void SendReturnManager::deleteSend (const juce::String& sendId)
{
    sends_.erase (
        std::remove_if (sends_.begin(), sends_.end(),
                        [&] (const auto& s) { return s->getId() == sendId; }),
        sends_.end());
    listeners_.call (&SendReturnListener::sendDeleted, sendId);
}

SendReturn* SendReturnManager::getSend (const juce::String& sendId)
{
    for (auto& s : sends_)
        if (s->getId() == sendId)
            return s.get();
    return nullptr;
}

const SendReturn* SendReturnManager::getSend (const juce::String& sendId) const
{
    for (auto& s : sends_)
        if (s->getId() == sendId)
            return s.get();
    return nullptr;
}

std::vector<SendReturn*> SendReturnManager::getSendsForTrack (const juce::String& trackId)
{
    std::vector<SendReturn*> result;
    for (auto& s : sends_)
        if (s->getSourceTrackId() == trackId)
            result.push_back (s.get());
    return result;
}

std::vector<SendReturn*> SendReturnManager::getSendsForBus (const juce::String& busId)
{
    std::vector<SendReturn*> result;
    for (auto& s : sends_)
        if (s->getDestBusId() == busId)
            result.push_back (s.get());
    return result;
}

void SendReturnManager::addListener (SendReturnListener* listener)
{
    listeners_.add (listener);
}

void SendReturnManager::removeListener (SendReturnListener* listener)
{
    listeners_.remove (listener);
}

} // namespace magda
