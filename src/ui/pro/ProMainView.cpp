#include "ProMainView.hpp"

namespace aidaw {

// =============================================================================
// TimeRuler
// =============================================================================

TimeRuler::TimeRuler() {
    setOpaque(true);
}

void TimeRuler::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    g.setColour(DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND));
    g.fillRect(bounds);

    // Bottom separator line
    g.setColour(DarkTheme::getColour(DarkTheme::BORDER));
    g.drawHorizontalLine(bounds.getHeight() - 1, 0.0f, (float)bounds.getWidth());

    // Draw bar/beat markers
    const double beatsPerBar = (double)ProLayoutConfig::beatsPerBar;
    const double startBeat = scrollOffset;
    const double endBeat = scrollOffset + (double)bounds.getWidth() / pixelsPerBeat;

    int firstBar = (int)(startBeat / beatsPerBar);
    int lastBar = (int)(endBeat / beatsPerBar) + 1;

    g.setFont(11.0f);

    for (int bar = firstBar; bar <= lastBar; ++bar) {
        double barBeat = bar * beatsPerBar;
        int x = (int)((barBeat - scrollOffset) * pixelsPerBeat);

        if (x < 0 || x > bounds.getWidth()) continue;

        // Bar line (major)
        g.setColour(DarkTheme::getColour(DarkTheme::TEXT_SECONDARY));
        g.drawVerticalLine(x, (float)(bounds.getHeight() - 14), (float)(bounds.getHeight() - 2));

        // Bar number label
        g.setColour(DarkTheme::getColour(DarkTheme::TEXT_DIM));
        g.drawText(juce::String(bar + 1), x + 3, 4, 40, 14, juce::Justification::centredLeft);

        // Beat subdivisions
        for (int beat = 1; beat < (int)beatsPerBar; ++beat) {
            double beatPos = barBeat + beat;
            int bx = (int)((beatPos - scrollOffset) * pixelsPerBeat);
            if (bx < 0 || bx > bounds.getWidth()) continue;

            g.setColour(DarkTheme::getColour(DarkTheme::TEXT_DISABLED));
            g.drawVerticalLine(bx, (float)(bounds.getHeight() - 8), (float)(bounds.getHeight() - 2));
        }
    }
}

// =============================================================================
// TrackHeaderComponent
// =============================================================================

TrackHeaderComponent::TrackHeaderComponent(TrackData& track, int index)
    : trackData(track), trackIndex(index) {
    muteButton.setColour(juce::TextButton::buttonColourId,
                         DarkTheme::getColour(DarkTheme::BUTTON_NORMAL));
    muteButton.setColour(juce::TextButton::textColourOffId,
                         DarkTheme::getColour(DarkTheme::TEXT_SECONDARY));
    muteButton.onClick = [this] {
        trackData.muted = !trackData.muted;
        muteButton.setColour(juce::TextButton::buttonColourId,
                             trackData.muted ? DarkTheme::getColour(DarkTheme::STATUS_WARNING)
                                            : DarkTheme::getColour(DarkTheme::BUTTON_NORMAL));
        getParentComponent()->repaint();
    };
    addAndMakeVisible(muteButton);

    soloButton.setColour(juce::TextButton::buttonColourId,
                         DarkTheme::getColour(DarkTheme::BUTTON_NORMAL));
    soloButton.setColour(juce::TextButton::textColourOffId,
                         DarkTheme::getColour(DarkTheme::TEXT_SECONDARY));
    soloButton.onClick = [this] {
        trackData.soloed = !trackData.soloed;
        soloButton.setColour(juce::TextButton::buttonColourId,
                             trackData.soloed ? DarkTheme::getColour(DarkTheme::ACCENT_CYAN)
                                            : DarkTheme::getColour(DarkTheme::BUTTON_NORMAL));
        getParentComponent()->repaint();
    };
    addAndMakeVisible(soloButton);

    armButton.setColour(juce::TextButton::buttonColourId,
                        DarkTheme::getColour(DarkTheme::BUTTON_NORMAL));
    armButton.setColour(juce::TextButton::textColourOffId,
                        DarkTheme::getColour(DarkTheme::TEXT_SECONDARY));
    armButton.onClick = [this] {
        trackData.armed = !trackData.armed;
        armButton.setColour(juce::TextButton::buttonColourId,
                            trackData.armed ? DarkTheme::getColour(DarkTheme::STATUS_ERROR)
                                           : DarkTheme::getColour(DarkTheme::BUTTON_NORMAL));
    };
    addAndMakeVisible(armButton);

    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(track.volume);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setColour(juce::Slider::trackColourId,
                           DarkTheme::getColour(DarkTheme::ACCENT_BLUE));
    volumeSlider.setColour(juce::Slider::backgroundColourId,
                           DarkTheme::getColour(DarkTheme::SURFACE));
    addAndMakeVisible(volumeSlider);
}

void TrackHeaderComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();

    // Background
    auto bgColour = selected ? DarkTheme::getColour(DarkTheme::TRACK_SELECTED)
                             : DarkTheme::getColour(DarkTheme::TRACK_BACKGROUND);
    g.setColour(bgColour);
    g.fillRect(bounds);

    // Color strip on left edge
    g.setColour(trackData.colour);
    g.fillRect(bounds.removeFromLeft(4));

    // Track name
    g.setColour(DarkTheme::getColour(DarkTheme::TEXT_PRIMARY));
    g.setFont(13.0f);
    g.drawText(trackData.name, 10, 6, 120, 18, juce::Justification::centredLeft);

    // Bottom separator
    g.setColour(DarkTheme::getColour(DarkTheme::SEPARATOR));
    g.drawHorizontalLine(getHeight() - 1, 0.0f, (float)getWidth());

    // Right border
    g.setColour(DarkTheme::getColour(DarkTheme::BORDER));
    g.drawVerticalLine(getWidth() - 1, 0.0f, (float)getHeight());
}

void TrackHeaderComponent::resized() {
    auto bounds = getLocalBounds().reduced(6);
    bounds.removeFromLeft(4);  // color strip space
    bounds.removeFromTop(22);  // name space

    auto buttonRow = bounds.removeFromTop(22);
    muteButton.setBounds(buttonRow.removeFromLeft(24).reduced(1));
    soloButton.setBounds(buttonRow.removeFromLeft(24).reduced(1));
    armButton.setBounds(buttonRow.removeFromLeft(24).reduced(1));

    bounds.removeFromTop(4);
    auto sliderRow = bounds.removeFromTop(18);
    volumeSlider.setBounds(sliderRow.withWidth(juce::jmin(sliderRow.getWidth(), 100)));
}

void TrackHeaderComponent::mouseDown(const juce::MouseEvent&) {
    if (onSelected)
        onSelected(trackIndex);
}

// =============================================================================
// TrackHeadersPanel
// =============================================================================

TrackHeadersPanel::TrackHeadersPanel() {
    setOpaque(true);
}

void TrackHeadersPanel::paint(juce::Graphics& g) {
    g.setColour(DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND));
    g.fillRect(getLocalBounds());
}

void TrackHeadersPanel::resized() {
    int y = 0;
    for (auto* header : headers) {
        header->setBounds(0, y, getWidth(), trackHeight);
        y += trackHeight;
    }
}

void TrackHeadersPanel::setTracks(std::vector<TrackData>& tracks) {
    headers.clear();
    for (int i = 0; i < (int)tracks.size(); ++i) {
        auto* h = new TrackHeaderComponent(tracks[(size_t)i], i);
        h->onSelected = [this](int idx) {
            setSelectedTrack(idx);
            if (onTrackSelected)
                onTrackSelected(idx);
        };
        headers.add(h);
        addAndMakeVisible(h);
    }
    resized();
}

void TrackHeadersPanel::setSelectedTrack(int index) {
    selectedTrack = index;
    for (int i = 0; i < headers.size(); ++i) {
        headers[i]->selected = (i == index);
        headers[i]->repaint();
    }
}

