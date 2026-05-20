#include "ClipLibrary.hpp"

#include <juce_audio_formats/juce_audio_formats.h>

#include <algorithm>

namespace magda {

namespace {

const juce::StringArray kAudioExtensions{".wav", ".aiff", ".aif", ".flac",
                                         ".mp3", ".ogg", ".m4a"};
const juce::StringArray kMidiExtensions{".mid", ".midi", ".smf"};

}  // namespace

ClipLibrary::ClipLibrary() = default;
ClipLibrary::~ClipLibrary() = default;

void ClipLibrary::addScanDirectory(const juce::File& directory) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (std::find(scanDirectories_.begin(), scanDirectories_.end(), directory) ==
        scanDirectories_.end())
        scanDirectories_.push_back(directory);
}

void ClipLibrary::removeScanDirectory(const juce::File& directory) {
    std::lock_guard<std::mutex> lock(mutex_);
    scanDirectories_.erase(
        std::remove(scanDirectories_.begin(), scanDirectories_.end(), directory),
        scanDirectories_.end());
}

std::vector<juce::File> ClipLibrary::getScanDirectories() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return scanDirectories_;
}

void ClipLibrary::scanAll() {
    scanning_ = true;
    std::vector<juce::File> dirs;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        dirs = scanDirectories_;
        entries_.clear();
    }

    for (const auto& dir : dirs)
        scanDirectory(dir);

    scanning_ = false;

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto* l : listeners_)
        l->clipLibraryScanComplete();
}

void ClipLibrary::rescanDirectory(const juce::File& directory) {
    scanning_ = true;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        entries_.erase(
            std::remove_if(entries_.begin(), entries_.end(),
                           [&](const ClipEntry& e) {
                               return e.path.getParentDirectory() == directory ||
                                      e.path.isAChildOf(directory);
                           }),
            entries_.end());
    }
    scanDirectory(directory);
    scanning_ = false;
}

bool ClipLibrary::isScanning() const { return scanning_.load(); }

std::vector<ClipEntry> ClipLibrary::searchClips(const juce::String& query) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (query.isEmpty()) return entries_;

    auto lowerQuery = query.toLowerCase();
    std::vector<ClipEntry> results;
    for (const auto& entry : entries_) {
        if (entry.name.toLowerCase().contains(lowerQuery)) {
            results.push_back(entry);
            continue;
        }
        for (const auto& tag : entry.tags) {
            if (tag.toLowerCase().contains(lowerQuery)) {
                results.push_back(entry);
                break;
            }
        }
    }
    return results;
}

std::vector<ClipEntry> ClipLibrary::filterByType(ClipFileType type) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ClipEntry> results;
    for (const auto& entry : entries_)
        if (entry.type == type) results.push_back(entry);
    return results;
}

std::vector<ClipEntry> ClipLibrary::filterByBpm(double minBpm,
                                                double maxBpm) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ClipEntry> results;
    for (const auto& entry : entries_)
        if (entry.bpm >= minBpm && entry.bpm <= maxBpm)
            results.push_back(entry);
    return results;
}

std::vector<ClipEntry> ClipLibrary::filterByKey(MusicalKey key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ClipEntry> results;
    for (const auto& entry : entries_)
        if (entry.key == key) results.push_back(entry);
    return results;
}

std::vector<ClipEntry> ClipLibrary::getAllClips() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_;
}

void ClipLibrary::toggleFavorite(const juce::File& clipPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find(favorites_.begin(), favorites_.end(), clipPath);
    if (it != favorites_.end()) {
        favorites_.erase(it);
        for (auto& entry : entries_)
            if (entry.path == clipPath) entry.isFavorite = false;
    } else {
        favorites_.push_back(clipPath);
        for (auto& entry : entries_)
            if (entry.path == clipPath) entry.isFavorite = true;
    }
}

std::vector<ClipEntry> ClipLibrary::getFavorites() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ClipEntry> results;
    for (const auto& entry : entries_)
        if (entry.isFavorite) results.push_back(entry);
    return results;
}

void ClipLibrary::recordAccess(const juce::File& clipPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    // Remove existing entry if present
    recentFiles_.erase(
        std::remove_if(recentFiles_.begin(), recentFiles_.end(),
                       [&](const auto& p) { return p.first == clipPath; }),
        recentFiles_.end());
    recentFiles_.insert(recentFiles_.begin(),
                        {clipPath, juce::Time::getCurrentTime()});
    // Cap at 50 entries
    if (recentFiles_.size() > 50) recentFiles_.resize(50);
}

