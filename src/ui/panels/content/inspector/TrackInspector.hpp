#pragma once

#include <map>
#include <unordered_set>

#include "../../common/DraggableValueLabel.hpp"
#include "../../mixer/InputTypeSelector.hpp"
#include "../../mixer/RoutingSelector.hpp"
#include "BaseInspector.hpp"
#include "audio/MidiBridge.hpp"
#include "core/TrackManager.hpp"

namespace aidaw::daw::ui {

/**
 * @brief Inspector for track properties
 *
 * Displays and edits:
 * - Track name
 * - Mute/Solo/Record state
 * - Volume and Pan
 * - Audio/MIDI routing (input/output)
 * - Sends/Receives
 * - Clip count
 */
class TrackInspector : public BaseInspector,
                       public aidaw::TrackManagerListener,
                       public aidaw::MidiBridge::Listener,
                       public juce::Timer {
  public:
    TrackInspector();
    ~TrackInspector() override;

    void onActivated() override;
    void onDeactivated() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    /**
     * @brief Set the currently selected track
     * @param trackId The track to inspect (INVALID_TRACK_ID for none)
     */
    void setSelectedTrack(aidaw::TrackId trackId);

    /**
     * @brief Set multiple selected tracks for multi-track display
     * @param trackIds Set of selected track IDs
     */
    void setSelectedTracks(const std::unordered_set<aidaw::TrackId>& trackIds);

    // TrackManagerListener interface
    void tracksChanged() override;
    void trackPropertyChanged(int trackId) override;
    void trackDevicesChanged(aidaw::TrackId trackId) override;
    void trackSelectionChanged(aidaw::TrackId trackId) override;
    void masterChannelChanged() override;
    void deviceParameterChanged(aidaw::DeviceId deviceId, int paramIndex, float newValue) override;

    // Timer for polling MIDI device changes
    void timerCallback() override;

  private:
    // Current selection
    aidaw::TrackId selectedTrackId_ = aidaw::INVALID_TRACK_ID;
    std::unordered_set<aidaw::TrackId> selectedTrackIds_;  // For multi-track mode
    bool isMultiTrackMode_ = false;

    // Base values for relative multi-track drag (captured at drag start)
    std::unordered_map<aidaw::TrackId, float> multiTrackBaseVolumes_;
    std::unordered_map<aidaw::TrackId, float> multiTrackBasePans_;
    double multiTrackDragStartDb_ = 0.0;
    double multiTrackDragStartPan_ = 0.0;

    // Track properties section
    juce::Label trackNameLabel_;
    juce::Label trackNameValue_;
    std::unique_ptr<juce::Component> colourSwatch_;
    juce::TextButton muteButton_;
    std::unique_ptr<juce::DrawableButton> speakerButton_;  // Speaker icon for master mute
    juce::TextButton soloButton_;
    juce::TextButton recordButton_;
    juce::TextButton monitorButton_;
    std::unique_ptr<aidaw::DraggableValueLabel> gainLabel_;
    std::unique_ptr<aidaw::DraggableValueLabel> panLabel_;

    // Routing section (unified input type toggle + selectors)
    juce::Label routingSectionLabel_;
    std::unique_ptr<aidaw::InputTypeSelector> inputTypeSelector_;  // Hidden, internal state
    std::unique_ptr<aidaw::RoutingSelector> audioInputSelector_;   // Audio input
    std::unique_ptr<aidaw::RoutingSelector> inputSelector_;        // MIDI input
    std::unique_ptr<aidaw::RoutingSelector> outputSelector_;       // Audio output
    std::unique_ptr<aidaw::RoutingSelector> midiOutputSelector_;   // MIDI output
    juce::Label audioColumnLabel_;                                 // "Audio" column header
    juce::Label midiColumnLabel_;                                  // "MIDI" column header
    std::unique_ptr<juce::Component> inputIcon_;                   // Non-interactive Input icon
    std::unique_ptr<juce::Component> outputIcon_;                  // Non-interactive Output icon

    // Send/Receive section
    juce::Label sendReceiveSectionLabel_;
    juce::TextButton addSendButton_;
    std::vector<std::unique_ptr<juce::Label>> sendDestLabels_;
    std::vector<std::unique_ptr<aidaw::DraggableValueLabel>> sendLevelLabels_;
    std::vector<std::unique_ptr<juce::TextButton>> sendDeleteButtons_;
    juce::Label noSendsLabel_;
    juce::Label receivesLabel_;

    // Clips section
    juce::Label clipsSectionLabel_;
    juce::Label clipCountLabel_;

    // Latency display
    juce::Label latencyLabel_;
    juce::Label latencyValue_;

    // Section separator Y positions (computed in resized, drawn in paint)
    std::vector<int> sectionSeparatorYs_;

    // Update methods
    void updateFromSelectedTrack();
    void updateFromMultiTrackSelection();
    void showTrackControls(bool show);
    void rebuildSendsUI();
    void showAddSendMenu();
    void populateRoutingSelectors();
    void populateAudioInputOptions();
    void populateAudioOutputOptions();
    void populateMidiInputOptions();

    // MidiBridge::Listener
    void midiDeviceListChanged() override;
    void populateMidiOutputOptions();
    void updateRoutingSelectorsFromTrack();

    // Routing: option ID → TrackId mapping for destinations/sources
    std::map<int, aidaw::TrackId> outputTrackMapping_;
    std::map<int, aidaw::TrackId> midiOutputTrackMapping_;
    std::map<int, aidaw::TrackId> inputTrackMapping_;
    std::map<int, juce::String> inputChannelMapping_;

    // MIDI device change detection
    size_t lastMidiInputCount_ = 0;
    size_t lastMidiOutputCount_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackInspector)
};

}  // namespace aidaw::daw::ui
