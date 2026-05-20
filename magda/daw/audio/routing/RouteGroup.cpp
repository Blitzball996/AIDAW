#include "RouteGroup.hpp"

namespace magda
{

//==============================================================================
// RouteGroup
//==============================================================================

RouteGroup::RouteGroup (const juce::String& id, const juce::String& name)
    : id_ (id), name_ (name)
{
}

RouteGroup::~RouteGroup() = default;

const juce::String& RouteGroup::getId() const noexcept       { return id_; }
const juce::String& RouteGroup::getName() const noexcept     { return name_; }
void RouteGroup::setName (const juce::String& name)          { name_ = name; }
juce::Colour RouteGroup::getColour() const noexcept          { return colour_; }
void RouteGroup::setColour (juce::Colour colour)             { colour_ = colour; }

void RouteGroup::addTrack (const juce::String& trackId)
{
    if (! containsTrack (trackId))
        trackIds_.push_back (trackId);
}

void RouteGroup::removeTrack (const juce::String& trackId)
{
    trackIds_.erase (
        std::remove (trackIds_.begin(), trackIds_.end(), trackId),
        trackIds_.end());
}

bool RouteGroup::containsTrack (const juce::String& trackId) const
{
    return std::find (trackIds_.begin(), trackIds_.end(), trackId) != trackIds_.end();
}

const std::vector<juce::String>& RouteGroup::getTrackIds() const noexcept
{
    return trackIds_;
}

void RouteGroup::linkParam (LinkedParam param)
{
    if (! isParamLinked (param))
        linkedParams_.push_back (param);
}

void RouteGroup::unlinkParam (LinkedParam param)
{
    linkedParams_.erase (
        std::remove (linkedParams_.begin(), linkedParams_.end(), param),
        linkedParams_.end());
}

bool RouteGroup::isParamLinked (LinkedParam param) const
{
    return std::find (linkedParams_.begin(), linkedParams_.end(), param)
           != linkedParams_.end();
}

const std::vector<LinkedParam>& RouteGroup::getLinkedParams() const noexcept
{
    return linkedParams_;
}

RouteGroupInfo RouteGroup::getInfo() const
{
    return { id_, name_, colour_, trackIds_, linkedParams_ };
}

//==============================================================================
// RouteGroupManager
//==============================================================================

RouteGroupManager::RouteGroupManager() = default;
RouteGroupManager::~RouteGroupManager() = default;

juce::String RouteGroupManager::createGroup (const juce::String& name)
{
    auto id = "group_" + juce::String (nextGroupId_++);
    groups_.push_back (std::make_unique<RouteGroup> (id, name));
    listeners_.call (&RouteGroupListener::routeGroupStructureChanged);
    return id;
}

void RouteGroupManager::deleteGroup (const juce::String& groupId)
{
    groups_.erase (
        std::remove_if (groups_.begin(), groups_.end(),
                        [&] (const auto& g) { return g->getId() == groupId; }),
        groups_.end());
    listeners_.call (&RouteGroupListener::routeGroupStructureChanged);
}

RouteGroup* RouteGroupManager::getGroup (const juce::String& groupId)
{
    for (auto& g : groups_)
        if (g->getId() == groupId)
            return g.get();
    return nullptr;
}

const RouteGroup* RouteGroupManager::getGroup (const juce::String& groupId) const
{
    for (auto& g : groups_)
        if (g->getId() == groupId)
            return g.get();
    return nullptr;
}

const std::vector<std::unique_ptr<RouteGroup>>& RouteGroupManager::getGroups() const noexcept
{
    return groups_;
}

juce::String RouteGroupManager::getGroupForTrack (const juce::String& trackId) const
{
    for (auto& g : groups_)
        if (g->containsTrack (trackId))
            return g->getId();
    return {};
}

void RouteGroupManager::notifyParamChanged (const juce::String& trackId,
                                            LinkedParam param,
                                            float newValue)
{
    auto groupId = getGroupForTrack (trackId);
    if (groupId.isEmpty())
        return;

    auto* group = getGroup (groupId);
    if (group == nullptr || ! group->isParamLinked (param))
        return;

    // Propagate to all tracks in the group
    listeners_.call (&RouteGroupListener::routeGroupParamChanged,
                     groupId, trackId, param, newValue);
}

void RouteGroupManager::addListener (RouteGroupListener* listener)
{
    listeners_.add (listener);
}

void RouteGroupManager::removeListener (RouteGroupListener* listener)
{
    listeners_.remove (listener);
}

} // namespace magda
