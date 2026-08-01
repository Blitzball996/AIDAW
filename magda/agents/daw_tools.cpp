#include "daw_tools.hpp"

#include <algorithm>

#include "../daw/api/clip_api.hpp"
#include "../daw/api/magda_api.hpp"
#include "../daw/api/track_api.hpp"
#include "../daw/audio/plugins/InstrumentCatalog.hpp"
#include "../daw/core/ClipInfo.hpp"
#include "../daw/core/DeviceInfo.hpp"
#include "../daw/core/RackInfo.hpp"
#include "../daw/core/TrackManager.hpp"

namespace magda {

namespace {

juce::var jsonObject() {
    return juce::var(new juce::DynamicObject());
}

void setProp(juce::var& obj, const juce::Identifier& key, const juce::var& value) {
    if (auto* d = obj.getDynamicObject())
        d->setProperty(key, value);
}

juce::String toJson(const juce::var& v) {
    return juce::JSON::toString(v, true);
}

juce::String errorJson(const juce::String& message) {
    auto obj = jsonObject();
    setProp(obj, "error", message);
    return toJson(obj);
}

/** Schema helper: an object with the given properties, all optional. */
juce::var schema(std::initializer_list<std::pair<const char*, const char*>> props) {
    auto properties = jsonObject();
    for (const auto& [name, description] : props) {
        auto p = jsonObject();
        setProp(p, "type", "string");
        setProp(p, "description", description);
        setProp(properties, juce::Identifier(name), p);
    }
    auto root = jsonObject();
    setProp(root, "type", "object");
    setProp(root, "properties", properties);
    setProp(root, "required", juce::var(juce::Array<juce::var>{}));
    return root;
}

const char* kNoteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "A♭", "A", "B♭", "B"};

juce::String pitchName(int noteNumber) {
    static const char* names[] = {"C",  "C#", "D",  "D#", "E",  "F",
                                  "F#", "G",  "G#", "A",  "A#", "B"};
    juce::ignoreUnused(kNoteNames);
    int c = juce::jlimit(0, 127, noteNumber);
    return juce::String(names[c % 12]) + juce::String(c / 12 - 1);
}

/** Resolve a track by 1-based index or by name. Returns -1 when not found. */
int resolveTrack(MagdaApi& api, const juce::String& token) {
    const auto& tracks = api.tracks().getTracks();
    if (token.isEmpty())
        return -1;

    if (token.containsOnly("0123456789")) {
        int idx = token.getIntValue();
        if (idx >= 1 && idx <= static_cast<int>(tracks.size()))
            return tracks[static_cast<size_t>(idx - 1)].id;
        return -1;
    }
    for (const auto& t : tracks) {
        if (t.name.equalsIgnoreCase(token))
            return t.id;
    }
    return -1;
}

juce::String argString(const juce::var& args, const char* key) {
    return args[juce::Identifier(key)].toString().trim();
}

// --- tool implementations -------------------------------------------------

juce::String runListTracks(MagdaApi& api, const juce::var&) {
    juce::Array<juce::var> arr;
    int index = 1;
    for (const auto& t : api.tracks().getTracks()) {
        auto obj = jsonObject();
        setProp(obj, "index", index++);
        setProp(obj, "name", t.name);
        setProp(obj, "clips", static_cast<int>(api.clips().getClipsOnTrack(t.id).size()));

        juce::String chain;
        for (const auto& element : TrackManager::getInstance().getChainElements(t.id)) {
            if (!isDevice(element))
                continue;
            if (chain.isNotEmpty())
                chain += " > ";
            chain += getDevice(element).name;
        }
        setProp(obj, "devices", chain);
        arr.add(obj);
    }
    auto root = jsonObject();
    setProp(root, "tracks", juce::var(arr));
    return toJson(root);
}

juce::String runReadTrack(MagdaApi& api, const juce::var& args) {
    auto token = argString(args, "track");
    int trackId = resolveTrack(api, token);
    if (trackId < 0)
        return errorJson("No track matches '" + token + "'. Use list_tracks to see what exists.");

    auto root = jsonObject();
    if (const auto* info = api.tracks().getTrack(trackId))
        setProp(root, "name", info->name);

    juce::Array<juce::var> devices;
    for (const auto& element : TrackManager::getInstance().getChainElements(trackId)) {
        if (!isDevice(element))
            continue;
        const auto& d = getDevice(element);
        auto obj = jsonObject();
        setProp(obj, "name", d.name);
        setProp(obj, "pluginId", d.pluginId);
        setProp(obj, "isInstrument", d.isInstrument);
        setProp(obj, "bypassed", d.bypassed);
        setProp(obj, "parameterCount", static_cast<int>(d.parameters.size()));
        devices.add(obj);
    }
    setProp(root, "devices", juce::var(devices));

    juce::Array<juce::var> clips;
    int clipIndex = 1;
    for (auto clipId : api.clips().getClipsOnTrack(trackId)) {
        const auto* clip = api.clips().getClip(clipId);
        if (!clip)
            continue;
        auto obj = jsonObject();
        setProp(obj, "index", clipIndex++);
        setProp(obj, "name", clip->name);
        setProp(obj, "startBeat", clip->placement.startBeat);
        setProp(obj, "lengthBeats", clip->placement.lengthBeats);
        setProp(obj, "noteCount", static_cast<int>(clip->midiNotes.size()));
        clips.add(obj);
    }
    setProp(root, "clips", juce::var(clips));
    setProp(root, "hint", "Use read_clip for the actual notes in one of these clips.");
    return toJson(root);
}

juce::String runReadClip(MagdaApi& api, const juce::var& args) {
    auto token = argString(args, "track");
    int trackId = resolveTrack(api, token);
    if (trackId < 0)
        return errorJson("No track matches '" + token + "'. Use list_tracks to see what exists.");

    auto clipIds = api.clips().getClipsOnTrack(trackId);
    if (clipIds.empty())
        return errorJson("Track '" + token + "' has no clips.");

    auto clipToken = argString(args, "clip");
    int wanted = clipToken.isEmpty() ? 1 : clipToken.getIntValue();
    if (wanted < 1 || wanted > static_cast<int>(clipIds.size()))
        return errorJson("Clip " + juce::String(wanted) + " does not exist; this track has "
                         + juce::String(static_cast<int>(clipIds.size())) + ".");

    const auto* clip = api.clips().getClip(clipIds[static_cast<size_t>(wanted - 1)]);
    if (clip == nullptr)
        return errorJson("Clip could not be read.");

    auto sorted = clip->midiNotes;
    std::sort(sorted.begin(), sorted.end(), [](const MidiNote& a, const MidiNote& b) {
        if (a.startBeat != b.startBeat)
            return a.startBeat < b.startBeat;
        return a.noteNumber < b.noteNumber;
    });

    // Cap the response: a dense clip can hold thousands of notes and this text
    // is fed straight back to the model.
    constexpr int kMaxNotes = 500;
    juce::Array<juce::var> notes;
    for (const auto& n : sorted) {
        if (notes.size() >= kMaxNotes)
            break;
        auto obj = jsonObject();
        setProp(obj, "pitch", pitchName(n.noteNumber));
        setProp(obj, "beat", n.startBeat);
        setProp(obj, "length", n.lengthBeats);
        setProp(obj, "velocity", n.velocity);
        notes.add(obj);
    }

    auto root = jsonObject();
    setProp(root, "name", clip->name);
    setProp(root, "startBeat", clip->placement.startBeat);
    setProp(root, "lengthBeats", clip->placement.lengthBeats);
    setProp(root, "noteCount", static_cast<int>(sorted.size()));
    setProp(root, "notes", juce::var(notes));
    if (static_cast<int>(sorted.size()) > kMaxNotes) {
        setProp(root, "truncated",
                juce::String("Only the first ") + juce::String(kMaxNotes)
                    + " notes are listed; do not assume the rest are empty.");
    }
    return toJson(root);
}

juce::String runListInstruments(MagdaApi&, const juce::var& args) {
    auto query = argString(args, "query").toLowerCase();

    juce::Array<juce::var> matches;
    auto add = [&matches](const juce::String& alias, const juce::String& name,
                          const juce::String& category) {
        if (matches.size() >= 60)
            return;
        auto obj = jsonObject();
        setProp(obj, "alias", alias);
        setProp(obj, "name", name);
        setProp(obj, "category", category);
        matches.add(obj);
    };

    for (const auto& inst : audio::getInternalInstruments()) {
        juce::String name(inst.name);
        if (query.isEmpty() || name.toLowerCase().contains(query)
            || juce::String(inst.category).toLowerCase().contains(query))
            add(inst.pluginId, name, inst.category);
    }
    for (const auto& gm : audio::getGMInstruments()) {
        juce::String name(gm.name);
        if (query.isEmpty() || name.toLowerCase().contains(query)
            || juce::String(gm.category).toLowerCase().contains(query))
            add("soundfont:" + juce::String(gm.program), name, gm.category);
    }
    for (const auto& dk : audio::getDrumKits()) {
        juce::String name(dk.name);
        if (query.isEmpty() || name.toLowerCase().contains(query) || query == "drums")
            add("drums:" + juce::String(dk.program), name, "Drums");
    }

    auto root = jsonObject();
    setProp(root, "instruments", juce::var(matches));
    setProp(root, "hint", "Load one with: TRACK FX <alias>");
    return toJson(root);
}

juce::String runListPluginParams(MagdaApi& api, const juce::var& args) {
    auto token = argString(args, "track");
    int trackId = resolveTrack(api, token);
    if (trackId < 0)
        return errorJson("No track matches '" + token + "'. Use list_tracks to see what exists.");

    auto deviceQuery = argString(args, "device").toLowerCase();

    juce::Array<juce::var> devices;
    for (const auto& element : TrackManager::getInstance().getChainElements(trackId)) {
        if (!isDevice(element))
            continue;
        const auto& d = getDevice(element);
        if (deviceQuery.isNotEmpty() && !d.name.toLowerCase().contains(deviceQuery))
            continue;

        juce::Array<juce::var> params;
        for (const auto& p : d.parameters) {
            if (params.size() >= 80)
                break;
            auto obj = jsonObject();
            setProp(obj, "name", p.name);
            setProp(obj, "value", p.currentValue);
            setProp(obj, "min", p.minValue);
            setProp(obj, "max", p.maxValue);
            if (p.unit.isNotEmpty())
                setProp(obj, "unit", p.unit);
            params.add(obj);
        }

        auto obj = jsonObject();
        setProp(obj, "device", d.name);
        setProp(obj, "parameters", juce::var(params));
        devices.add(obj);
    }

    if (devices.isEmpty()) {
        return errorJson(deviceQuery.isEmpty()
                             ? "That track has no plugins."
                             : "No plugin on that track matches '" + deviceQuery + "'.");
    }

    auto root = jsonObject();
    setProp(root, "devices", juce::var(devices));
    setProp(root, "hint",
            "Change one with: PARAM [track] <name>=<value>. Values are in the unit shown, "
            "or write N% for a percentage of the range.");
    return toJson(root);
}

}  // namespace

