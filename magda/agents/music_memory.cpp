#include "music_memory.hpp"

#include "../daw/api/clip_api.hpp"
#include "../daw/api/magda_api.hpp"
#include "../daw/api/track_api.hpp"
#include "../daw/core/AppPaths.hpp"
#include "../daw/core/ClipInfo.hpp"
#include "../daw/core/ClipManager.hpp"
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
    if (!recent.empty()) {
        prompt += "\nCONVERSATION HISTORY:\n";
        for (const auto& entry : recent) {
            prompt += (entry.role == "user" ? "User: " : "Assistant: ");
            auto content = entry.content;
            if (content.size() > 300)
                content = content.substr(0, 300) + "...";
            prompt += content + "\n";
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
                snapshot += std::to_string(static_cast<int>(clip->midiNotes.size())) + " notes";
                if (!clip->midiNotes.empty()) {
                    double minBeat = 9999, maxBeat = 0;
                    int minNote = 127, maxNote = 0;
                    for (const auto& n : clip->midiNotes) {
                        if (n.startBeat < minBeat) minBeat = n.startBeat;
                        if (n.startBeat + n.lengthBeats > maxBeat)
                            maxBeat = n.startBeat + n.lengthBeats;
                        if (n.noteNumber < minNote) minNote = n.noteNumber;
                        if (n.noteNumber > maxNote) maxNote = n.noteNumber;
                    }
                    int bars = static_cast<int>(maxBeat / 4.0) + 1;
                    snapshot += ", " + std::to_string(bars) + " bars";
                    snapshot += ", range " + std::to_string(minNote) + "-" + std::to_string(maxNote);
                }
                snapshot += "\n";
            }
        }
        trackIdx++;
    }

    return snapshot;
}

}  // namespace magda
