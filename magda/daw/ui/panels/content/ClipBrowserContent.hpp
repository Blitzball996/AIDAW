#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <memory>
#include <vector>

#include "PanelContent.hpp"

namespace magda::daw::ui {

/**
 * @brief Clip browser panel for browsing and dragging clips to the timeline.
 *
 * Features:
 * - File tree on left showing project clip folders
 * - Clip list on right showing clips in selected folder
 * - Search bar at top for filtering
 * - Star button for marking favorites
 * - Drag clips to timeline for insertion
 */
class ClipBrowserContent : public PanelContent,
                           public juce::FileBrowserListener {
  public:
    ClipBrowserContent();
    ~ClipBrowserContent() override;

    PanelContentType getContentType() const override { return PanelContentType::Empty; }
    PanelContentInfo getContentInfo() const override {
        return {PanelContentType::Empty, "Clip Browser", "Browse and manage clips", "Clip"};
    }

    void paint(juce::Graphics& g) override;
    void resized() override;

    void onActivated() override;
    void onDeactivated() override;

    // Set the root directory for clip browsing
    void setRootDirectory(const juce::File& directory);

    // FileBrowserListener
    void selectionChanged() override;
    void fileClicked(const juce::File& file, const juce::MouseEvent& e) override;
    void fileDoubleClicked(const juce::File& file) override;
    void browserRootChanged(const juce::File& newRoot) override;

    // Callbacks
    std::function<void(const juce::File& file)> onClipSelected;
    std::function<void(const juce::File& file)> onClipDoubleClicked;
    std::function<void(const juce::File& file, bool isFavorite)> onFavoriteToggled;

  private:
    // Search
    juce::TextEditor searchBox_;
    juce::Label searchLabel_;

    // File browser
    std::unique_ptr<juce::FileFilter> clipFilter_;
    std::unique_ptr<juce::FileBrowserComponent> fileBrowser_;

    // Favorites
    juce::TextButton favoriteButton_;
    std::vector<juce::File> favorites_;

    // Clip info
    juce::Label clipInfoLabel_;
    juce::Label clipDurationLabel_;

    // Filter state
    juce::String searchTerm_;

    void updateSearch();
    void updateClipInfo(const juce::File& file);
    bool isClipFile(const juce::File& file) const;
    bool isFavorite(const juce::File& file) const;
    void toggleFavorite(const juce::File& file);

    // Drag support
    void mouseDrag(const juce::MouseEvent& e) override;
    juce::File dragFile_;
    juce::Point<int> dragStartPos_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipBrowserContent)
};

}  // namespace magda::daw::ui