const std::vector<DawToolRegistry::Tool>& DawToolRegistry::tools() {
    static const std::vector<Tool> kTools = {
        {"list_tracks",
         "List every track with its name, clip count and plugin chain. Start here when you "
         "need to know what the project contains.",
         schema({}), runListTracks},

        {"read_track",
         "Read one track in detail: its plugins and its clips (positions and note counts).",
         schema({{"track", "Track name or 1-based index"}}), runReadTrack},

        {"read_clip",
         "Read the actual notes in one clip. Use this before editing existing music so you "
         "modify what is there instead of replacing it.",
         schema({{"track", "Track name or 1-based index"},
                 {"clip", "1-based clip index on that track (default 1)"}}),
         runReadClip},

        {"list_instruments",
         "Search the available instruments. Returns aliases you can load with TRACK FX.",
         schema({{"query", "Optional filter, e.g. 'piano', 'bass', 'drums'"}}),
         runListInstruments},

        {"list_plugin_params",
         "List a plugin's parameters with their current values and ranges, so PARAM can "
         "target real names instead of guesses.",
         schema({{"track", "Track name or 1-based index"},
                 {"device", "Optional plugin name filter"}}),
         runListPluginParams},
    };
    return kTools;
}

std::vector<llm::ToolDef> DawToolRegistry::toolDefs() {
    std::vector<llm::ToolDef> defs;
    defs.reserve(tools().size());
    for (const auto& t : tools()) {
        llm::ToolDef def;
        def.name = t.name;
        def.description = t.description;
        def.parameters = t.parameters;
        defs.push_back(std::move(def));
    }
    return defs;
}

juce::String DawToolRegistry::dispatch(MagdaApi& api, const juce::String& name,
                                       const juce::String& argumentsJson) {
    for (const auto& t : tools()) {
        if (t.name != name)
            continue;

        auto args = juce::JSON::parse(argumentsJson);
        if (!args.isObject())
            args = jsonObject();  // tolerate "" / "null" / malformed argument blobs

        return t.run(api, args);
    }

    juce::StringArray known;
    for (const auto& t : tools())
        known.add(t.name);
    return errorJson("Unknown tool '" + name + "'. Available: " + known.joinIntoString(", "));
}

}  // namespace magda
