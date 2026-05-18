#pragma once

#include <string>
#include <vector>

#ifdef AIDAW_HAS_TRACKTION
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion;
#endif

namespace aidaw {

class Engine;

struct PluginDescription {
    std::string name;
    std::string manufacturer;
    std::string format;  // "VST3", "AU", "Internal"
    std::string identifier;
};

class PluginHost {
public:
#ifdef AIDAW_HAS_TRACKTION
    explicit PluginHost(Engine& engine);
#else
    PluginHost() = default;
#endif

    void scanForPlugins();
    int getPluginCount() const;
    std::vector<PluginDescription> getAvailablePlugins() const;

private:
#ifdef AIDAW_HAS_TRACKTION
    Engine& engineRef;
#endif
    int pluginCount = 0;
};

}  // namespace aidaw