// =============================================================================
// TrackContentPanel
// =============================================================================

TrackContentPanel::TrackContentPanel() {
    setOpaque(true);
}

void TrackContentPanel::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();

    // Background
    g.setColour(DarkTheme::getColour(DarkTheme::BACKGROUND));
    g.fillRect(bounds);

    drawGrid(g);
    drawClips(g);
    drawPlayhead(g);
}

void TrackContentPanel::drawGrid(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    const double beatsPerBar = (double)ProLayoutConfig::beatsPerBar;
    const double startBeat = scrollOffset;
    const double endBeat = scrollOffset + (double)bounds.getWidth() / pixelsPerBeat;

    int firstBar = (int)(startBeat / beatsPerBar);
    int lastBar = (int)(endBeat / beatsPerBar) + 1;

    // Horizontal track separators
    if (trackList) {
        for (int i = 0; i <= (int)trackList->size(); ++i) {
            int y = i * trackHeight;
            g.setColour(DarkTheme::getColour(DarkTheme::SEPARATOR));
            g.drawHorizontalLine(y, 0.0f, (float)bounds.getWidth());
        }
    }

    // Vertical grid lines
    for (int bar = firstBar; bar <= lastBar; ++bar) {
        double barBeat = bar * beatsPerBar;
        int x = (int)((barBeat - scrollOffset) * pixelsPerBeat);

        if (x < 0 || x > bounds.getWidth()) continue;

        // Bar line (brighter)
        g.setColour(DarkTheme::getColour(DarkTheme::BAR_LINE));
        g.drawVerticalLine(x, 0.0f, (float)bounds.getHeight());

        // Beat lines (dimmer)
        for (int beat = 1; beat < (int)beatsPerBar; ++beat) {
            double beatPos = barBeat + beat;
            int bx = (int)((beatPos - scrollOffset) * pixelsPerBeat);
            if (bx < 0 || bx > bounds.getWidth()) continue;

            g.setColour(DarkTheme::getColour(DarkTheme::GRID_LINE));
            g.drawVerticalLine(bx, 0.0f, (float)bounds.getHeight());
        }
    }
}

void TrackContentPanel::drawClips(juce::Graphics& g) {
    if (!trackList) return;

    for (int i = 0; i < (int)trackList->size(); ++i) {
        const auto& track = (*trackList)[(size_t)i];
        int trackY = i * trackHeight;

        for (const auto& clip : track.clips) {
            double clipStartX = (clip.startBeat - scrollOffset) * pixelsPerBeat;
            double clipWidth = clip.lengthBeats * pixelsPerBeat;

            int x = (int)clipStartX;
            int w = (int)clipWidth;
            int y = trackY + 4;
            int h = trackHeight - 8;

            if (x + w < 0 || x > getWidth()) continue;

            auto clipBounds = juce::Rectangle<int>(x, y, w, h);

            // Clip body with rounded corners
            g.setColour(clip.colour.withAlpha(0.3f));
            g.fillRoundedRectangle(clipBounds.toFloat(), 3.0f);

            // Clip border
            g.setColour(clip.colour.withAlpha(0.7f));
            g.drawRoundedRectangle(clipBounds.toFloat(), 3.0f, 1.0f);

            // Clip top accent bar
            auto topBar = clipBounds.removeFromTop(3);
            g.setColour(clip.colour.withAlpha(0.9f));
            g.fillRoundedRectangle(topBar.toFloat(), 1.5f);

            // Fake waveform/MIDI content inside clip
            juce::Random rng((juce::int64)(clip.startBeat * 1000 + i * 100));
            auto contentArea = juce::Rectangle<int>(x + 2, y + 6, w - 4, h - 10);
            g.setColour(clip.colour.withAlpha(0.4f));

            int numBars = juce::jmax(1, w / 8);
            for (int b = 0; b < numBars; ++b) {
                float barH = rng.nextFloat() * (float)contentArea.getHeight() * 0.8f + 2.0f;
                float barX = (float)contentArea.getX() + (float)b * ((float)contentArea.getWidth() / (float)numBars);
                float barY = (float)contentArea.getCentreY() - barH * 0.5f;
                g.fillRect(barX, barY, 3.0f, barH);
            }

            // Clip name
            g.setColour(DarkTheme::getColour(DarkTheme::TEXT_PRIMARY).withAlpha(0.8f));
            g.setFont(10.0f);
            g.drawText(clip.name, x + 5, y + 4, w - 10, 12, juce::Justification::centredLeft);
        }
    }
}

