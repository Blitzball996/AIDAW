#include "LuaScriptStore.hpp"

#include <algorithm>

// TODO: Port AppPaths from magda-core or provide AIDAW equivalent.
// For now, a local helper computes the scripts directory.

namespace aidaw::scripting {

namespace {

// Stub: returns the per-user AIDAW controller scripts directory.
// Replace with actual AIDAW paths utility once ported.
juce::File controllerScriptsDir() {
    auto appData = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    return appData.getChildFile("AIDAW").getChildFile("Scripts").getChildFile("Controllers");
}

}  // namespace

LuaScriptStore::LuaScriptStore() : root_(controllerScriptsDir()) {}

LuaScriptStore::LuaScriptStore(const juce::File& root) : root_(root) {}

bool LuaScriptStore::ensureExists() const {
    if (root_.exists())
        return root_.isDirectory();
    auto result = root_.createDirectory();
    return result.wasOk();
}

std::vector<juce::File> LuaScriptStore::enumerate() const {
    std::vector<juce::File> out;
    if (!root_.isDirectory())
        return out;

    auto found = root_.findChildFiles(juce::File::findFiles, false, "*.lua");
    out.reserve(static_cast<size_t>(found.size()));
    for (auto& f : found)
        out.push_back(f);

    std::sort(out.begin(), out.end(), [](const juce::File& a, const juce::File& b) {
        return a.getFileName().compareIgnoreCase(b.getFileName()) < 0;
    });

    return out;
}

juce::File LuaScriptStore::findBundledScriptsDirectory() {
    auto appFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile);

    juce::Array<juce::File> candidates;
#if JUCE_MAC
    candidates.add(appFile.getChildFile("Contents/Resources/controllers/scripts"));
#endif
#if JUCE_LINUX
    if (auto real = juce::File("/proc/self/exe").getLinkedTarget(); real.exists())
        candidates.add(real.getParentDirectory().getChildFile("controllers/scripts"));
#endif
    candidates.add(appFile.getParentDirectory().getChildFile("controllers/scripts"));

    auto walk = appFile.getParentDirectory();
    for (int i = 0; i < 8 && walk.exists(); ++i) {
        auto maybe =
            walk.getChildFile("resources").getChildFile("controllers").getChildFile("scripts");
        if (maybe.isDirectory()) {
            candidates.add(maybe);
            break;
        }
        walk = walk.getParentDirectory();
    }

    for (const auto& c : candidates)
        if (c.isDirectory())
            return c;
    return {};
}

}  // namespace aidaw::scripting
