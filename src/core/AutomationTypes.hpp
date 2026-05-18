#pragma once

namespace aidaw {

/**
 * @brief Type of automation lane
 */
enum class AutomationLaneType {
    Absolute,  // Single curve spanning entire timeline
    ClipBased  // Automation clips that can loop/stretch
};

/**
 * @brief Curve interpolation type between automation points
 */
enum class AutomationCurveType {
    Linear,  // Straight line between points
    Bezier,  // Smooth bezier curve with control handles
    Step     // Instant jump to next value (no interpolation)
};

/**
 * @brief Drawing/editing mode for automation curves
 */
enum class AutomationDrawMode {
    Select,  // Select and move existing points
    Pencil,  // Freehand drawing creates points
    Line,    // Draw straight line segments
    Curve    // Draw smooth curves
};

inline const char* getLaneTypeName(AutomationLaneType type) {
    switch (type) {
        case AutomationLaneType::Absolute: return "Absolute";
        case AutomationLaneType::ClipBased: return "Clip-Based";
    }
    return "Unknown";
}

inline const char* getCurveTypeName(AutomationCurveType type) {
    switch (type) {
        case AutomationCurveType::Linear: return "Linear";
        case AutomationCurveType::Bezier: return "Bezier";
        case AutomationCurveType::Step: return "Step";
    }
    return "Unknown";
}

inline const char* getDrawModeName(AutomationDrawMode mode) {
    switch (mode) {
        case AutomationDrawMode::Select: return "Select";
        case AutomationDrawMode::Pencil: return "Pencil";
        case AutomationDrawMode::Line: return "Line";
        case AutomationDrawMode::Curve: return "Curve";
    }
    return "Unknown";
}

}  // namespace aidaw
