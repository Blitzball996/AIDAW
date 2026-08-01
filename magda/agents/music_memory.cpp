#include "music_memory.hpp"

#include <algorithm>

#include "../daw/api/clip_api.hpp"
#include "../daw/api/magda_api.hpp"
#include "../daw/api/track_api.hpp"
#include "../daw/core/AppPaths.hpp"
#include "../daw/core/ClipInfo.hpp"
#include "../daw/core/ClipManager.hpp"
#include "../daw/core/RackInfo.hpp"
#include "../daw/core/TrackManager.hpp"

namespace magda {

MusicMemory& MusicMemory::getInstance() {
    static MusicMemory instance;
    return instance;
}

void MusicMemory::setProjectId(const std::string& projectPath) {
    if (projectId_ == projectPath)
        return;
    projectId_ = projectPath;
    load();
}

void MusicMemory::addEntry(const std::string& role, const std::string& content) {
    MusicMemoryEntry entry;
    entry.role = role;
    entry.content = content;
    entry.timestamp = juce::Time::currentTimeMillis();
    entries_.push_back(std::move(entry));

    if (entries_.size() > 100)
        entries_.erase(entries_.begin(), entries_.begin() + 50);

    save();
}

std::vector<MusicMemoryEntry> MusicMemory::getRecentEntries(int count) const {
    if (static_cast<int>(entries_.size()) <= count)
        return entries_;
    return {entries_.end() - count, entries_.end()};
}

void MusicMemory::clear() {
    entries_.clear();
    context_ = {};
    save();
}

void MusicMemory::setContext(const MusicContext& ctx) {
    context_ = ctx;
    save();
}

MusicContext MusicMemory::getContext() const {
    return context_;
}

std::string MusicMemory::buildContextPrompt(int maxEntries) const {
    std::string prompt;

    // DAW state snapshot (tracks + MIDI content summary)
    auto& tm = magda::TrackManager::getInstance();
    auto& cm = magda::ClipManager::getInstance();
    auto tracks = tm.getTracks();
    if (!tracks.empty()) {
        prompt += "\n\nCURRENT PROJECT STATE:\n";
        prompt += "Tracks (" + std::to_string(tracks.size()) + "):\n";
        int trackIdx = 1;
        for (const auto& track : tracks) {
            prompt += "  " + std::to_string(trackIdx) + ". " + track.name.toStdString();
            auto clipIds = cm.getClipsOnTrack(track.id);
            if (clipIds.empty()) {
                prompt += " (no clips)\n";
            } else {
                prompt += " (" + std::to_string(clipIds.size()) + " clips):\n";
                for (auto clipId : clipIds) {
                    auto* clip = cm.getClip(clipId);
                    if (!clip) continue;
                    prompt += "    - ";
                    if (!clip->name.isEmpty())
                        prompt += clip->name.toStdString() + ": ";
                    prompt += std::to_string(static_cast<int>(clip->midiNotes.size())) + " notes";
                    if (!clip->midiNotes.empty()) {
                        double maxBeat = 0;
                        for (const auto& n : clip->midiNotes)
                            if (n.startBeat + n.lengthBeats > maxBeat)
                                maxBeat = n.startBeat + n.lengthBeats;
                        int bars = static_cast<int>(maxBeat / 4.0) + 1;
                        prompt += ", " + std::to_string(bars) + " bars";
                    }
                    prompt += "\n";
                }
            }
            trackIdx++;
        }
    }

    if (!context_.key.empty() || !context_.style.empty() || !context_.tempo.empty()) {
        prompt += "\nMUSICAL CONTEXT:\n";
        if (!context_.key.empty())
            prompt += "- Key: " + context_.key + "\n";
        if (!context_.style.empty())
            prompt += "- Style: " + context_.style + "\n";
        if (!context_.tempo.empty())
            prompt += "- Tempo: " + context_.tempo + "\n";
        if (!context_.chordProgression.empty())
            prompt += "- Last progression: " + context_.chordProgression + "\n";
        if (!context_.notes.empty())
            prompt += "- Notes: " + context_.notes + "\n";
    }

    auto recent = getRecentEntries(maxEntries);
    // Conversation history. Keep this SHORT and CLEAN: long, noisy history
    // (execution-result text, timing tags, multi-line errors) drags the model
    // off the strict DSL format after a few turns and it stops emitting notes.
    if (!recent.empty()) {
        prompt +=
            "\nRECENT CONVERSATION (most recent last; context only -- always "
            "answer in the required DSL format):\n";
        for (const auto& entry : recent) {
            juce::String content = juce::String(entry.content);

            // Drop the trailing "[123ms router, ...]" timing annotation.
            int tagPos = content.lastIndexOf("[");
            if (tagPos > 0 && content.substring(tagPos).containsIgnoreCase("router"))
                content = content.substring(0, tagPos);

            // Collapse newlines and trim.
            content = content.replaceCharacters("\r\n", "  ").trim();

            // Skip pure error/warning assistant turns -- not good exemplars.
            if (entry.role == "assistant" &&
                (content.startsWithIgnoreCase("error") || content.startsWith("[!]") ||
                 content.startsWithIgnoreCase("[warning]") || content.isEmpty()))
                continue;

            if (content.length() > 160)
                content = content.substring(0, 160) + "...";

            prompt += (entry.role == "user" ? "User: " : "Assistant: ");
            prompt += content.toStdString() + "\n";
        }
    }

    return prompt;
}

juce::File MusicMemory::getMemoryFile() const {
    if (projectId_.empty())
        return {};
    auto hash = juce::String(projectId_).hashCode64();
    auto filename = "music_memory_" + juce::String(hash) + ".json";
    return magda::paths::dataDir().getChildFile(filename);
}

void MusicMemory::load() {
    entries_.clear();
    context_ = {};

    auto file = getMemoryFile();
    if (!file.existsAsFile())
        return;

    auto text = file.loadFileAsString();
    auto json = juce::JSON::parse(text);
    if (json.isVoid())
        return;

    if (auto* ctx = json["context"].getDynamicObject()) {
        context_.key = ctx->getProperty("key").toString().toStdString();
        context_.style = ctx->getProperty("style").toString().toStdString();
        context_.tempo = ctx->getProperty("tempo").toString().toStdString();
        context_.chordProgression = ctx->getProperty("chordProgression").toString().toStdString();
        context_.notes = ctx->getProperty("notes").toString().toStdString();
    }

    if (auto* arr = json["entries"].getArray()) {
        for (const auto& item : *arr) {
            MusicMemoryEntry entry;
            entry.role = item["role"].toString().toStdString();
            entry.content = item["content"].toString().toStdString();
            entry.timestamp = static_cast<int64_t>(static_cast<int>(item["timestamp"]));
            entries_.push_back(std::move(entry));
        }
    }
}

void MusicMemory::save() {
    auto file = getMemoryFile();
    if (file == juce::File())
        return;

    auto* root = new juce::DynamicObject();

    auto* ctx = new juce::DynamicObject();
    ctx->setProperty("key", juce::String(context_.key));
    ctx->setProperty("style", juce::String(context_.style));
    ctx->setProperty("tempo", juce::String(context_.tempo));
    ctx->setProperty("chordProgression", juce::String(context_.chordProgression));
    ctx->setProperty("notes", juce::String(context_.notes));
    root->setProperty("context", juce::var(ctx));

    juce::Array<juce::var> arr;
    for (const auto& entry : entries_) {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("role", juce::String(entry.role));
        obj->setProperty("content", juce::String(entry.content));
        obj->setProperty("timestamp", static_cast<int>(entry.timestamp));
        arr.add(juce::var(obj));
    }
    root->setProperty("entries", arr);

    file.getParentDirectory().createDirectory();
    file.replaceWithText(juce::JSON::toString(juce::var(root)));
}

std::string MusicMemory::buildDAWSnapshot(MagdaApi& api) {
    auto& tm = api.tracks();
    auto& cm = api.clips();

    // Budget guard: a busy project can hold thousands of notes and this text
    // goes into every request. Emit notes until the cap, then summarise the
    // rest so the model is told it is seeing a partial view rather than
    // silently concluding those bars are empty.
    constexpr int kMaxNotesEmitted = 400;
    int notesEmitted = 0;
    int notesOmitted = 0;

    static const char* kNames[] = {"C",  "C#", "D",  "D#", "E",  "F",
                                   "F#", "G",  "G#", "A",  "A#", "B"};
    auto pitchName = [](int noteNumber) {
        int clamped = std::clamp(noteNumber, 0, 127);
        return std::string(kNames[clamped % 12]) + std::to_string(clamped / 12 - 1);
    };
    auto num = [](double v) {
        // Compact: drop a trailing ".0" so beats read as 0, 1.5, 4 not 0.000000.
        auto s = juce::String(v, 3).trimCharactersAtEnd("0").trimCharactersAtEnd(".");
        return (s.isEmpty() ? juce::String("0") : s).toStdString();
    };

    std::string snapshot = "\n\nCURRENT PROJECT STATE:\n";
    auto tracks = tm.getTracks();
    if (tracks.empty()) {
        snapshot += "(empty project - no tracks)\n";
        return snapshot;
    }

    snapshot += "Tracks (" + std::to_string(tracks.size()) + "):\n";
    int trackIdx = 1;
    for (const auto& track : tracks) {
        snapshot += "  " + std::to_string(trackIdx) + ". " + track.name.toStdString();

        // Instrument / FX chain, so the model can see what it is writing for
        // and can target PARAM at the right device.
        {
            const auto& elements = TrackManager::getInstance().getChainElements(track.id);
            std::string chain;
            for (const auto& element : elements) {
                if (!isDevice(element))
                    continue;
                const auto& device = getDevice(element);
                if (!chain.empty())
                    chain += " > ";
                chain += device.name.toStdString();
                if (device.bypassed)
                    chain += "(bypassed)";
            }
            if (!chain.empty())
                snapshot += " [" + chain + "]";
        }

        auto clipIds = cm.getClipsOnTrack(track.id);
        if (clipIds.empty()) {
            snapshot += " (no clips)\n";
        } else {
            snapshot += " (" + std::to_string(clipIds.size()) + " clips):\n";
            for (auto clipId : clipIds) {
                auto* clip = cm.getClip(clipId);
                if (!clip)
                    continue;
                snapshot += "    - ";
                if (!clip->name.isEmpty())
                    snapshot += clip->name.toStdString() + ": ";
                snapshot += "startBeat=" + num(clip->placement.startBeat)
                            + " lengthBeats=" + num(clip->placement.lengthBeats) + ", "
                            + std::to_string(static_cast<int>(clip->midiNotes.size())) + " notes";

                if (!clip->midiNotes.empty()) {
                    // Actual note content, not just a count — editing an
                    // existing part is impossible without knowing what is
                    // already there. Format: pitch@startBeat:lengthBeats,vel
                    auto sorted = clip->midiNotes;
                    std::sort(sorted.begin(), sorted.end(),
                              [](const MidiNote& a, const MidiNote& b) {
                                  if (a.startBeat != b.startBeat)
                                      return a.startBeat < b.startBeat;
                                  return a.noteNumber < b.noteNumber;
                              });

                    snapshot += "\n      ";
                    bool first = true;
                    for (const auto& n : sorted) {
                        if (notesEmitted >= kMaxNotesEmitted) {
                            ++notesOmitted;
                            continue;
                        }
                        if (!first)
                            snapshot += " ";
                        snapshot += pitchName(n.noteNumber) + "@" + num(n.startBeat) + ":"
                                    + num(n.lengthBeats) + "," + std::to_string(n.velocity);
                        first = false;
                        ++notesEmitted;
                    }
                }
                snapshot += "\n";
            }
        }
        trackIdx++;
    }

    if (notesOmitted > 0) {
        snapshot += "(" + std::to_string(notesOmitted)
                    + " further notes not shown - ask before assuming those bars are empty)\n";
    }

    snapshot += "Note format: pitch@startBeat:lengthBeats,velocity (beats are "
                "relative to the clip start).\n";

    return snapshot;
}

}  // namespace magda
