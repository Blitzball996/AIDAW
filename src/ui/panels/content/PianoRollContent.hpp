#pragma once

#include <functional>
#include <memory>

#include "MidiEditorContent.hpp"
#include "core/SelectionManager.hpp"

namespace aidaw {
class PianoRollGridComponent;
class PianoRollKeyboard;
class SvgButton;
class MidiBridge;
struct MidiNoteEvent;
}  // namespace aidaw

namespace aidaw::daw::ui {

/**
 * @brief Piano roll editor for MIDI clips
 *
 * Displays MIDI notes in a piano roll grid layout:
 * - Keyboard on the left showing note names
 * - Note rectangles in the grid representing MIDI notes (interactive)
 * - Time ruler along the top (switchable between absolute/relative)
 */
class PianoRollContent : public MidiEditorContent, public aidaw::SelectionManagerListener {
  public:
    PianoRollContent();
    ~PianoRollContent() override;

    PanelContentType getContentType() const override {
        return PanelContentType::PianoRoll;
    }

    PanelContentInfo getContentInfo() const override {
        return {PanelContentType::PianoRoll, "Piano Roll", "MIDI note editor", "PianoRoll"};
    }

    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;
    void resized() override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    void onActivated() override;
    void onDeactivated() override;

    bool wantsHeader() const override {
        return true;
    }

    int getOptimalPanelHeight(int windowHeight) const override {
        return windowHeight * 2 / 3;
    }

    // ClipManagerListener overrides
    void clipsChanged() override;
    void clipPropertyChanged(aidaw::ClipId clipId) override;
    void clipSelectionChanged(aidaw::ClipId clipId) override;
    void clipDragPreview(aidaw::ClipId clipId, double previewStartTime,
                         double previewLength) override;

    // SelectionManagerListener
    void selectionTypeChanged(aidaw::SelectionType newType) override;
    void multiClipSelectionChanged(const std::unordered_set<aidaw::ClipId>& clipIds) override;
    void noteSelectionChanged(const aidaw::NoteSelection& selection) override;

    // TimelineStateListener — not overridden, base handles it

    // Set the clip to edit
    void setClip(aidaw::ClipId clipId);

    // Timeline mode (overrides base for multi-clip handling)
    void setRelativeTimeMode(bool relative) override;

    // Chord row visibility
    void setChordRowVisible(bool visible);
    bool isChordRowVisible() const {
        return showChordRow_;
    }

  private:
    // MidiEditorContent virtual implementations
    int getLeftPanelWidth() const override {
        return SIDEBAR_WIDTH + KEYBOARD_WIDTH;
    }
    void updateGridSize() override;
    void setGridPixelsPerBeat(double ppb) override;
    void setGridPlayheadPosition(double position) override;
    void setGridEditCursorPosition(double positionSeconds, bool visible) override;
    void onScrollPositionChanged(int scrollX, int scrollY) override;
    void onGridResolutionChanged() override;
    void updateGridLoopRegion() override;
    void setGridPhasePreview(double beats, bool active) override;

    // Override velocity lane methods
    void updateVelocityLane() override;
    void onVelocityEdited() override;

    // Layout constants (PianoRoll-specific)
    static constexpr int SIDEBAR_WIDTH = 32;
    static constexpr int KEYBOARD_WIDTH = 60;
    static constexpr int DEFAULT_NOTE_HEIGHT = 12;
    static constexpr int CHORD_ROW_HEIGHT = 24;
    static constexpr int HEADER_HEIGHT = CHORD_ROW_HEIGHT + RULER_HEIGHT;
    static constexpr int MIN_NOTE = 0;    // C-2
    static constexpr int MAX_NOTE = 127;  // G9

    // Vertical zoom limits
    static constexpr int MIN_NOTE_HEIGHT = 6;
    static constexpr int MAX_NOTE_HEIGHT = 40;

    // Zoom state (vertical — horizontal is in base)
    int noteHeight_ = DEFAULT_NOTE_HEIGHT;

    // Chord row visibility
    bool showChordRow_ = false;
    bool isSyncingChords_ = false;  // Re-entry guard for syncChordAnnotations

    // Initial centering flag
    bool needsInitialCentering_ = true;

    // Components (PianoRoll-specific)
    std::unique_ptr<aidaw::PianoRollGridComponent> gridComponent_;
    std::unique_ptr<aidaw::PianoRollKeyboard> keyboard_;
    std::unique_ptr<aidaw::SvgButton> chordToggle_;
    std::unique_ptr<aidaw::SvgButton> chordDetectBtn_;
    std::unique_ptr<aidaw::SvgButton> velocityToggle_;

    aidaw::MidiBridge* monitoredMidiBridge_ = nullptr;
    std::function<void(aidaw::TrackId, const aidaw::MidiNoteEvent&)> previousMidiNoteCallback_;
    bool midiNoteMonitorInstalled_ = false;

    // Grid component management
    void setupGridCallbacks();
    void installMidiNoteMonitor();
    void uninstallMidiNoteMonitor();
    void handleMidiNoteEvent(aidaw::TrackId trackId, const aidaw::MidiNoteEvent& event);
    void drawSidebar(juce::Graphics& g, juce::Rectangle<int> area);
    void drawChordRow(juce::Graphics& g, juce::Rectangle<int> area);
    void drawVelocityHeader(juce::Graphics& g, juce::Rectangle<int> area);
    void detectChordsFromNotes();
    void syncChordAnnotations(aidaw::ClipId clipId);

    // Helper to get current header height based on chord row visibility
    int getHeaderHeight() const {
        return showChordRow_ ? HEADER_HEIGHT : RULER_HEIGHT;
    }

    // Center the view on middle C (C4)
    void centerOnNote(int noteNumber);
    void centerOnNotes();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollContent)
};

}  // namespace aidaw::daw::ui
