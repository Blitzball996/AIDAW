#pragma once

#include <juce_core/juce_core.h>
#include <vector>

namespace magda
{

//==============================================================================
/// Information about a single send connection.
struct SendInfo
{
    juce::String id;
    juce::String sourceTrackId;
    juce::String destBusId;
    float        level     = 0.0f;  ///< Send level (0.0 - 1.0)
    bool         preFader  = false; ///< true = pre-fader, false = post-fader
};

//==============================================================================
/// Information about a return bus.
struct ReturnBusInfo
{
    juce::String id;
    juce::String name;
    float        gain = 1.0f;  ///< Return bus gain
    float        pan  = 0.0f;  ///< Return bus pan (-1 to 1)
};

//==============================================================================
/// Listener interface for send/return changes.
class SendReturnListener
{
public:
    virtual ~SendReturnListener() = default;
    virtual void sendCreated (const SendInfo& info) = 0;
    virtual void sendDeleted (const juce::String& sendId) = 0;
    virtual void sendLevelChanged (const juce::String& sendId, float newLevel) = 0;
    virtual void returnBusCreated (const ReturnBusInfo& info) = 0;
    virtual void returnBusDeleted (const juce::String& busId) = 0;
};

//==============================================================================
/**
    Manages aux send/return routing.
    Supports pre-fader and post-fader send modes.
*/
class SendReturn
{
public:
    SendReturn (const juce::String& id,
                const juce::String& sourceTrackId,
                const juce::String& destBusId);
    ~SendReturn();

    const juce::String& getId() const noexcept;
    const juce::String& getSourceTrackId() const noexcept;
    const juce::String& getDestBusId() const noexcept;

    /// Get/set send level (0.0 - 1.0).
    float getLevel() const noexcept;
    void setLevel (float level);

    /// Get/set pre-fader mode.
    bool isPreFader() const noexcept;
    void setPreFader (bool preFader);

    /// Get full info struct.
    SendInfo getInfo() const;

private:
    juce::String id_;
    juce::String sourceTrackId_;
    juce::String destBusId_;
    float        level_    = 0.0f;
    bool         preFader_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SendReturn)
};

//==============================================================================
/**
    Manages send/return connections and return buses.
*/
class SendReturnManager
{
public:
    SendReturnManager();
    ~SendReturnManager();

    //==========================================================================
    // Return bus management
    //==========================================================================

    /// Create a new return bus. Returns the bus ID.
    juce::String createReturnBus (const juce::String& name);

    /// Delete a return bus and all sends routed to it.
    void deleteReturnBus (const juce::String& busId);

    /// Get return bus info.
    const ReturnBusInfo* getReturnBus (const juce::String& busId) const;

    /// Get all return buses.
    const std::vector<ReturnBusInfo>& getReturnBuses() const noexcept;

    /// Set return bus gain.
    void setReturnBusGain (const juce::String& busId, float gain);

    /// Set return bus pan.
    void setReturnBusPan (const juce::String& busId, float pan);

    //==========================================================================
    // Send management
    //==========================================================================

    /// Create a send from a track to a return bus. Returns the send ID.
    juce::String createSend (const juce::String& sourceTrackId,
                             const juce::String& destBusId);

    /// Delete a send.
    void deleteSend (const juce::String& sendId);

    /// Get a send by ID.
    SendReturn* getSend (const juce::String& sendId);
    const SendReturn* getSend (const juce::String& sendId) const;

    /// Get all sends for a given track.
    std::vector<SendReturn*> getSendsForTrack (const juce::String& trackId);

    /// Get all sends routed to a given bus.
    std::vector<SendReturn*> getSendsForBus (const juce::String& busId);

    /// Add/remove listeners.
    void addListener (SendReturnListener* listener);
    void removeListener (SendReturnListener* listener);

private:
    std::vector<std::unique_ptr<SendReturn>> sends_;
    std::vector<ReturnBusInfo>               returnBuses_;
    juce::ListenerList<SendReturnListener>   listeners_;
    int nextSendId_ = 1;
    int nextBusId_  = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SendReturnManager)
};

} // namespace magda
