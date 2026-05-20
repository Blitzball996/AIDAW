#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <vector>
#include <unordered_map>

namespace magda
{

//==============================================================================
/// Parameters that can be linked within a route group.
enum class LinkedParam
{
    Volume,
    Pan,
    Mute,
    Solo,
    Record
};

//==============================================================================
/// Information about a route group.
struct RouteGroupInfo
{
    juce::String              id;
    juce::String              name;
    juce::Colour              colour { juce::Colours::grey };
    std::vector<juce::String> trackIds;
    std::vector<LinkedParam>  linkedParams;
};

//==============================================================================
/// Listener interface for route group changes.
class RouteGroupListener
{
public:
    virtual ~RouteGroupListener() = default;

    /// Called when a linked parameter changes on a track within a group.
    virtual void routeGroupParamChanged (const juce::String& groupId,
                                         const juce::String& sourceTrackId,
                                         LinkedParam param,
                                         float newValue) = 0;

    /// Called when a group is created, modified, or deleted.
    virtual void routeGroupStructureChanged() = 0;
};

//==============================================================================
/**
    Links multiple tracks for grouped control.
    When a linked parameter changes on one track, the change propagates
    to all other tracks in the group.
*/
class RouteGroup
{
public:
    RouteGroup (const juce::String& id, const juce::String& name);
    ~RouteGroup();

    const juce::String& getId() const noexcept;
    const juce::String& getName() const noexcept;
    void setName (const juce::String& name);

    juce::Colour getColour() const noexcept;
    void setColour (juce::Colour colour);

    /// Add a track to this group.
    void addTrack (const juce::String& trackId);

    /// Remove a track from this group.
    void removeTrack (const juce::String& trackId);

    /// Check if a track belongs to this group.
    bool containsTrack (const juce::String& trackId) const;

    /// Get all track IDs in this group.
    const std::vector<juce::String>& getTrackIds() const noexcept;

    /// Link a parameter for group control.
    void linkParam (LinkedParam param);

    /// Unlink a parameter.
    void unlinkParam (LinkedParam param);

    /// Check if a parameter is linked.
    bool isParamLinked (LinkedParam param) const;

    /// Get all linked parameters.
    const std::vector<LinkedParam>& getLinkedParams() const noexcept;

    /// Get full info struct.
    RouteGroupInfo getInfo() const;

private:
    juce::String              id_;
    juce::String              name_;
    juce::Colour              colour_ { juce::Colours::grey };
    std::vector<juce::String> trackIds_;
    std::vector<LinkedParam>  linkedParams_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RouteGroup)
};

//==============================================================================
/**
    Manages route groups: CRUD operations and change propagation.
*/
class RouteGroupManager
{
public:
    RouteGroupManager();
    ~RouteGroupManager();

    /// Create a new route group. Returns the group ID.
    juce::String createGroup (const juce::String& name);

    /// Delete a route group by ID.
    void deleteGroup (const juce::String& groupId);

    /// Get a group by ID (nullptr if not found).
    RouteGroup* getGroup (const juce::String& groupId);
    const RouteGroup* getGroup (const juce::String& groupId) const;

    /// Get all groups.
    const std::vector<std::unique_ptr<RouteGroup>>& getGroups() const noexcept;

    /// Find which group a track belongs to (empty string if none).
    juce::String getGroupForTrack (const juce::String& trackId) const;

    /// Notify that a parameter changed on a track. Propagates to group members.
    void notifyParamChanged (const juce::String& trackId,
                             LinkedParam param,
                             float newValue);

    /// Add/remove listeners.
    void addListener (RouteGroupListener* listener);
    void removeListener (RouteGroupListener* listener);

private:
    std::vector<std::unique_ptr<RouteGroup>> groups_;
    juce::ListenerList<RouteGroupListener>   listeners_;
    int nextGroupId_ = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RouteGroupManager)
};

} // namespace magda