void TrackContentPanel::drawPlayhead(juce::Graphics& g) {
    double x = (playheadBeat - scrollOffset) * pixelsPerBeat;
    if (x < 0 || x > getWidth()) return;

    // Playhead line
    g.setColour(juce::Colours::white);
    g.drawVerticalLine((int)x, 0.0f, (float)getHeight());

    // Playhead triangle at top
    juce::Path triangle;
    float tx = (float)x;
    triangle.addTriangle(tx - 4.0f, 0.0f, tx + 4.0f, 0.0f, tx, 6.0f);
    g.setColour(juce::Colours::white);
    g.fillPath(triangle);
}

// =============================================================================
// BottomEditorPanel
// =============================================================================

BottomEditorPanel::BottomEditorPanel() {
    const char* tabNames[] = {"Piano Roll", "Mixer", "Chain"};
    for (int i = 0; i < NumTabs; ++i) {
        tabButtons[i].setButtonText(tabNames[i]);
        tabButtons[i].setColour(juce::TextButton::buttonColourId,
                                DarkTheme::getColour(DarkTheme::BUTTON_NORMAL));
        tabButtons[i].setColour(juce::TextButton::textColourOffId,
                                DarkTheme::getColour(DarkTheme::TEXT_SECONDARY));
        tabButtons[i].onClick = [this, i] {
            setActiveTab((Tab)i);
        };
        addAndMakeVisible(tabButtons[i]);
    }
}

void BottomEditorPanel::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();

    // Panel background
    g.setColour(DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND));
    g.fillRect(bounds);

    // Top separator
    g.setColour(DarkTheme::getColour(DarkTheme::BORDER));
    g.drawHorizontalLine(0, 0.0f, (float)bounds.getWidth());

    // Content area below tabs
    auto contentArea = bounds;
    contentArea.removeFromTop(ProLayoutConfig::tabBarHeight);

    switch (activeTab) {
        case PianoRoll: drawPianoRollPlaceholder(g, contentArea); break;
        case Mixer:     drawMixerPlaceholder(g, contentArea); break;
        case Chain:     drawChainPlaceholder(g, contentArea); break;
        default: break;
    }
}

void BottomEditorPanel::resized() {
    auto tabArea = getLocalBounds().removeFromTop(ProLayoutConfig::tabBarHeight);
    tabArea = tabArea.reduced(4, 2);

    for (int i = 0; i < NumTabs; ++i) {
        tabButtons[i].setBounds(tabArea.removeFromLeft(80).reduced(1));
        // Highlight active tab
        tabButtons[i].setColour(juce::TextButton::buttonColourId,
                                (i == (int)activeTab)
                                    ? DarkTheme::getColour(DarkTheme::ACCENT_BLUE)
                                    : DarkTheme::getColour(DarkTheme::BUTTON_NORMAL));
    }
}

