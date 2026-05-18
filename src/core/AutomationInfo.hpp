#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

#include <array>
#include <vector>

#include "AutomationTypes.hpp"
#include "TypeIds.hpp"

namespace aidaw {

/**
 * @brief Visual state for a control bound to an automation target.
 */
enum class AutomationVisualState {
    None,        // No lane exists
    Active,      // Lane exists and is driving the parameter
    Overridden,  // Lane exists but the user has taken over
};

/**
 * @brief Bezier handle for smooth curve control
 */
struct BezierHandle {
    double beatOffset = 0.0;
    double value = 0.0;
    bool linked = true;

    bool isZero() const { return beatOffset == 0.0 && value == 0.0; }
};

/**
 * @brief A single point on an automation curve
 */
struct AutomationPoint {
    AutomationPointId id = INVALID_AUTOMATION_POINT_ID;
    double beatPosition = 0.0;
    double value = 0.5;  // Normalized 0-1

    AutomationCurveType curveType = AutomationCurveType::Linear;
    BezierHandle inHandle;
    BezierHandle outHandle;
    double tension = 0.0;  // -1.0 to +1.0

    bool operator<(const AutomationPoint& other) const {
        return beatPosition < other.beatPosition;
    }

    bool operator==(const AutomationPoint& other) const {
        return id == other.id;
    }
};

/**
 * @brief Automation target — identifies what parameter a lane controls
 */
struct AutomationTarget {
    TrackId trackId = INVALID_TRACK_ID;
    DeviceId deviceId = INVALID_DEVICE_ID;
    int paramIndex = -1;

    enum class Kind { TrackVolume, TrackPan, PluginParam };
    Kind kind = Kind::TrackVolume;

    bool operator==(const AutomationTarget& other) const {
        return trackId == other.trackId && deviceId == other.deviceId &&
               paramIndex == other.paramIndex && kind == other.kind;
    }
};

/**
 * @brief An automation clip for clip-based automation
 */
struct AutomationClipInfo {
    AutomationClipId id = INVALID_AUTOMATION_CLIP_ID;
    AutomationLaneId laneId = INVALID_AUTOMATION_LANE_ID;
    juce::String name;
    juce::Colour colour;

    double startBeats = 0.0;
    double lengthBeats = 4.0;

    bool looping = false;
    double loopLengthBeats = 4.0;

    std::vector<AutomationPoint> points;

    double getEndBeats() const { return startBeats + lengthBeats; }

    bool containsBeat(double beat) const {
        return beat >= startBeats && beat < getEndBeats();
    }

    double getLocalBeat(double globalBeat) const {
        double localBeatPosition = globalBeat - startBeats;
        if (looping && loopLengthBeats > 0.0) {
            localBeatPosition = std::fmod(localBeatPosition, loopLengthBeats);
            if (localBeatPosition < 0.0)
                localBeatPosition += loopLengthBeats;
        }
        return localBeatPosition;
    }

    static inline const std::array<juce::uint32, 8> defaultColors = {
        0xFFCC8866, 0xFFCCCC66, 0xFF66CC88, 0xFF66CCCC,
        0xFF6688CC, 0xFF8866CC, 0xFFCC66AA, 0xFFCC6666,
    };

    static juce::Colour getDefaultColor(int index) {
        return juce::Colour(defaultColors[index % defaultColors.size()]);
    }
};

/**
 * @brief An automation lane containing curve data for a target
 */
struct AutomationLaneInfo {
    AutomationLaneId id = INVALID_AUTOMATION_LANE_ID;
    AutomationTarget target;
    AutomationLaneType type = AutomationLaneType::Absolute;

    juce::String paramName;
    juce::String name;
    bool visible = true;
    bool expanded = true;
    bool bypass = false;
    bool touchSuppressed = false;
    bool snapEditsToBeatGrid = true;
    bool snapValue = false;
    int height = 60;

    std::vector<AutomationPoint> absolutePoints;
    std::vector<AutomationClipId> clipIds;

    bool isAbsolute() const { return type == AutomationLaneType::Absolute; }
    bool isClipBased() const { return type == AutomationLaneType::ClipBased; }

    juce::String getDisplayName() const {
        if (name.isNotEmpty()) return name;
        if (paramName.isNotEmpty()) return paramName;
        return "Automation";
    }

    bool hasData() const {
        if (isAbsolute()) return !absolutePoints.empty();
        return !clipIds.empty();
    }
};

}  // namespace aidaw
