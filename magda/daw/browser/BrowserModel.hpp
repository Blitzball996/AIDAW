#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>
#include <vector>

#include "ClipLibrary.hpp"

namespace magda {

/**
 * @brief A node in the browser folder tree.
 */
struct BrowserNode {
    juce::String name;
    juce::File path;
    bool isDirectory = false;
    bool isExpanded = false;
    std::vector<std::unique_ptr<BrowserNode>> children;
    BrowserNode* parent = nullptr;

    bool hasChildren() const { return !children.empty(); }
    int getDepth() const {
        int depth = 0;
        auto* p = parent;
        while (p) {
            ++depth;
            p = p->parent;
        }
        return depth;
    }
};

/**
 * @brief Data payload for drag-and-drop of clips from the browser.
 */
struct BrowserDragData {
    std::vector<ClipEntry> clips;
    juce::Point<int> dragOffset;

    /** Create a JUCE var representation for drag-and-drop. */
    juce::var toVar() const {
        juce::var result;
        for (const auto& clip : clips)
            result.append(clip.path.getFullPathName());
        return result;
    }

    /** Reconstruct from a JUCE var (from drop target). */
    static std::vector<juce::File> fromVar(const juce::var& data) {
        std::vector<juce::File> files;
        if (auto* arr = data.getArray()) {
            for (const auto& item : *arr)
                files.emplace_back(item.toString());
        }
        return files;
    }
};

/**
 * @brief Tree model for the clip browser folder navigation.
 *
 * Provides a hierarchical view of scan directories and their contents,
 * with support for lazy loading and drag-and-drop.
 */
class BrowserModel {
  public:
    explicit BrowserModel(ClipLibrary& library);
    ~BrowserModel();

    /** Rebuild the tree from the library's scan directories. */
    void refresh();

    /** Get the root nodes (one per scan directory). */
    const std::vector<std::unique_ptr<BrowserNode>>& getRoots() const;

    /** Expand a directory node (lazy-loads children). */
    void expandNode(BrowserNode& node);

    /** Collapse a directory node. */
    void collapseNode(BrowserNode& node);

    /** Get clips contained in a directory node. */
    std::vector<ClipEntry> getClipsInNode(const BrowserNode& node) const;

    /** Create drag data for selected clips. */
    BrowserDragData createDragData(const std::vector<ClipEntry>& selectedClips,
                                   juce::Point<int> offset = {}) const;

  private:
    void buildChildren(BrowserNode& node);

    ClipLibrary& library_;
    std::vector<std::unique_ptr<BrowserNode>> roots_;
};

}  // namespace magda