void BottomEditorPanel::drawPianoRollPlaceholder(juce::Graphics& g, juce::Rectangle<int> area) {
    area = area.reduced(8);

    // Piano keys on left
    int keyWidth = 40;
    auto keysArea = area.removeFromLeft(keyWidth);
    auto gridArea = area;

    // Draw piano keys
    const int numKeys = 24;
    float keyHeight = (float)keysArea.getHeight() / (float)numKeys;

    for (int i = 0; i < numKeys; ++i) {
        int noteInOctave = i % 12;
        bool isBlack = (noteInOctave == 1 || noteInOctave == 3 || noteInOctave == 6 ||
                        noteInOctave == 8 || noteInOctave == 10);

        float y = (float)keysArea.getY() + (float)(numKeys - 1 - i) * keyHeight;
        auto keyRect = juce::Rectangle<float>((float)keysArea.getX(), y,
                                              (float)keyWidth, keyHeight);

        g.setColour(isBlack ? juce::Colour(0xFF2A2A35) : juce::Colour(0xFF555566));
        g.fillRect(keyRect.reduced(0.5f));
        g.setColour(DarkTheme::getColour(DarkTheme::SEPARATOR));
        g.drawRect(keyRect, 0.5f);
    }

    // Draw note grid
    g.setColour(DarkTheme::getColour(DarkTheme::BACKGROUND));
    g.fillRect(gridArea);

    // Horizontal lines for each key
    for (int i = 0; i <= numKeys; ++i) {
        float y = (float)gridArea.getY() + (float)i * keyHeight;
        g.setColour(DarkTheme::getColour(DarkTheme::GRID_LINE));
        g.drawHorizontalLine((int)y, (float)gridArea.getX(), (float)gridArea.getRight());
    }

    // Vertical beat lines
    int numBeats = gridArea.getWidth() / 24;
    for (int i = 0; i <= numBeats; ++i) {
        int x = gridArea.getX() + i * 24;
        bool isBar = (i % 4 == 0);
        g.setColour(isBar ? DarkTheme::getColour(DarkTheme::BAR_LINE)
                          : DarkTheme::getColour(DarkTheme::GRID_LINE));
        g.drawVerticalLine(x, (float)gridArea.getY(), (float)gridArea.getBottom());
    }

    // Some fake MIDI notes
    juce::Random rng(42);
    g.setColour(DarkTheme::getColour(DarkTheme::ACCENT_CYAN).withAlpha(0.7f));
    for (int n = 0; n < 12; ++n) {
        int noteIdx = rng.nextInt(numKeys);
        int startBeat = rng.nextInt(numBeats - 2);
        int length = rng.nextInt(3) + 1;
        float ny = (float)gridArea.getY() + (float)(numKeys - 1 - noteIdx) * keyHeight;
        float nx = (float)gridArea.getX() + (float)startBeat * 24.0f;
        float nw = (float)length * 24.0f;
        g.fillRoundedRectangle(nx, ny + 1.0f, nw, keyHeight - 2.0f, 2.0f);
    }
}

void BottomEditorPanel::drawMixerPlaceholder(juce::Graphics& g, juce::Rectangle<int> area) {
    area = area.reduced(8);

    // Draw mixer channel strips
    int numChannels = 8;
    int channelWidth = juce::jmin(area.getWidth() / numChannels, 60);

    juce::Colour channelColours[] = {
        DarkTheme::getColour(DarkTheme::ACCENT_CYAN),
        DarkTheme::getColour(DarkTheme::ACCENT_GREEN),
        DarkTheme::getColour(DarkTheme::ACCENT_ORANGE),
        DarkTheme::getColour(DarkTheme::ACCENT_PURPLE),
    };

    juce::Random rng(123);
    for (int i = 0; i < numChannels; ++i) {
        auto channelArea = area.removeFromLeft(channelWidth).reduced(2);

        // Channel background
        g.setColour(DarkTheme::getColour(DarkTheme::SURFACE));
        g.fillRoundedRectangle(channelArea.toFloat(), 3.0f);

        // Fader track
        auto faderArea = channelArea.reduced(8, 20);
        g.setColour(DarkTheme::getColour(DarkTheme::BACKGROUND));
        g.fillRoundedRectangle(faderArea.toFloat(), 2.0f);

        // Fader level
        float level = 0.4f + rng.nextFloat() * 0.5f;
        auto levelArea = faderArea;
        levelArea = levelArea.removeFromBottom((int)((float)faderArea.getHeight() * level));
        g.setColour(channelColours[i % 4].withAlpha(0.6f));
        g.fillRoundedRectangle(levelArea.toFloat(), 2.0f);

        // Channel label
        g.setColour(DarkTheme::getColour(DarkTheme::TEXT_DIM));
        g.setFont(9.0f);
        g.drawText(juce::String(i + 1), channelArea.removeFromBottom(14),
                   juce::Justification::centred);
    }
}

