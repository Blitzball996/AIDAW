#pragma once

#include <juce_graphics/juce_graphics.h>

#include <string>

namespace magda {

using LocationId = int;
constexpr LocationId INVALID_LOCATION_ID = -1;

/**
 * @brief Type of location marker
 */
enum class LocationType {
    Marker,   // Single point marker
    Range,    // Named range (selection memory)
    Loop,     // Loop range (transport loop)
    Punch,    // Punch in/out range (recording)
    CDTrack   // CD track marker (for mastering/export)
};

/**
 * @brief Complete information for a location marker or range
 */
struct LocationInfo {
    LocationId id = INVALID_LOCATION_ID;
    LocationType type = LocationType::Marker;
    std::string name;
    double startTime = 0.0;  // seconds
    double endTime = 0.0;    // seconds (only meaningful for range types)
    juce::Colour colour = juce::Colours::yellow;
    bool locked = false;

    bool isRange() const {
        return type == LocationType::Range || type == LocationType::Loop ||
               type == LocationType::Punch || type == LocationType::CDTrack;
    }

    bool isValid() const {
        return id != INVALID_LOCATION_ID;
    }
};

}  // namespace magda
