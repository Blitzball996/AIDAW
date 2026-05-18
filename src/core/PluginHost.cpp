#include "PluginHost.hpp"
#include "Engine.hpp"

namespace aidaw {

#ifdef AIDAW_HAS_TRACKTION

PluginHost::PluginHost(Engine& engine)
    : engineRef(engine) {}

void PluginHost::scanForPlugins() {
    auto& pm = engineRef.getTracktionEngine().getPluginManager();
    pluginCount = pm.knownPluginList.getNumTypes();
}

int PluginHost::getPluginCount() const {
    return pluginCount;
}

std::vector<PluginDescription> PluginHost::getAvailablePlugins() const {
    std::vector<PluginDescription> result;
    auto& pm = engineRef.getTracktionEngine().getPluginManager();

    for (auto& desc : pm.knownPluginList.getTypes()) {
        PluginDescription pd;
        pd.name = desc.name.toStdString();
        pd.manufacturer = desc.manufacturerName.toStdString();
        pd.format = desc.pluginFormatName.toStdString();
        pd.identifier = desc.createIdentifierString().toStdString();
        result.push_back(std::move(pd));
    }

    return result;
}

#else  // Fallback placeholder implementation

void PluginHost::scanForPlugins() {
    pluginCount = 0;
}

int PluginHost::getPluginCount() const {
    return pluginCount;
}

std::vector<PluginDescription> PluginHost::getAvailablePlugins() const {
    return {};
}

#endif

}  // namespace aidaw