void BottomEditorPanel::drawChainPlaceholder(juce::Graphics& g, juce::Rectangle<int> area) {
    area = area.reduced(12);

    // Draw plugin chain slots
    g.setColour(DarkTheme::getColour(DarkTheme::TEXT_DIM));
    g.setFont(12.0f);
    g.drawText("Effect Chain", area.removeFromTop(20), juce::Justification::centredLeft);

    area.removeFromTop(8);

    const char* plugins[] = {"EQ", "Compressor", "Reverb", "Delay"};
    for (int i = 0; i < 4; ++i) {
        auto slot = area.removeFromTop(32).withWidth(juce::jmin(area.getWidth(), 200));

        // Slot background
        g.setColour(DarkTheme::getColour(DarkTheme::SURFACE));
        g.fillRoundedRectangle(slot.toFloat(), 4.0f);

        // Slot border
        g.setColour(DarkTheme::getColour(DarkTheme::BORDER));
        g.drawRoundedRectangle(slot.toFloat(), 4.0f, 1.0f);

        // Plugin name
        g.setColour(DarkTheme::getColour(DarkTheme::TEXT_SECONDARY));
        g.setFont(11.0f);
        g.drawText(plugins[i], slot.reduced(8, 0), juce::Justification::centredLeft);

        // Bypass indicator
        auto indicator = slot.removeFromRight(24).reduced(6);
        g.setColour(DarkTheme::getColour(DarkTheme::ACCENT_GREEN));
        g.fillEllipse(indicator.toFloat());

        area.removeFromTop(4);
    }
}

// =============================================================================
// HorizontalSplitResizer
// =============================================================================

HorizontalSplitResizer::HorizontalSplitResizer() {
    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
}

void HorizontalSplitResizer::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    g.setColour(DarkTheme::getColour(DarkTheme::BORDER));
    g.fillRect(bounds);

    // Draw grip dots in center
    g.setColour(DarkTheme::getColour(DarkTheme::RESIZE_HANDLE));
    int cx = bounds.getCentreX();
    int cy = bounds.getCentreY();
    for (int i = -2; i <= 2; ++i) {
        g.fillEllipse((float)(cx + i * 8 - 1), (float)(cy - 1), 3.0f, 3.0f);
    }
}

void HorizontalSplitResizer::mouseDown(const juce::MouseEvent& e) {
    dragStartY = e.getScreenY();
    dragging = true;
}

void HorizontalSplitResizer::mouseDrag(const juce::MouseEvent& e) {
    if (dragging && onDrag) {
        int delta = dragStartY - e.getScreenY();
        dragStartY = e.getScreenY();
        onDrag(delta);
    }
}

void HorizontalSplitResizer::mouseUp(const juce::MouseEvent&) {
    dragging = false;
}

// =============================================================================
// ProMainView
// =============================================================================

