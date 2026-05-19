#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include <vector>

#include "ui/themes/DarkTheme.hpp"

namespace aidaw {

// =============================================================================
// Layout constants (mirrors magda-core LayoutConfig)
// =============================================================================
struct ProLayoutConfig {
    static constexpr int trackHeaderWidth = 180;
    static constexpr int defaultTrackHeight = 80;
    static constexpr int minTrackHeight = 40;
    static constexpr int maxTrackHeight = 200;
    static constexpr int timeRulerHeight = 32;
    static constexpr int resizeHandleHeight = 5;
    static constexpr int bottomPanelMinHeight = 120;
    static constexpr int bottomPanelDefaultHeight = 250;
    static constexpr int tabBarHeight = 28;
    static constexpr int playheadWidth = 1;
    static constexpr double defaultPixelsPerBeat = 24.0;
    static constexpr double defaultBpm = 120.0;
    static constexpr int beatsPerBar = 4;
};

// =============================================================================
// Clip data (fake clips for display)
// =============================================================================
struct ClipData {
    double startBeat;
    double lengthBeats;
    juce::String name;
    juce::Colour colour;
};

// =============================================================================
// Track data
// =============================================================================
struct TrackData {
    juce::String name;
    juce::Colour colour;
    float volume = 0.8f;
    bool muted = false;
    bool soloed = false;
    bool armed = false;
    std::vector<ClipData> clips;
};

// =============================================================================
// TimeRuler - horizontal time ruler at top of track content
// =============================================================================
class TimeRuler : public juce::Component {
public:
    TimeRuler();
    void paint(juce::Graphics& g) override;

    void setPixelsPerBeat(double ppb) { pixelsPerBeat = ppb; repaint(); }
    void setScrollOffsetBeats(double offset) { scrollOffset = offset; repaint(); }
    void setBpm(double bpm) { tempo = bpm; repaint(); }

private:
    double pixelsPerBeat = ProLayoutConfig::defaultPixelsPerBeat;
    double scrollOffset = 0.0;
    double tempo = ProLayoutConfig::defaultBpm;
};

// =============================================================================
// TrackHeaderComponent - single track header (name, color, M/S/R, volume)
// =============================================================================
class TrackHeaderComponent : public juce::Component {
public:
    TrackHeaderComponent(TrackData& track, int index);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

    std::function<void(int)> onSelected;

private:
    TrackData& trackData;
    int trackIndex;
    bool selected = false;

    juce::TextButton muteButton{"M"};
    juce::TextButton soloButton{"S"};
    juce::TextButton armButton{"R"};
    juce::Slider volumeSlider;

    friend class TrackHeadersPanel;
};

// =============================================================================
// TrackHeadersPanel - vertical list of track headers
// =============================================================================
class TrackHeadersPanel : public juce::Component {
public:
    TrackHeadersPanel();
    void paint(juce::Graphics& g) override;
    void resized() override;

    void setTracks(std::vector<TrackData>& tracks);
    void setTrackHeight(int height) { trackHeight = height; resized(); }
    void setSelectedTrack(int index);

    std::function<void(int)> onTrackSelected;

private:
    juce::OwnedArray<TrackHeaderComponent> headers;
    int trackHeight = ProLayoutConfig::defaultTrackHeight;
    int selectedTrack = -1;
};

// =============================================================================
// TrackContentPanel - scrollable area with clips and grid
// =============================================================================
class TrackContentPanel : public juce::Component {
public:
    TrackContentPanel();
    void paint(juce::Graphics& g) override;

    void setTracks(const std::vector<TrackData>& tracks) { trackList = &tracks; repaint(); }
    void setPixelsPerBeat(double ppb) { pixelsPerBeat = ppb; repaint(); }
    void setScrollOffsetBeats(double offset) { scrollOffset = offset; repaint(); }
    void setTrackHeight(int height) { trackHeight = height; repaint(); }
    void setPlayheadBeat(double beat) { playheadBeat = beat; repaint(); }

private:
    const std::vector<TrackData>* trackList = nullptr;
    double pixelsPerBeat = ProLayoutConfig::defaultPixelsPerBeat;
    double scrollOffset = 0.0;
    int trackHeight = ProLayoutConfig::defaultTrackHeight;
    double playheadBeat = 0.0;

    void drawGrid(juce::Graphics& g);
    void drawClips(juce::Graphics& g);
    void drawPlayhead(juce::Graphics& g);
};

// =============================================================================
// BottomEditorPanel - tabbed bottom panel (Piano Roll / Mixer / Chain)
// =============================================================================
class BottomEditorPanel : public juce::Component {
public:
    BottomEditorPanel();
    void paint(juce::Graphics& g) override;
    void resized() override;

    enum Tab { PianoRoll = 0, Mixer, Chain, NumTabs };
    void setActiveTab(Tab tab) { activeTab = tab; repaint(); }

private:
    Tab activeTab = PianoRoll;
    juce::TextButton tabButtons[NumTabs];

    void drawPianoRollPlaceholder(juce::Graphics& g, juce::Rectangle<int> area);
    void drawMixerPlaceholder(juce::Graphics& g, juce::Rectangle<int> area);
    void drawChainPlaceholder(juce::Graphics& g, juce::Rectangle<int> area);
};

// =============================================================================
// HorizontalSplitResizer - draggable resize handle between top/bottom panels
// =============================================================================
class HorizontalSplitResizer : public juce::Component {
public:
    HorizontalSplitResizer();
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    std::function<void(int)> onDrag;  // delta Y

private:
    int dragStartY = 0;
    bool dragging = false;
};

// =============================================================================
// ProMainView - main DAW arrangement view (magda-core style)
// =============================================================================
class ProMainView : public juce::Component, public juce::Timer {
public:
    ProMainView();
    ~ProMainView() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void mouseWheelMove(const juce::MouseEvent& e,
                        const juce::MouseWheelDetails& wheel) override;

private:
    // Track data
    std::vector<TrackData> tracks;

    // Top panel: arrangement area
    TimeRuler timeRuler;
    TrackHeadersPanel trackHeaders;
    juce::Viewport trackContentViewport;
    TrackContentPanel trackContent;

    // Bottom panel: editor
    BottomEditorPanel bottomPanel;
    HorizontalSplitResizer splitResizer;

    // Layout state
    int bottomPanelHeight = ProLayoutConfig::bottomPanelDefaultHeight;
    double pixelsPerBeat = ProLayoutConfig::defaultPixelsPerBeat;
    double scrollOffsetBeats = 0.0;
    double playheadBeat = 0.0;
    int selectedTrack = -1;
    bool playing = false;

    // Initialization
    void createDefaultTracks();
    void updateContentSize();
};

}  // namespace aidaw

