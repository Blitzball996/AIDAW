#include "GrooveTemplate.hpp"

namespace magda {

std::vector<GrooveTemplate> GrooveTemplateFactory::getBuiltInGrooves() {
    std::vector<GrooveTemplate> grooves;

    // MPC Swing 54% — subtle swing feel
    {
        GrooveTemplate g;
        g.name = "MPC Swing 54%";
        // 16 steps, every other step slightly delayed
        g.timingOffsets.resize(16, 0.0);
        g.velocityOffsets.resize(16, 0.0);
        for (int i = 1; i < 16; i += 2) {
            g.timingOffsets[static_cast<size_t>(i)] = 8.0;  // ~54% swing
            g.velocityOffsets[static_cast<size_t>(i)] = -0.05;
        }
        grooves.push_back(std::move(g));
    }

    // MPC Swing 71% — heavy swing
    {
        GrooveTemplate g;
        g.name = "MPC Swing 71%";
        g.timingOffsets.resize(16, 0.0);
        g.velocityOffsets.resize(16, 0.0);
        for (int i = 1; i < 16; i += 2) {
            g.timingOffsets[static_cast<size_t>(i)] = 42.0;  // ~71% swing
            g.velocityOffsets[static_cast<size_t>(i)] = -0.1;
        }
        grooves.push_back(std::move(g));
    }

    // Shuffle Light — gentle shuffle
    {
        GrooveTemplate g;
        g.name = "Shuffle Light";
        g.timingOffsets.resize(8, 0.0);
        g.velocityOffsets.resize(8, 0.0);
        for (int i = 1; i < 8; i += 2) {
            g.timingOffsets[static_cast<size_t>(i)] = 15.0;
            g.velocityOffsets[static_cast<size_t>(i)] = -0.08;
        }
        grooves.push_back(std::move(g));
    }

    // Shuffle Heavy — pronounced shuffle
    {
        GrooveTemplate g;
        g.name = "Shuffle Heavy";
        g.timingOffsets.resize(8, 0.0);
        g.velocityOffsets.resize(8, 0.0);
        for (int i = 1; i < 8; i += 2) {
            g.timingOffsets[static_cast<size_t>(i)] = 35.0;
            g.velocityOffsets[static_cast<size_t>(i)] = -0.15;
        }
        grooves.push_back(std::move(g));
    }

    return grooves;
}

GrooveTemplate GrooveTemplateFactory::getGrooveByName(const juce::String& name) {
    auto grooves = getBuiltInGrooves();
    for (const auto& g : grooves)
        if (g.name == name) return g;
    return {};
}

bool GrooveTemplateFactory::saveToFile(const GrooveTemplate& groove,
                                       const juce::File& file) {
    auto obj = std::make_unique<juce::DynamicObject>();
    obj->setProperty("name", groove.name);

    juce::var timingArray;
    for (auto t : groove.timingOffsets) timingArray.append(t);
    obj->setProperty("timingOffsets", timingArray);

    juce::var velArray;
    for (auto v : groove.velocityOffsets) velArray.append(v);
    obj->setProperty("velocityOffsets", velArray);

    return file.replaceWithText(juce::JSON::toString(juce::var(obj.release())));
}

GrooveTemplate GrooveTemplateFactory::loadFromFile(const juce::File& file) {
    GrooveTemplate groove;
    if (!file.existsAsFile()) return groove;

    auto json = juce::JSON::parse(file.loadFileAsString());
    if (json.isVoid()) return groove;

    groove.name = json.getProperty("name", "").toString();

    if (auto* timings = json.getProperty("timingOffsets", {}).getArray()) {
        for (const auto& t : *timings)
            groove.timingOffsets.push_back(static_cast<double>(t));
    }

    if (auto* vels = json.getProperty("velocityOffsets", {}).getArray()) {
        for (const auto& v : *vels)
            groove.velocityOffsets.push_back(static_cast<double>(v));
    }

    return groove;
}

}  // namespace magda
