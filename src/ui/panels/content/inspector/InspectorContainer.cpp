#include "InspectorContainer.hpp"

#include "../../themes/DarkTheme.hpp"
#include "../../themes/FontManager.hpp"
#include "ClipInspector.hpp"
#include "DeviceInspector.hpp"
#include "InspectorFactory.hpp"
#include "NoteInspector.hpp"
#include "TrackInspector.hpp"
#include "core/SelectionManager.hpp"

namespace aidaw::daw::ui {

InspectorContainer::InspectorContainer() {
    // No selection label
    noSelectionLabel_.setText("No selection", juce::dontSendNotification);
    noSelectionLabel_.setFont(FontManager::getInstance().getUIFont(12.0f));
    noSelectionLabel_.setColour(juce::Label::textColourId, DarkTheme::getSecondaryTextColour());
    noSelectionLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(noSelectionLabel_);

    // Register as SelectionManager listener
    aidaw::SelectionManager::getInstance().addListener(this);

    // Initialize with current selection
    currentSelectionType_ = aidaw::SelectionManager::getInstance().getSelectionType();
    switchToInspector(currentSelectionType_);
}

InspectorContainer::~InspectorContainer() {
    if (currentInspector_) {
        currentInspector_->onDeactivated();
    }
    aidaw::SelectionManager::getInstance().removeListener(this);
}

PanelContentType InspectorContainer::getContentType() const {
    return PanelContentType::Inspector;
}

PanelContentInfo InspectorContainer::getContentInfo() const {
    return {PanelContentType::Inspector, "Inspector", "View and edit properties of selected items",
            "Inspector"};
}

void InspectorContainer::onActivated() {
    // Activate current inspector when container is shown
    if (currentInspector_) {
        currentInspector_->onActivated();
    }
}

void InspectorContainer::onDeactivated() {
    // Deactivate current inspector when container is hidden
    if (currentInspector_) {
        currentInspector_->onDeactivated();
    }
}

void InspectorContainer::paint(juce::Graphics& g) {
    g.fillAll(DarkTheme::getPanelBackgroundColour());
}

void InspectorContainer::resized() {
    auto bounds = getLocalBounds();

    if (currentInspector_) {
        currentInspector_->setBounds(bounds);
        noSelectionLabel_.setVisible(false);
    } else {
        // Center the no-selection label
        noSelectionLabel_.setBounds(bounds);
        noSelectionLabel_.setVisible(true);
    }
}

void InspectorContainer::setTimelineController(aidaw::TimelineController* controller) {
    timelineController_ = controller;
    if (currentInspector_) {
        currentInspector_->setTimelineController(controller);
    }
}

void InspectorContainer::setAudioEngine(aidaw::AudioEngine* engine) {
    audioEngine_ = engine;
    if (currentInspector_) {
        currentInspector_->setAudioEngine(engine);
    }
}

void InspectorContainer::selectionTypeChanged(aidaw::SelectionType newType) {
    if (newType != currentSelectionType_) {
        switchToInspector(newType);
    }
}

void InspectorContainer::trackSelectionChanged(aidaw::TrackId trackId) {
    auto* trackInspector = dynamic_cast<TrackInspector*>(currentInspector_.get());
    if (trackInspector) {
        trackInspector->setSelectedTrack(trackId);
    }
}

void InspectorContainer::clipSelectionChanged(aidaw::ClipId clipId) {
    auto* clipInspector = dynamic_cast<ClipInspector*>(currentInspector_.get());
    if (clipInspector) {
        clipInspector->setSelectedClips({clipId});
    }
}

void InspectorContainer::multiClipSelectionChanged(
    const std::unordered_set<aidaw::ClipId>& clipIds) {
    auto* clipInspector = dynamic_cast<ClipInspector*>(currentInspector_.get());
    if (clipInspector) {
        clipInspector->setSelectedClips(clipIds);
    }
}

void InspectorContainer::noteSelectionChanged(const aidaw::NoteSelection& selection) {
    auto* noteInspector = dynamic_cast<NoteInspector*>(currentInspector_.get());
    if (noteInspector) {
        noteInspector->setSelectedNotes(selection);
    }
}

void InspectorContainer::multiTrackSelectionChanged(
    const std::unordered_set<aidaw::TrackId>& trackIds) {
    auto* trackInspector = dynamic_cast<TrackInspector*>(currentInspector_.get());
    if (trackInspector) {
        trackInspector->setSelectedTracks(trackIds);
    }
}

void InspectorContainer::chainNodeSelectionChanged(const aidaw::ChainNodePath& path) {
    auto* deviceInspector = dynamic_cast<DeviceInspector*>(currentInspector_.get());
    if (deviceInspector) {
        deviceInspector->setSelectedChainNode(path);
    }
}

void InspectorContainer::switchToInspector(aidaw::SelectionType type) {
    // Deactivate current inspector
    if (currentInspector_) {
        currentInspector_->onDeactivated();
        removeChildComponent(currentInspector_.get());
        currentInspector_.reset();
    }

    currentSelectionType_ = type;

    // Create new inspector
    currentInspector_ = InspectorFactory::createInspector(type);

    if (currentInspector_) {
        // Set up dependencies
        currentInspector_->setTimelineController(timelineController_);
        currentInspector_->setAudioEngine(audioEngine_);

        // Activate and add to UI
        currentInspector_->onActivated();
        addAndMakeVisible(*currentInspector_);

        // Forward current selection data to the newly created inspector
        auto& sm = aidaw::SelectionManager::getInstance();
        if (type == aidaw::SelectionType::Track) {
            auto* trackInspector = dynamic_cast<TrackInspector*>(currentInspector_.get());
            if (trackInspector)
                trackInspector->setSelectedTrack(sm.getSelectedTrack());
        } else if (type == aidaw::SelectionType::MultiTrack) {
            auto* trackInspector = dynamic_cast<TrackInspector*>(currentInspector_.get());
            if (trackInspector)
                trackInspector->setSelectedTracks(sm.getSelectedTracks());
        } else if (type == aidaw::SelectionType::Clip) {
            auto* clipInspector = dynamic_cast<ClipInspector*>(currentInspector_.get());
            if (clipInspector)
                clipInspector->setSelectedClips({sm.getSelectedClip()});
        } else if (type == aidaw::SelectionType::MultiClip) {
            auto* clipInspector = dynamic_cast<ClipInspector*>(currentInspector_.get());
            if (clipInspector)
                clipInspector->setSelectedClips(sm.getSelectedClips());
        } else if (type == aidaw::SelectionType::Note) {
            auto* noteInspector = dynamic_cast<NoteInspector*>(currentInspector_.get());
            if (noteInspector)
                noteInspector->setSelectedNotes(sm.getNoteSelection());
        } else if (type == aidaw::SelectionType::ChainNode) {
            auto* deviceInspector = dynamic_cast<DeviceInspector*>(currentInspector_.get());
            if (deviceInspector)
                deviceInspector->setSelectedChainNode(sm.getSelectedChainNode());
        }
    }

    resized();
}

}  // namespace aidaw::daw::ui
