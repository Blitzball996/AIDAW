#include "ClipBrowserContent.hpp"

#include "../../themes/DarkTheme.hpp"
#include "../../themes/FontManager.hpp"

namespace magda::daw::ui {

ClipBrowserContent::ClipBrowserContent() {
    // Search box
    searchBox_.setTextToShowWhenEmpty("Search clips...", juce::Colours::grey);
    searchBox_.onTextChange = [this]() { updateSearch(); };
    addAndMakeVisible(searchBox_);

    // Favorite button
    favoriteButton_.setButtonText("*");
    favoriteButton_.setTooltip("Toggle favorite");
    favoriteButton_.onClick = [this]() {
        if (fileBrowser_) {
            auto selected = fileBrowser_->getSelectedFile(0);
            if (selected.existsAsFile()) toggleFavorite(selected);
        }
    };
    addAndMakeVisible(favoriteButton_);

    // Clip info labels
    clipInfoLabel_.setFont(FontManager::getInstance().getUIFont(11.0f));
    clipInfoLabel_.setColour(juce::Label::textColourId,
                             DarkTheme::getColour(DarkTheme::TEXT_SECONDARY));
    addAndMakeVisible(clipInfoLabel_);

    clipDurationLabel_.setFont(FontManager::getInstance().getUIFont(11.0f));
    clipDurationLabel_.setColour(juce::Label::textColourId,
                                 DarkTheme::getColour(DarkTheme::TEXT_SECONDARY));
    addAndMakeVisible(clipDurationLabel_);

    // File filter for clips
    clipFilter_ = std::make_unique<juce::WildcardFileFilter>(
        "*.wav;*.aiff;*.flac;*.mp3;*.ogg;*.mid;*.midi;*.magdaclip",
        "*", "Audio and MIDI clips");

    // File browser
    auto defaultDir = juce::File::getSpecialLocation(juce::File::userMusicDirectory);
    fileBrowser_ = std::make_unique<juce::FileBrowserComponent>(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        defaultDir, clipFilter_.get(), nullptr);
    fileBrowser_->addListener(this);
    addAndMakeVisible(*fileBrowser_);
}

ClipBrowserContent::~ClipBrowserContent() {
    if (fileBrowser_) fileBrowser_->removeListener(this);
}

void ClipBrowserContent::paint(juce::Graphics& g) {
    g.fillAll(DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND));
}

void ClipBrowserContent::resized() {
    auto bounds = getLocalBounds();

    // Top bar: search + favorite
    auto topBar = bounds.removeFromTop(28).reduced(4, 2);
    favoriteButton_.setBounds(topBar.removeFromRight(28));
    topBar.removeFromRight(4);
    searchBox_.setBounds(topBar);

    // Bottom info bar
    auto infoBar = bounds.removeFromBottom(20).reduced(4, 0);
    clipInfoLabel_.setBounds(infoBar.removeFromLeft(infoBar.getWidth() / 2));
    clipDurationLabel_.setBounds(infoBar);

    // File browser fills the rest
    if (fileBrowser_)
        fileBrowser_->setBounds(bounds);
}

void ClipBrowserContent::onActivated() {
    // Refresh browser when panel becomes active
}

void ClipBrowserContent::onDeactivated() {}

void ClipBrowserContent::setRootDirectory(const juce::File& directory) {
    if (fileBrowser_ && directory.isDirectory())
        fileBrowser_->setRoot(directory);
}

void ClipBrowserContent::selectionChanged() {
    if (!fileBrowser_) return;
    auto file = fileBrowser_->getSelectedFile(0);
    if (file.existsAsFile()) {
        updateClipInfo(file);
        if (onClipSelected) onClipSelected(file);
    }
}

void ClipBrowserContent::fileClicked(const juce::File& file, const juce::MouseEvent& e) {
    if (file.existsAsFile()) {
        dragFile_ = file;
        dragStartPos_ = e.getPosition();
    }
}

void ClipBrowserContent::fileDoubleClicked(const juce::File& file) {
    if (onClipDoubleClicked) onClipDoubleClicked(file);
}

void ClipBrowserContent::browserRootChanged(const juce::File&) {}

void ClipBrowserContent::mouseDrag(const juce::MouseEvent& e) {
    if (dragFile_.existsAsFile()) {
        auto dist = e.getPosition().getDistanceFrom(dragStartPos_);
        if (dist > 5.0f) {
            juce::DragAndDropContainer* container =
                juce::DragAndDropContainer::findParentDragContainerFor(this);
            if (container) {
                juce::StringArray files;
                files.add(dragFile_.getFullPathName());
                container->performExternalDragDropOfFiles(files, false);
                dragFile_ = juce::File();
            }
        }
    }
}

void ClipBrowserContent::updateSearch() {
    searchTerm_ = searchBox_.getText().toLowerCase();
    // The JUCE FileBrowserComponent doesn't support dynamic filtering easily,
    // so we'd need a custom list model for full search. For now, the wildcard
    // filter handles file type filtering.
}

void ClipBrowserContent::updateClipInfo(const juce::File& file) {
    clipInfoLabel_.setText(file.getFileName(), juce::dontSendNotification);

    auto size = file.getSize();
    juce::String sizeStr;
    if (size > 1024 * 1024)
        sizeStr = juce::String(size / (1024.0 * 1024.0), 1) + " MB";
    else if (size > 1024)
        sizeStr = juce::String(size / 1024.0, 1) + " KB";
    else
        sizeStr = juce::String(size) + " B";

    clipDurationLabel_.setText(sizeStr, juce::dontSendNotification);
}

bool ClipBrowserContent::isClipFile(const juce::File& file) const {
    auto ext = file.getFileExtension().toLowerCase();
    return ext == ".wav" || ext == ".aiff" || ext == ".flac" || ext == ".mp3" ||
           ext == ".ogg" || ext == ".mid" || ext == ".midi" || ext == ".magdaclip";
}

bool ClipBrowserContent::isFavorite(const juce::File& file) const {
    for (auto& f : favorites_)
        if (f == file) return true;
    return false;
}

void ClipBrowserContent::toggleFavorite(const juce::File& file) {
    bool wasFav = isFavorite(file);
    if (wasFav) {
        favorites_.erase(std::remove(favorites_.begin(), favorites_.end(), file),
                         favorites_.end());
    } else {
        favorites_.push_back(file);
    }
    if (onFavoriteToggled) onFavoriteToggled(file, !wasFav);
}

}  // namespace magda::daw::ui
