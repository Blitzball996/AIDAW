#pragma once

#include <juce_core/juce_core.h>

#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace magda {

/**
 * @brief Type of clip content.
 */
enum class ClipFileType { Audio, Midi };

/**
 * @brief Musical key signature for clip metadata.
 */
enum class MusicalKey {
    Unknown,
    C,  Cs, D,  Ds, E,  F,  Fs, G,  Gs, A,  As, B,
    Cm, Csm, Dm, Dsm, Em, Fm, Fsm, Gm, Gsm, Am, Asm, Bm
};

/**
 * @brief Metadata entry for a clip file in the library.
 */
struct ClipEntry {
    juce::File path;
    juce::String name;
    ClipFileType type = ClipFileType::Audio;
    double duration = 0.0;       ///< Duration in seconds
    double sampleRate = 0.0;     ///< Sample rate (audio only)
    double bpm = 0.0;            ///< Detected or tagged BPM
    MusicalKey key = MusicalKey::Unknown;
    juce::StringArray tags;
    bool isFavorite = false;
    juce::Time lastAccessed;

    bool isValid() const { return path.existsAsFile(); }
};

/**
 * @brief Listener for clip library changes.
 */
class ClipLibraryListener {
  public:
    virtual ~ClipLibraryListener() = default;
    virtual void clipLibraryScanComplete() = 0;
    virtual void clipLibraryEntryAdded(const ClipEntry& entry) = 0;
};

/**
 * @brief Manages a searchable library of audio and MIDI clip files.
 *
 * Scans configured directories for supported file types, extracts metadata,
 * and provides search/filter capabilities for the browser UI.
 */
class ClipLibrary {
  public:
    ClipLibrary();
    ~ClipLibrary();

    // --- Directory management ---

    /** Add a directory to scan for clips. */
    void addScanDirectory(const juce::File& directory);

    /** Remove a scan directory. */
    void removeScanDirectory(const juce::File& directory);

    /** Get all configured scan directories. */
    std::vector<juce::File> getScanDirectories() const;

    // --- Scanning ---

    /** Scan all configured directories (runs on background thread). */
    void scanAll();

    /** Rescan a single directory. */
    void rescanDirectory(const juce::File& directory);

    /** Check if a scan is currently in progress. */
    bool isScanning() const;

    // --- Search and filter ---

    /** Search clips by name/tag query string. */
    std::vector<ClipEntry> searchClips(const juce::String& query) const;

    /** Filter clips by file type. */
    std::vector<ClipEntry> filterByType(ClipFileType type) const;

    /** Filter clips by BPM range. */
    std::vector<ClipEntry> filterByBpm(double minBpm, double maxBpm) const;

    /** Filter clips by musical key. */
    std::vector<ClipEntry> filterByKey(MusicalKey key) const;

    /** Get all clips (unfiltered). */
    std::vector<ClipEntry> getAllClips() const;

    // --- Favorites ---

    /** Toggle favorite status for a clip. */
    void toggleFavorite(const juce::File& clipPath);

    /** Get all favorited clips. */
    std::vector<ClipEntry> getFavorites() const;

    // --- Recent files ---

    /** Record a file access (adds to recent list). */
    void recordAccess(const juce::File& clipPath);

    /** Get recently accessed clips. */
    std::vector<ClipEntry> getRecentFiles(int maxCount = 20) const;

    // --- Persistence ---

    /** Save library state (favorites, recents) to a file. */
    void saveState(const juce::File& stateFile) const;

    /** Load library state from a file. */
    void loadState(const juce::File& stateFile);

    // --- Listeners ---

    void addListener(ClipLibraryListener* listener);
    void removeListener(ClipLibraryListener* listener);

  private:
    void scanDirectory(const juce::File& directory);
    ClipEntry createEntryFromFile(const juce::File& file) const;
    ClipFileType detectFileType(const juce::File& file) const;
    bool isSupportedFile(const juce::File& file) const;

    mutable std::mutex mutex_;
    std::vector<juce::File> scanDirectories_;
    std::vector<ClipEntry> entries_;
    std::vector<juce::File> favorites_;
    std::vector<std::pair<juce::File, juce::Time>> recentFiles_;
    std::vector<ClipLibraryListener*> listeners_;
    std::atomic<bool> scanning_{false};
};

}  // namespace magda
