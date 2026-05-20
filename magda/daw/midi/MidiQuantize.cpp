#include "MidiQuantize.hpp"

#include <cmath>
#include <random>

namespace magda {

void MidiQuantize::quantize(std::vector<QuantizableNote>& notes,
                            const QuantizeSettings& settings) const {
    double gridSize = settings.getGridInBeats();
    float strength = juce::jlimit(0.0f, 100.0f, settings.strength) / 100.0f;
    float swing = juce::jlimit(0.0f, 100.0f, settings.swing) / 100.0f;
    float humanize = juce::jlimit(0.0f, 100.0f, settings.humanize) / 100.0f;

    for (auto& note : notes) {
        // Determine which grid position this note is closest to
        int gridIndex = static_cast<int>(std::round(note.startBeat / gridSize));
        double quantized =
            quantizePosition(note.startBeat, gridSize, strength, swing, gridIndex);

        if (humanize > 0.0f)
            quantized = addHumanize(quantized, humanize, gridSize);

        note.startBeat = quantized;
    }
}

void MidiQuantize::applyGroove(std::vector<QuantizableNote>& notes,
                               const GrooveTemplate& groove,
                               float strength) const {
    if (!groove.isValid() || strength <= 0.0f) return;

    float s = juce::jlimit(0.0f, 1.0f, strength);
    int stepCount = groove.getStepCount();

    for (auto& note : notes) {
        // Map note position to groove step
        // Assume groove covers one bar (4 beats) divided into stepCount steps
        double beatsPerStep = 4.0 / static_cast<double>(stepCount);
        double posInBar = std::fmod(note.startBeat, 4.0);
        int step = static_cast<int>(posInBar / beatsPerStep);

        // Apply timing offset (convert from ticks to beats, assuming 480 PPQN)
        double timingOffset = groove.getTimingOffset(step) / 480.0;
        note.startBeat += timingOffset * static_cast<double>(s);

        // Apply velocity offset
        double velOffset = groove.getVelocityOffset(step);
        note.velocity = juce::jlimit(
            0.0f, 1.0f,
            note.velocity + static_cast<float>(velOffset * static_cast<double>(s)));
    }
}

GrooveTemplate MidiQuantize::extractGroove(
    const std::vector<QuantizableNote>& notes, QuantizeGrid grid) const {
    GrooveTemplate groove;

    double gridSize = 0.25;  // default 1/16
    switch (grid) {
        case QuantizeGrid::Quarter:
            gridSize = 1.0;
            break;
        case QuantizeGrid::Eighth:
            gridSize = 0.5;
            break;
        case QuantizeGrid::Sixteenth:
            gridSize = 0.25;
            break;
        case QuantizeGrid::ThirtySecond:
            gridSize = 0.125;
            break;
    }

    // Determine number of steps per bar
    int stepsPerBar = static_cast<int>(4.0 / gridSize);
    groove.name = "Extracted Groove";
    groove.timingOffsets.resize(static_cast<size_t>(stepsPerBar), 0.0);
    groove.velocityOffsets.resize(static_cast<size_t>(stepsPerBar), 0.0);

    // Count notes per step for averaging
    std::vector<int> counts(static_cast<size_t>(stepsPerBar), 0);
    std::vector<double> avgVelocity(static_cast<size_t>(stepsPerBar), 0.0);

    for (const auto& note : notes) {
        double posInBar = std::fmod(note.startBeat, 4.0);
        int nearestStep = static_cast<int>(std::round(posInBar / gridSize));
        if (nearestStep >= stepsPerBar) nearestStep = 0;

        double gridPos = static_cast<double>(nearestStep) * gridSize;
        double deviation = posInBar - gridPos;

        auto idx = static_cast<size_t>(nearestStep);
        groove.timingOffsets[idx] += deviation * 480.0;  // Convert to ticks
        avgVelocity[idx] += static_cast<double>(note.velocity);
        counts[idx]++;
    }

    // Average the offsets
    double meanVelocity = 0.8;  // Reference velocity
    for (size_t i = 0; i < static_cast<size_t>(stepsPerBar); ++i) {
        if (counts[i] > 0) {
            groove.timingOffsets[i] /= static_cast<double>(counts[i]);
            avgVelocity[i] /= static_cast<double>(counts[i]);
            groove.velocityOffsets[i] = avgVelocity[i] - meanVelocity;
        }
    }

    return groove;
}

double MidiQuantize::quantizePosition(double position, double gridSize,
                                      float strength, float swing,
                                      int gridIndex) const {
    double nearestGrid = static_cast<double>(gridIndex) * gridSize;

    // Apply swing to off-beat positions (odd grid indices)
    if (gridIndex % 2 != 0 && swing > 0.0f) {
        double swingAmount = gridSize * 0.5 * static_cast<double>(swing);
        nearestGrid += swingAmount;
    }

    // Interpolate between original and quantized position based on strength
    return position + (nearestGrid - position) * static_cast<double>(strength);
}

double MidiQuantize::addHumanize(double position, float humanize,
                                 double gridSize) const {
    static std::mt19937 rng(42);  // Deterministic for reproducibility
    double maxOffset = gridSize * 0.1 * static_cast<double>(humanize);
    std::uniform_real_distribution<double> dist(-maxOffset, maxOffset);
    return position + dist(rng);
}

}  // namespace magda
