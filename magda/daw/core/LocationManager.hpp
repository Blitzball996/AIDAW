#pragma once

#include <vector>

#include "LocationTypes.hpp"

namespace magda {

/**
 * @brief Listener interface for location/marker changes
 */
class LocationManagerListener {
  public:
    virtual ~LocationManagerListener() = default;

    /// Called when markers are added, removed, or modified
    virtual void locationsChanged() = 0;

    /// Called when the loop range changes
    virtual void loopRangeChanged() {}

    /// Called when the punch range changes
    virtual void punchRangeChanged() {}
};

/**
 * @brief Singleton manager for all location markers and ranges
 *
 * Manages markers (single points), named ranges, loop range, punch range,
 * and CD track markers. Provides CRUD operations and notifies listeners.
 *
 * Usage:
 *   auto& lm = LocationManager::getInstance();
 *   LocationId id = lm.addMarker("Chorus", 32.5);
 *   lm.setLoopRange(16.0, 48.0);
 */
class LocationManager {
  public:
    static LocationManager& getInstance();

    // Prevent copying
    LocationManager(const LocationManager&) = delete;
    LocationManager& operator=(const LocationManager&) = delete;

    // --- Marker CRUD ---

    /**
     * @brief Add a point marker at the given time
     * @return The new marker's ID
     */
    LocationId addMarker(const std::string& name, double time,
                         juce::Colour colour = juce::Colours::yellow);

    /**
     * @brief Add a range marker
     * @return The new range's ID
     */
    LocationId addRange(const std::string& name, double startTime, double endTime,
                        juce::Colour colour = juce::Colours::cyan);

    /**
     * @brief Add a CD track marker
     * @return The new CD marker's ID
     */
    LocationId addCDMarker(const std::string& name, double startTime, double endTime);

    /**
     * @brief Remove a location by ID
     */
    void removeLocation(LocationId id);

    /**
     * @brief Restore a previously removed location (used by undo)
     */
    void restoreLocation(const LocationInfo& info);

    // --- Accessors ---

    const LocationInfo* getLocation(LocationId id) const;
    LocationInfo* getLocation(LocationId id);
    const std::vector<LocationInfo>& getLocations() const { return locations_; }

    /**
     * @brief Get all locations of a specific type
     */
    std::vector<const LocationInfo*> getLocationsByType(LocationType type) const;

    /**
     * @brief Find the nearest marker to a given time
     * @return Pointer to nearest marker, or nullptr if none exist
     */
    const LocationInfo* findNearestMarker(double time) const;

    // --- Property setters ---

    void setLocationName(LocationId id, const std::string& name);
    void setLocationTime(LocationId id, double startTime);
    void setLocationRange(LocationId id, double startTime, double endTime);
    void setLocationColour(LocationId id, juce::Colour colour);
    void setLocationLocked(LocationId id, bool locked);

    // --- Loop range ---

    void setLoopRange(double startTime, double endTime);
    void clearLoopRange();
    bool hasLoopRange() const { return loopEnabled_; }
    double getLoopStart() const { return loopStart_; }
    double getLoopEnd() const { return loopEnd_; }

    // --- Punch range ---

    void setPunchRange(double startTime, double endTime);
    void clearPunchRange();
    bool hasPunchRange() const { return punchEnabled_; }
    double getPunchStart() const { return punchStart_; }
    double getPunchEnd() const { return punchEnd_; }

    // --- Bulk operations ---

    void clearAllLocations();
    int getNumLocations() const { return static_cast<int>(locations_.size()); }

    // --- Listener management ---

    void addListener(LocationManagerListener* listener);
    void removeListener(LocationManagerListener* listener);

  private:
    LocationManager();
    ~LocationManager() = default;

    LocationId allocateId();

    void notifyLocationsChanged();
    void notifyLoopRangeChanged();
    void notifyPunchRangeChanged();

    std::vector<LocationInfo> locations_;
    std::vector<LocationManagerListener*> listeners_;

    int nextId_ = 1;

    // Loop state
    bool loopEnabled_ = false;
    double loopStart_ = 0.0;
    double loopEnd_ = 0.0;

    // Punch state
    bool punchEnabled_ = false;
    double punchStart_ = 0.0;
    double punchEnd_ = 0.0;
};

}  // namespace magda