ProMainView::ProMainView() {
    // Initialize default tracks with clips
    tracks.push_back({"Drums", juce::Colour(0xffe06040), 0.9f, false, false, false,
        {{0, 16, "Drum Loop", juce::Colour(0xffe06040)}, {16, 16, "Fill", juce::Colour(0xffcc5030)}}});
    tracks.push_back({"Bass", juce::Colour(0xff40a0e0), 0.75f, false, false, false,
        {{0, 8, "Intro Bass", juce::Colour(0xff40a0e0)}, {8, 24, "Main Bass", juce::Colour(0xff3090d0)}}});
    tracks.push_back({"Keys", juce::Colour(0xff60cc60), 0.7f, false, false, false,
        {{4, 12, "Chord Pad", juce::Colour(0xff60cc60)}, {20, 12, "Stab", juce::Colour(0xff50bb50)}}});
    tracks.push_back({"Lead", juce::Colour(0xffcc60cc), 0.65f, false, false, false,
        {{8, 8, "Melody A", juce::Colour(0xffcc60cc)}, {24, 8, "Melody B", juce::Colour(0xffbb50bb)}}});
    tracks.push_back({"Vocals", juce::Colour(0xffcccc40), 0.8f, false, false, false,
        {{4, 28, "Main Vocal", juce::Colour(0xffcccc40)}}});
    tracks.push_back({"FX", juce::Colour(0xff40cccc), 0.5f, false, false, false,
        {{0, 4, "Riser", juce::Colour(0xff40cccc)}, {15, 2, "Impact", juce::Colour(0xff30bbbb)}}});

    // Time ruler
    addAndMakeVisible(timeRuler);

    // Track headers
    trackHeaders.setTracks(tracks);
    trackHeaders.onTrackSelected = [this](int idx) { selectedTrack = idx; };
    addAndMakeVisible(trackHeaders);

    // Track content in viewport
    trackContent.setTracks(tracks);
    trackContentViewport.setViewedComponent(&trackContent, false);
    trackContentViewport.setScrollBarsShown(true, true);
    addAndMakeVisible(trackContentViewport);

    // Bottom panel
    addAndMakeVisible(bottomPanel);

    // Resize handle
    splitResizer.onDrag = [this](int delta) {
        bottomPanelHeight = juce::jlimit(ProLayoutConfig::bottomPanelMinHeight,
                                          getHeight() - 200, bottomPanelHeight + delta);
        resized();
    };
    addAndMakeVisible(splitResizer);

    startTimerHz(30);
}

ProMainView::~ProMainView() {
    stopTimer();
}

void ProMainView::paint(juce::Graphics& g) {
    g.fillAll(DarkTheme::getColour(DarkTheme::BACKGROUND));
}

void ProMainView::resized() {
    auto bounds = getLocalBounds();

    // Bottom panel + resizer
    auto bottomArea = bounds.removeFromBottom(bottomPanelHeight);
    splitResizer.setBounds(bounds.removeFromBottom(ProLayoutConfig::resizeHandleHeight));
    bottomPanel.setBounds(bottomArea);

    // Time ruler
    auto rulerArea = bounds.removeFromTop(ProLayoutConfig::timeRulerHeight);
    rulerArea.removeFromLeft(ProLayoutConfig::trackHeaderWidth);
    timeRuler.setBounds(rulerArea);

    // Track headers (left)
    auto headersArea = bounds.removeFromLeft(ProLayoutConfig::trackHeaderWidth);
    trackHeaders.setBounds(headersArea);

    // Track content (right, scrollable)
    trackContentViewport.setBounds(bounds);
    int contentHeight = (int)tracks.size() * ProLayoutConfig::defaultTrackHeight;
    int contentWidth = (int)(64.0 * pixelsPerBeat); // 64 beats visible
    trackContent.setSize(juce::jmax(contentWidth, bounds.getWidth()),
                         juce::jmax(contentHeight, bounds.getHeight()));
}

void ProMainView::timerCallback() {
    playheadBeat += (ProLayoutConfig::defaultBpm / 60.0) / 30.0;
    if (playheadBeat > 64.0) playheadBeat = 0.0;
    trackContent.setPlayheadBeat(playheadBeat);
}

void ProMainView::mouseWheelMove(const juce::MouseEvent& e,
                                  const juce::MouseWheelDetails& wheel) {
    if (e.mods.isCtrlDown()) {
        // Zoom
        pixelsPerBeat *= (1.0 + wheel.deltaY * 0.1);
        pixelsPerBeat = juce::jlimit(4.0, 100.0, pixelsPerBeat);
        timeRuler.setPixelsPerBeat(pixelsPerBeat);
        trackContent.setPixelsPerBeat(pixelsPerBeat);
        resized();
    } else {
        // Pass to viewport
        trackContentViewport.mouseWheelMove(e, wheel);
    }
}

}  // namespace aidaw
