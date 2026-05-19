#pragma once

#include <memory>

#include "../PanelContent.hpp"
#include "BaseInspector.hpp"
#include "core/SelectionManager.hpp"

namespace aidaw {
class TimelineController;
class AudioEngine;
}  // namespace aidaw

namespace aidaw::daw::ui {

/**
 * @brief Container that manages specialized inspectors based on selection
 *
 * Replaces InspectorContent by delegating to specialized inspectors:
 * - Listens to SelectionManager for selection changes
 * - Uses InspectorFactory to create appropriate inspector
 * - Manages inspector lifetime and layout
 * - Shows "No selection" message when nothing is selected
 *
 * This architecture keeps each inspector focused (~200-1000 LOC) and
 * makes it easy to add new inspector types in the future.
 */
class InspectorContainer : public PanelContent, public aidaw::SelectionManagerListener {
  public:
    InspectorContainer();
    ~InspectorContainer() override;

    // PanelContent interface
    PanelContentType getContentType() const override;
    PanelContentInfo getContentInfo() const override;
    void onActivated() override;
    void onDeactivated() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    /**
     * @brief Set the timeline controller reference
     * @param controller Timeline controller for accessing tempo/time signature
     */
    void setTimelineController(aidaw::TimelineController* controller);

    /**
     * @brief Set the audio engine reference
     * @param engine Audio engine for accessing audio/MIDI devices
     */
    void setAudioEngine(aidaw::AudioEngine* engine);

    // SelectionManagerListener interface
    void selectionTypeChanged(aidaw::SelectionType newType) override;
    void trackSelectionChanged(aidaw::TrackId trackId) override;
    void clipSelectionChanged(aidaw::ClipId clipId) override;
    void multiClipSelectionChanged(const std::unordered_set<aidaw::ClipId>& clipIds) override;
    void noteSelectionChanged(const aidaw::NoteSelection& selection) override;
    void multiTrackSelectionChanged(const std::unordered_set<aidaw::TrackId>& trackIds) override;
    void chainNodeSelectionChanged(const aidaw::ChainNodePath& path) override;

  private:
    // Current inspector (nullptr when no selection)
    std::unique_ptr<BaseInspector> currentInspector_;
    aidaw::SelectionType currentSelectionType_ = aidaw::SelectionType::None;

    // No selection message
    juce::Label noSelectionLabel_;

    // Shared dependencies passed to inspectors
    aidaw::TimelineController* timelineController_ = nullptr;
    aidaw::AudioEngine* audioEngine_ = nullptr;

    // Update methods
    void switchToInspector(aidaw::SelectionType type);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InspectorContainer)
};

}  // namespace aidaw::daw::ui
