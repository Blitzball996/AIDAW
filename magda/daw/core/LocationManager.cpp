#include "LocationManager.hpp"

#include <algorithm>
#include <cmath>

namespace magda {

LocationManager& LocationManager::getInstance() {
    static LocationManager instance;
    return instance;
}

LocationManager::LocationManager() = default;

// --- Marker CRUD ---

LocationId LocationManager::addMarker(const std::string& name, double time,
                                      juce::Colour colour) {
    LocationInfo info;
    info.id = allocateId();
    info.type = LocationType::Marker;
    info.name = name;
    info.startTime = time;
    info.endTime = time;
    info.colour = colour;

    locations_.push_back(info);
    notifyLocationsChanged();
    return info.id;
}

LocationId LocationManager::addRange(const std::string& name, double startTime,
                                     double endTime, juce::Colour colour) {
    LocationInfo info;
    info.id = allocateId();
    info.type = LocationType::Range;
    info.name = name;
    info.startTime = startTime;
    info.endTime = endTime;
    info.colour = colour;

    locations_.push_back(info);
    notifyLocationsChanged();
    return info.id;
}

LocationId LocationManager::addCDMarker(const std::string& name,
                                        double startTime, double endTime) {
    LocationInfo info;
    info.id = allocateId();
    info.type = LocationType::CDTrack;
    info.name = name;
    info.startTime = startTime;
    info.endTime = endTime;
    info.colour = juce::Colours::red;

    locations_.push_back(info);
    notifyLocationsChanged();
    return info.id;
}

void LocationManager::removeLocation(LocationId id) {
    auto it = std::find_if(locations_.begin(), locations_.end(),
                           [id](const LocationInfo& loc) {
                               return loc.id == id;
                           });
    if (it != locations_.end()) {
        locations_.erase(it);
        notifyLocationsChanged();
    }
}

void LocationManager::restoreLocation(const LocationInfo& info) {
    // Insert maintaining time order
    auto it = std::lower_bound(
        locations_.begin(), locations_.end(), info,
        [](const LocationInfo& a, const LocationInfo& b) {
            return a.startTime < b.startTime;
        });
    locations_.insert(it, info);

    // Ensure the ID won't collide with future allocations
    if (info.id >= nextId_)
        nextId_ = info.id + 1;

    notifyLocationsChanged();
}

// --- Accessors ---

const LocationInfo* LocationManager::getLocation(LocationId id) const {
    for (const auto& loc : locations_) {
        if (loc.id == id)
            return &loc;
    }
    return nullptr;
}

LocationInfo* LocationManager::getLocation(LocationId id) {
    for (auto& loc : locations_) {
        if (loc.id == id)
            return &loc;
    }
    return nullptr;
}

std::vector<const LocationInfo*> LocationManager::getLocationsByType(
    LocationType type) const {
    std::vector<const LocationInfo*> result;
    for (const auto& loc : locations_) {
        if (loc.type == type)
            result.push_back(&loc);
    }
    return result;
}

const LocationInfo* LocationManager::findNearestMarker(double time) const {
    const LocationInfo* nearest = nullptr;
    double minDist = std::numeric_limits<double>::max();

    for (const auto& loc : locations_) {
        if (loc.type != LocationType::Marker)
            continue;

        double dist = std::abs(loc.startTime - time);
        if (dist < minDist) {
            minDist = dist;
            nearest = &loc;
        }
    }

    return nearest;
}

// --- Property setters ---

void LocationManager::setLocationName(LocationId id,
                                      const std::string& name) {
    if (auto* loc = getLocation(id)) {
        loc->name = name;
        notifyLocationsChanged();
    }
}

void LocationManager::setLocationTime(LocationId id, double startTime) {
    if (auto* loc = getLocation(id)) {
        double duration = loc->endTime - loc->startTime;
        loc->startTime = startTime;
        if (loc->isRange())
            loc->endTime = startTime + duration;
        else
            loc->endTime = startTime;
        notifyLocationsChanged();
    }
}

void LocationManager::setLocationRange(LocationId id, double startTime,
                                       double endTime) {
    if (auto* loc = getLocation(id)) {
        loc->startTime = startTime;
        loc->endTime = endTime;
        notifyLocationsChanged();
    }
}

void LocationManager::setLocationColour(LocationId id, juce::Colour colour) {
    if (auto* loc = getLocation(id)) {
        loc->colour = colour;
        notifyLocationsChanged();
    }
}

void LocationManager::setLocationLocked(LocationId id, bool locked) {
    if (auto* loc = getLocation(id)) {
        loc->locked = locked;
        notifyLocationsChanged();
    }
}

// --- Loop range ---

void LocationManager::setLoopRange(double startTime, double endTime) {
    loopEnabled_ = true;
    loopStart_ = startTime;
    loopEnd_ = endTime;
    notifyLoopRangeChanged();
}

void LocationManager::clearLoopRange() {
    loopEnabled_ = false;
    loopStart_ = 0.0;
    loopEnd_ = 0.0;
    notifyLoopRangeChanged();
}

// --- Punch range ---

void LocationManager::setPunchRange(double startTime, double endTime) {
    punchEnabled_ = true;
    punchStart_ = startTime;
    punchEnd_ = endTime;
    notifyPunchRangeChanged();
}

void LocationManager::clearPunchRange() {
    punchEnabled_ = false;
    punchStart_ = 0.0;
    punchEnd_ = 0.0;
    notifyPunchRangeChanged();
}

// --- Bulk operations ---

void LocationManager::clearAllLocations() {
    locations_.clear();
    notifyLocationsChanged();
}

// --- Listener management ---

void LocationManager::addListener(LocationManagerListener* listener) {
    if (listener &&
        std::find(listeners_.begin(), listeners_.end(), listener) ==
            listeners_.end()) {
        listeners_.push_back(listener);
    }
}

void LocationManager::removeListener(LocationManagerListener* listener) {
    listeners_.erase(
        std::remove(listeners_.begin(), listeners_.end(), listener),
        listeners_.end());
}

// --- Private ---

LocationId LocationManager::allocateId() {
    return nextId_++;
}

void LocationManager::notifyLocationsChanged() {
    for (auto* listener : listeners_)
        listener->locationsChanged();
}

void LocationManager::notifyLoopRangeChanged() {
    for (auto* listener : listeners_)
        listener->loopRangeChanged();
}

void LocationManager::notifyPunchRangeChanged() {
    for (auto* listener : listeners_)
        listener->punchRangeChanged();
}

}  // namespace magda
