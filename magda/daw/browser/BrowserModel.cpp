#include "BrowserModel.hpp"

namespace magda {

BrowserModel::BrowserModel(ClipLibrary& library) : library_(library) {}
BrowserModel::~BrowserModel() = default;

void BrowserModel::refresh() {
    roots_.clear();
    for (const auto& dir : library_.getScanDirectories()) {
        auto node = std::make_unique<BrowserNode>();
        node->name = dir.getFileName();
        node->path = dir;
        node->isDirectory = true;
        roots_.push_back(std::move(node));
    }
}

const std::vector<std::unique_ptr<BrowserNode>>& BrowserModel::getRoots() const {
    return roots_;
}

void BrowserModel::expandNode(BrowserNode& node) {
    if (!node.isDirectory) return;
    node.isExpanded = true;
    if (node.children.empty()) buildChildren(node);
}

void BrowserModel::collapseNode(BrowserNode& node) {
    node.isExpanded = false;
}

std::vector<ClipEntry> BrowserModel::getClipsInNode(
    const BrowserNode& node) const {
    if (!node.isDirectory) return {};

    auto allClips = library_.getAllClips();
    std::vector<ClipEntry> results;
    for (const auto& clip : allClips) {
        if (clip.path.isAChildOf(node.path) ||
            clip.path.getParentDirectory() == node.path)
            results.push_back(clip);
    }
    return results;
}

BrowserDragData BrowserModel::createDragData(
    const std::vector<ClipEntry>& selectedClips,
    juce::Point<int> offset) const {
    return BrowserDragData{selectedClips, offset};
}

void BrowserModel::buildChildren(BrowserNode& node) {
    if (!node.path.isDirectory()) return;

    // Add subdirectories first
    for (const auto& entry : juce::RangedDirectoryIterator(
             node.path, false, "*", juce::File::findDirectories)) {
        auto child = std::make_unique<BrowserNode>();
        child->name = entry.getFile().getFileName();
        child->path = entry.getFile();
        child->isDirectory = true;
        child->parent = &node;
        node.children.push_back(std::move(child));
    }

    // Add files
    for (const auto& entry : juce::RangedDirectoryIterator(
             node.path, false, "*", juce::File::findFiles)) {
        auto file = entry.getFile();
        auto ext = file.getFileExtension().toLowerCase();
        // Only show supported audio/MIDI files
        if (ext == ".wav" || ext == ".aiff" || ext == ".aif" || ext == ".flac" ||
            ext == ".mp3" || ext == ".ogg" || ext == ".m4a" || ext == ".mid" ||
            ext == ".midi" || ext == ".smf") {
            auto child = std::make_unique<BrowserNode>();
            child->name = file.getFileName();
            child->path = file;
            child->isDirectory = false;
            child->parent = &node;
            node.children.push_back(std::move(child));
        }
    }
}

}  // namespace magda
