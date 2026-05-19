#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <unordered_set>

#include "core/ClipManager.hpp"
#include "ui/components/common/DraggableValueLabel.hpp"
#include "ui/components/common/SvgButton.hpp"

namespace aidaw::daw::ui {

/**
 * @brief Self-contained fades section for clip inspector panels.
 *
 * Session clips: loop crossfade + launch fade smoothing.
 * Arrangement clips: full fades (In/Out with type/behaviour curves, auto-crossfade).
 *
 * Used by ClipInspector, AudioClipPropertiesContent, and SessionClipEditor.
 * Call setClip() or setSelectedClips(), then setBounds() for layout.
 */
class ClipFadesSection : public juce::Component {
  public:
    ClipFadesSection();
    ~ClipFadesSection() override;

    /** Set a single clip (convenience wrapper for single-clip panels). */
    void setClip(aidaw::ClipId clipId);

    /** Set multiple selected clips (multi-edit, for ClipInspector). */
    void setSelectedClips(const std::unordered_set<aidaw::ClipId>& clipIds);

    /** Height in pixels needed for the current clip configuration. Returns 0 if nothing to show. */
    int getPreferredHeight() const;

    /** True if any draggable value inside this section is currently being dragged. */
    bool isAnyValueDragging() const;

    void resized() override;

  private:
    std::unordered_set<aidaw::ClipId> selectedClipIds_;

    aidaw::ClipId primaryClipId() const {
        return selectedClipIds_.empty() ? aidaw::INVALID_CLIP_ID : *selectedClipIds_.begin();
    }

    // Section label
    juce::Label sectionLabel_;

    // Arrangement-only controls
    std::unique_ptr<aidaw::DraggableValueLabel> fadeInValue_;
    std::unique_ptr<aidaw::DraggableValueLabel> fadeOutValue_;
    std::unique_ptr<aidaw::SvgButton> fadeInTypeButtons_[4];
    std::unique_ptr<aidaw::SvgButton> fadeOutTypeButtons_[4];
    std::unique_ptr<aidaw::SvgButton> fadeInBehaviourButtons_[2];
    std::unique_ptr<aidaw::SvgButton> fadeOutBehaviourButtons_[2];
    juce::TextButton autoCrossfadeToggle_;

    // Session-only controls
    juce::Label launchFadeLabel_;
    std::unique_ptr<aidaw::DraggableValueLabel> launchFadeValue_;

    // Multi-drag start tracking
    double multiFadeInDragStart_ = 0.0;
    double multiFadeOutDragStart_ = 0.0;

    void initControls();
    void update();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipFadesSection)
};

}  // namespace aidaw::daw::ui
