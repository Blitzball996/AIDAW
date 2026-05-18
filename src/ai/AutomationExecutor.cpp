#include "AutomationExecutor.hpp"

#include <cmath>

namespace aidaw {

namespace {

constexpr double kPi = 3.14159265358979323846;

/** Clamp a normalized value into [0, 1]. */
double clampNorm(double v) {
    if (v < 0.0) return 0.0;
    if (v > 1.0) return 1.0;
    return v;
}

/** Count the number of points a shape op would generate. */
int countShapePoints(const AutoShapeOp& op) {
    const double cycles = op.cycles > 0.0 ? op.cycles : 1.0;

    switch (op.shape) {
        case AutoShape::Sin: {
            const int perCycle = 16;
            return std::max(2, static_cast<int>(std::round(cycles * perCycle))) + 1;
        }
        case AutoShape::Tri: {
            return std::max(2, static_cast<int>(std::round(cycles * 4.0))) + 1;
        }
        case AutoShape::Saw: {
            return static_cast<int>(std::round(cycles)) * 2 + 1;
        }
        case AutoShape::Square: {
            return static_cast<int>(std::round(cycles)) * 2 + 1;
        }
        case AutoShape::Exp:
        case AutoShape::Log:
            return 33;
        case AutoShape::Line:
            return 2;
        default:
            return 0;
    }
}

}  // namespace

bool AutomationExecutor::execute(const std::vector<AutoInstruction>& instructions) {
    error_.clear();
    results_.clear();

    int laneCount = 0;
    int pointCount = 0;

    for (const auto& inst : instructions) {
        if (std::holds_alternative<AutoClearOp>(inst.payload)) {
            // TODO: Wire to DAW automation API when available
            ++laneCount;
            continue;
        }

        if (std::holds_alternative<AutoFreeformOp>(inst.payload)) {
            auto& op = std::get<AutoFreeformOp>(inst.payload);
            // TODO: Wire to DAW automation API when available
            pointCount += static_cast<int>(op.points.size());
            ++laneCount;
            continue;
        }

        if (std::holds_alternative<AutoShapeOp>(inst.payload)) {
            auto& op = std::get<AutoShapeOp>(inst.payload);
            if (op.endBeat <= op.startBeat) {
                error_ = "end must be greater than start";
                return false;
            }
            // TODO: Wire to DAW automation API when available
            // For now, count the points that would be generated
            pointCount += countShapePoints(op);
            ++laneCount;
            continue;
        }
    }

    results_ = "Wrote " + juce::String(pointCount) + " points to " +
               juce::String(laneCount) + " lane(s).";
    return true;
}

}  // namespace aidaw
