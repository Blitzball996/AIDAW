#include "four_osc_apply.hpp"

#include <set>

#include "audio/AudioBridge.hpp"
#include "core/PresetManager.hpp"
#include "core/TrackManager.hpp"
#include "core/aliases/ParamNameNormalize.hpp"
#include "engine/AudioEngine.hpp"
#include "internal_plugins.hpp"

// te::FourOscPlugin
#include <tracktion_engine/tracktion_engine.h>

namespace magda {

namespace {

// 4OSC's waveShape enum: 0=none, 1=sine, 2=square, 3=saw, 4=triangle, 5=noise.
// Matches the FourOscUI selector ordering. "saw_up"/"saw_down" both map to
// the single saw shape because TE's oscillator only has one saw direction.
int waveNameToShapeInt(const juce::String& name) {
    auto k = name.trim().toLowerCase();
    if (k == "none" || k == "off")
        return 0;
    if (k == "sine")
        return 1;
    if (k == "square")
        return 2;
    if (k == "saw" || k == "saw_up" || k == "saw_down")
        return 3;
    if (k == "triangle")
        return 4;
    if (k == "noise" || k == "random")
        return 5;
    return -1;
}

// 0=bypass, 1=lp, 2=hp, 3=bp, 4=notch — mirrors FourOscUI selector ordering.
int filterTypeNameToInt(const juce::String& name) {
    auto k = name.trim().toLowerCase();
    if (k == "off" || k == "bypass" || k == "none")
        return 0;
    if (k == "lp" || k == "lowpass" || k == "low_pass")
        return 1;
    if (k == "hp" || k == "highpass" || k == "high_pass")
        return 2;
    if (k == "bp" || k == "bandpass" || k == "band_pass")
        return 3;
    if (k == "notch" || k == "bandreject" || k == "band_reject")
        return 4;
    return -1;
}

// 0=Mono, 1=Leg, 2=Poly per FourOscUI's selector default.
int voiceModeNameToInt(const juce::String& name) {
    auto k = name.trim().toLowerCase();
    if (k == "mono")
        return 0;
    if (k == "leg" || k == "legato")
        return 1;
    if (k == "poly" || k == "polyphonic")
        return 2;
    return -1;
}

}  // namespace

juce::String applyFourOscPresetToPath(const FourOscAgent::Preset& preset,
                                      const ChainNodePath& path) {
    DBG("applyFourOscPresetToPath: path.isValid=" + juce::String(path.isValid() ? "yes" : "no") +
        " trackId=" + juce::String(path.trackId) +
        " steps=" + juce::String(static_cast<int>(path.steps.size())));

    auto& tm = TrackManager::getInstance();
    auto* device = tm.getDeviceInChainByPath(path);
    if (device == nullptr || internalPluginFromId(device->pluginId) != InternalPlugin::FourOsc) {
        DBG("applyFourOscPresetToPath: device=" + juce::String(device ? "found" : "NULL") +
            (device ? " pluginId=" + device->pluginId : ""));
        return "target device is not a 4OSC";
    }

    DBG("applyFourOscPresetToPath: device='" + device->name + "' params=" +
        juce::String(static_cast<int>(device->parameters.size())) +
        " preset='" + juce::String(preset.name) + "' presetParams=" +
        juce::String(static_cast<int>(preset.params.size())));

    // Map every param name on the device to its index, normalized the same
    // way AutoAliasGenerator does, so the preset's alias-style keys
    // ("amp_attack", "tune_1", …) resolve here.
    std::map<juce::String, int> indexByName;
    for (int i = 0; i < static_cast<int>(device->parameters.size()); ++i) {
        auto key = normalizeParamName(device->parameters[static_cast<size_t>(i)].name);
        if (key.isNotEmpty())
            indexByName[key] = i;
    }

    // Get the live TE plugin to write parameters directly — this bypasses
    // the TrackManager→AudioBridge→Processor roundtrip that was silently
    // failing for factory presets.
    auto* engine = tm.getAudioEngine();
    auto* bridge = engine ? engine->getAudioBridge() : nullptr;
    auto plugin = bridge ? bridge->getPlugin(device->id) : nullptr;
    auto* fourOsc = dynamic_cast<te::FourOscPlugin*>(plugin.get());

    int applied = 0;
    int skipped = 0;
    if (fourOsc) {
        auto params = fourOsc->getAutomatableParameters();
        for (const auto& [name, value] : preset.params) {
            const juce::String key(name);
            auto it = indexByName.find(key);
            if (it == indexByName.end()) {
                ++skipped;
                continue;
            }
            const auto& paramInfo = device->parameters[static_cast<size_t>(it->second)];
            int teIdx = paramInfo.paramIndex;
            if (teIdx >= 0 && teIdx < params.size() && params[teIdx] != nullptr) {
                params[teIdx]->setParameter(value, juce::sendNotificationSync);
                ++applied;
            } else {
                ++skipped;
            }
        }
        // Re-read all parameter values from the live plugin back into DeviceInfo
        // so UI rebuilds see the correct state.
        for (int i = 0; i < static_cast<int>(device->parameters.size()); ++i) {
            auto& pi = device->parameters[static_cast<size_t>(i)];
            int teIdx = pi.paramIndex;
            if (teIdx >= 0 && teIdx < params.size() && params[teIdx] != nullptr)
                pi.currentValue = params[teIdx]->getCurrentValue();
        }
    } else {
        // Fallback: use TrackManager path
        for (const auto& [name, value] : preset.params) {
            const juce::String key(name);
            auto it = indexByName.find(key);
            if (it == indexByName.end()) {
                ++skipped;
                continue;
            }
            const auto& paramInfo = device->parameters[static_cast<size_t>(it->second)];
            tm.setDeviceParameterValue(path, paramInfo.paramIndex, ParameterModelValue{value});
            ++applied;
        }
    }

    // Wave shapes go through a separate path: they're stored as int
    // properties on FourOscPlugin's ValueTree, not as automatable
    // parameters. Reach the live plugin via the AudioBridge cache and
    // write directly. This mirrors what FourOscUI does on user click.
    int wavesApplied = 0;
    int wavesSkipped = 0;
    if (!preset.waves.empty()) {
        auto* engine = tm.getAudioEngine();
        auto* bridge = engine ? engine->getAudioBridge() : nullptr;
        auto plugin = bridge ? bridge->getPlugin(device->id) : nullptr;
        auto* fourOsc = dynamic_cast<te::FourOscPlugin*>(plugin.get());
        if (fourOsc) {
            for (const auto& [oscNum, waveName] : preset.waves) {
                const int shape = waveNameToShapeInt(juce::String(waveName));
                if (shape < 0) {
                    ++wavesSkipped;
                    continue;
                }
                auto propName = juce::Identifier("waveShape" + juce::String(oscNum));
                fourOsc->state.setProperty(propName, shape, nullptr);
                ++wavesApplied;
            }
        } else {
            wavesSkipped = static_cast<int>(preset.waves.size());
        }
    }

    // Filter type, voice mode and FX gates all go through the same
    // ValueTree-property path as wave shape. Without filter_type set to
    // non-off the filter is bypassed; without <fx>OnValue set true the
    // matching FX param values (size / mix / feedback / speed / width)
    // are inert.
    bool filterTypeApplied = false;
    bool voiceModeApplied = false;
    int fxToggled = 0;
    if (!preset.filterType.empty() || !preset.voiceMode.empty() || !preset.fx.empty()) {
        auto* engine = tm.getAudioEngine();
        auto* bridge = engine ? engine->getAudioBridge() : nullptr;
        auto plugin = bridge ? bridge->getPlugin(device->id) : nullptr;
        if (auto* fourOsc = dynamic_cast<te::FourOscPlugin*>(plugin.get())) {
            if (!preset.filterType.empty()) {
                const int ft = filterTypeNameToInt(juce::String(preset.filterType));
                if (ft >= 0) {
                    fourOsc->state.setProperty(juce::Identifier("filterType"), ft, nullptr);
                    filterTypeApplied = true;
                }
            }
            if (!preset.voiceMode.empty()) {
                const int vm = voiceModeNameToInt(juce::String(preset.voiceMode));
                if (vm >= 0) {
                    fourOsc->state.setProperty(juce::Identifier("voiceMode"), vm, nullptr);
                    voiceModeApplied = true;
                }
            }
            // FX gates — keys map 1:1 to <fx>OnValue ValueTree properties
            // on FourOscPlugin. Setting these is the difference between
            // "FX param emitted but inaudible" and "FX actually engaged".
            static const std::map<juce::String, juce::Identifier> kFxOnProps = {
                {"distortion", juce::Identifier("distortionOn")},
                {"reverb", juce::Identifier("reverbOn")},
                {"delay", juce::Identifier("delayOn")},
                {"chorus", juce::Identifier("chorusOn")},
            };
            for (const auto& [name, on] : preset.fx) {
                auto it = kFxOnProps.find(juce::String(name));
                if (it == kFxOnProps.end())
                    continue;
                fourOsc->state.setProperty(it->second, on, nullptr);
                ++fxToggled;
            }
        }
    }

    // Sync plugin state and notify UI to refresh
    if (bridge) {
        bridge->getPluginManager().capturePluginState(device->id);
    }
    tm.notifyTrackDevicesChanged(path.trackId);

    // Stash the agent's preset name as the default for the next save
    // dialog on this device. If the agent picked a category, prepend it
    // so the save dialog auto-fills both the Category and Name fields.
    if (!preset.name.empty()) {
        juce::String suggested = juce::String(preset.name);
        if (!preset.category.empty())
            suggested = juce::String(preset.category) + "/" + suggested;
        PresetManager::getInstance().setSuggestedPresetName(device->id, suggested);
    }

    juce::String status = "applied " + juce::String(applied) + " params";
    if (wavesApplied > 0)
        status += ", " + juce::String(wavesApplied) + " waves";
    if (filterTypeApplied)
        status += ", filter " + juce::String(preset.filterType);
    if (voiceModeApplied)
        status += ", voice " + juce::String(preset.voiceMode);
    if (fxToggled > 0)
        status += ", " + juce::String(fxToggled) + " fx gates";
    status += " to " + device->name;
    if (skipped > 0 || wavesSkipped > 0) {
        status += ", skipped";
        if (skipped > 0)
            status += " " + juce::String(skipped) + " params";
        if (wavesSkipped > 0)
            status += " " + juce::String(wavesSkipped) + " waves";
    }
    return status;
}

}  // namespace magda
