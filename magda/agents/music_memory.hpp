#pragma once

#include <juce_core/juce_core.h>

#include <string>
#include <vector>

namespace magda {

class MagdaApi;

struct MusicMemoryEntry {
    std::string role;      // "user" or "assistant"
    std::string content;
    int64_t timestamp = 0;
};

struct MusicContext {
    std::string key;              // e.g. "C major", "D minor"
    std::string style;            // e.g. "jazz", "pop", "EDM"
    std::string tempo;            // e.g. "120 BPM"
    std::string chordProgression; // last used progression
    std::string notes;            // free-form musical notes
};

class MusicMemory {
  public:
    static MusicMemory& getInstance();

    void setProjectId(const std::string& projectPath);

    void addEntry(const std::string& role, const std::string& content);
    std::vector<MusicMemoryEntry> getRecentEntries(int count = 20) const;
    void clear();

    void setContext(const MusicContext& ctx);
    MusicContext getContext() const;

    std::string buildContextPrompt(int maxEntries = 20) const;

    static std::string buildDAWSnapshot(MagdaApi& api);

  private:
    MusicMemory() = default;

    void load();
    void save();

    juce::File getMemoryFile() const;

    std::string projectId_;
    std::vector<MusicMemoryEntry> entries_;
    MusicContext context_;
};

}  // namespace magda
