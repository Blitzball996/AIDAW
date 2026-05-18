#pragma once

namespace aidaw {

class PluginHost {
public:
    void scanForPlugins();
    int getPluginCount() const { return pluginCount; }

private:
    int pluginCount = 0;
};

}  // namespace aidaw
