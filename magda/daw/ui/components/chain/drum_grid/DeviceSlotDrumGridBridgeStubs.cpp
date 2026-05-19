#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
#include <map>
#include <vector>
#include <optional>
#include "core/ChainNodePath.hpp"
#include "core/MacroInfo.hpp"
#include "core/ModInfo.hpp"
#include "core/DeviceInfo.hpp"
#include "core/ParameterInfo.hpp"
#include "audio/plugins/FaustPlugin.hpp"
#include "audio/plugins/FaustParamSlot.hpp"
#include "audio/plugins/FaustParamPool.hpp"

namespace magda::daw::ui { class NodeComponent; class DrumGridUI; }

namespace magda::daw::ui::drum_grid_slot {
    struct PadChainLinkCallbacks {};
    void applySlotName(NodeComponent&, bool, const juce::String&) {}
    bool paintHeaderLogo(juce::Graphics&, bool, bool, int, int, const juce::Component*, std::initializer_list<const juce::Component*>) { return false; }
    std::optional<juce::Point<float>> getControllerIndicatorAnchor(bool, bool, int, const juce::Component*) { return std::nullopt; }
    bool shouldShowSidechainButton(bool, bool, bool) { return false; }
    juce::String getCollapsedName(bool, const juce::String& name, const juce::String&) { return name; }
    std::vector<tracktion::engine::Plugin*> getCollapsedPlugins(const DrumGridUI*) { return {}; }
    void setCollapsedPlugins(DrumGridUI*, const std::vector<tracktion::engine::Plugin*>&) {}
    int getPreferredContentWidth(bool, const DrumGridUI*) { return 200; }
    void setPadChainLinkContext(DrumGridUI*, const magda::ChainNodePath&, const std::vector<magda::MacroInfo>*, const std::vector<magda::ModInfo>*, const std::vector<magda::MacroInfo>*, const std::vector<magda::ModInfo>*) {}
    void appendAvailableDevices(const DrumGridUI*, std::vector<std::pair<int, juce::String>>&) {}
    void appendDeviceParamNames(const DrumGridUI*, std::map<int, std::vector<juce::String>>&) {}
    void wirePadChainLinkCallbacks(DrumGridUI*, PadChainLinkCallbacks) {}
    bool shouldShowModButton(bool, magda::DeviceType) { return false; }
    bool shouldShowMacroButton(bool, magda::DeviceType, bool, bool) { return false; }
    bool layoutDrumGridUI(DrumGridUI*, juce::Rectangle<int>) { return false; }
    bool paintContentHeader(juce::Graphics&, bool, bool, juce::Rectangle<int>) { return false; }
    bool shouldShowCollapsedUiButton(bool, bool) { return false; }
}

namespace magda::daw::audio {
    const char* FaustPlugin::xmlTypeName = "faust";
    bool FaustPlugin::loadDspSource(const juce::String&, const juce::String&, juce::String&, FaustCustomViewKind) { return false; }
    magda::ParameterInfo paramInfoFromSlot(const FaustParamSlot&) { return {}; }
    int FaustParamPool::activeCount() const { return 0; }
    struct StarterDsp { juce::String name, code; };
    std::vector<StarterDsp> getBundledStarterDsps() { return {}; }
}


#include "audio/processors/CompiledFaustProcessor.hpp"
namespace magda {
    CompiledFaustProcessor::CompiledFaustProcessor(int deviceId, juce::ReferenceCountedObjectPtr<tracktion::engine::Plugin> plugin) : DeviceProcessor(deviceId, plugin) {}
    int CompiledFaustProcessor::getParameterCount() const { return 0; }
    ParameterInfo CompiledFaustProcessor::getParameterInfo(int) const { return {}; }
    void CompiledFaustProcessor::populateParameters(DeviceInfo&) const {}
    void CompiledFaustProcessor::setParameterByIndex(int, float) {}
}