std::vector<ClipEntry> ClipLibrary::getRecentFiles(int maxCount) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ClipEntry> results;
    int count = 0;
    for (const auto& [path, time] : recentFiles_) {
        if (count >= maxCount) break;
        for (const auto& entry : entries_) {
            if (entry.path == path) {
                results.push_back(entry);
                ++count;
                break;
            }
        }
    }
    return results;
}

void ClipLibrary::saveState(const juce::File& stateFile) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto state = std::make_unique<juce::DynamicObject>();
    juce::var favArray;
    for (const auto& f : favorites_)
        favArray.append(f.getFullPathName());
    state->setProperty("favorites", favArray);

    juce::var recentArray;
    for (const auto& [path, time] : recentFiles_) {
        auto item = std::make_unique<juce::DynamicObject>();
        item->setProperty("path", path.getFullPathName());
        item->setProperty("time", juce::String(time.toMilliseconds()));
        recentArray.append(juce::var(item.release()));
    }
    state->setProperty("recent", recentArray);

    juce::var dirsArray;
    for (const auto& d : scanDirectories_)
        dirsArray.append(d.getFullPathName());
    state->setProperty("directories", dirsArray);

    stateFile.replaceWithText(juce::JSON::toString(juce::var(state.release())));
}

void ClipLibrary::loadState(const juce::File& stateFile) {
    if (!stateFile.existsAsFile()) return;

    auto json = juce::JSON::parse(stateFile.loadFileAsString());
    if (json.isVoid()) return;

    std::lock_guard<std::mutex> lock(mutex_);

    if (auto* favs = json.getProperty("favorites", {}).getArray()) {
        favorites_.clear();
        for (const auto& f : *favs)
            favorites_.emplace_back(f.toString());
    }

    if (auto* recents = json.getProperty("recent", {}).getArray()) {
        recentFiles_.clear();
        for (const auto& item : *recents) {
            auto path = juce::File(item.getProperty("path", "").toString());
            auto ms = item.getProperty("time", "0").toString().getLargeIntValue();
            recentFiles_.emplace_back(path, juce::Time(ms));
        }
    }

    if (auto* dirs = json.getProperty("directories", {}).getArray()) {
        scanDirectories_.clear();
        for (const auto& d : *dirs)
            scanDirectories_.emplace_back(d.toString());
    }
}

void ClipLibrary::addListener(ClipLibraryListener* listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.push_back(listener);
}

void ClipLibrary::removeListener(ClipLibraryListener* listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.erase(
        std::remove(listeners_.begin(), listeners_.end(), listener),
        listeners_.end());
}

// --- Private ---

void ClipLibrary::scanDirectory(const juce::File& directory) {
    if (!directory.isDirectory()) return;

    for (const auto& entry :
         juce::RangedDirectoryIterator(directory, true, "*", juce::File::findFiles)) {
        auto file = entry.getFile();
        if (!isSupportedFile(file)) continue;

        auto clipEntry = createEntryFromFile(file);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            clipEntry.isFavorite =
                std::find(favorites_.begin(), favorites_.end(), file) !=
                favorites_.end();
            entries_.push_back(clipEntry);
        }

        std::lock_guard<std::mutex> lock(mutex_);
        for (auto* l : listeners_)
            l->clipLibraryEntryAdded(clipEntry);
    }
}

ClipEntry ClipLibrary::createEntryFromFile(const juce::File& file) const {
    ClipEntry entry;
    entry.path = file;
    entry.name = file.getFileNameWithoutExtension();
    entry.type = detectFileType(file);

    if (entry.type == ClipFileType::Audio) {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        if (auto reader =
                std::unique_ptr<juce::AudioFormatReader>(
                    formatManager.createReaderFor(file))) {
            entry.sampleRate = reader->sampleRate;
            entry.duration =
                static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
            if (reader->metadataValues.containsKey("tempo"))
                entry.bpm = reader->metadataValues["tempo"].getDoubleValue();
        }
    }

    return entry;
}

ClipFileType ClipLibrary::detectFileType(const juce::File& file) const {
    auto ext = file.getFileExtension().toLowerCase();
    if (kMidiExtensions.contains(ext)) return ClipFileType::Midi;
    return ClipFileType::Audio;
}

bool ClipLibrary::isSupportedFile(const juce::File& file) const {
    auto ext = file.getFileExtension().toLowerCase();
    return kAudioExtensions.contains(ext) || kMidiExtensions.contains(ext);
}

}  // namespace magda
