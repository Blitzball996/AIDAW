#include "BeatBoxContent.hpp"

#include "../../themes/DarkTheme.hpp"
#include "../../themes/FontManager.hpp"

namespace magda::daw::ui {

const std::array<juce::String, BeatBoxContent::NUM_INSTRUMENTS>
    BeatBoxContent::instrumentNames_ = {
        "Kick", "Snare", "Closed HH", "Open HH",
        "Low Tom", "Mid Tom", "Hi Tom", "Clap"
};

BeatBoxContent::BeatBoxContent() {
    // Play button
    playButton_.setButtonText("Play");
    playButton_.onClick = [this]() {
        playing_ = !playing_;
        playButton_.setButtonText(playing_ ? "Stop" : "Play");
        if (onPlayStateChanged) onPlayStateChanged(playing_);
    };
    addAndMakeVisible(playButton_);

    // Clear button
    clearButton_.setButtonText("Clear");
    clearButton_.onClick = [this]() { clearAll(); };
    addAndMakeVisible(clearButton_);

    // Pattern selector
    patternSelector_.addItem("Pattern 1", 1);
    patternSelector_.addItem("Pattern 2", 2);
    patternSelector_.addItem("Pattern 3", 3);
    patternSelector_.addItem("Pattern 4", 4);
    patternSelector_.addItem("Four on Floor", 5);
    patternSelector_.addItem("Breakbeat", 6);
    patternSelector_.addItem("Bossa Nova", 7);
    patternSelector_.addItem("Shuffle", 8);
    patternSelector_.setSelectedId(1, juce::dontSendNotification);
    patternSelector_.onChange = [this]() {
        if (onPatternSelected) onPatternSelected(patternSelector_.getSelectedId() - 1);
    };
    addAndMakeVisible(patternSelector_);

    // Swing
    swingLabel_.setText("Swing:", juce::dontSendNotification);
    swingLabel_.setFont(FontManager::getInstance().getUIFont(11.0f));
    addAndMakeVisible(swingLabel_);

    swingSlider_.setRange(0.0, 1.0, 0.01);
    swingSlider_.setValue(0.0);
    swingSlider_.setSliderStyle(juce::Slider::Rotary);
    swingSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 18);
    swingSlider_.onValueChange = [this]() {
        swing_ = static_cast<float>(swingSlider_.getValue());
        if (onSwingChanged) onSwingChanged(swing_);
    };
    addAndMakeVisible(swingSlider_);

    // Initialize grid to empty
    for (auto& row : grid_)
        row.fill(0);
}

BeatBoxContent::~BeatBoxContent() = default;

void BeatBoxContent::paint(juce::Graphics& g) {
    g.fillAll(DarkTheme::getColour(DarkTheme::PANEL_BACKGROUND));

    auto bounds = getLocalBounds();
    auto gridArea = bounds.withTrimmedTop(HEADER_HEIGHT);

    // Draw step numbers header
    g.setFont(FontManager::getInstance().getUIFont(9.0f));
    g.setColour(DarkTheme::getColour(DarkTheme::TEXT_SECONDARY));
    for (int step = 0; step < NUM_STEPS; ++step) {
        auto cellRect = getCellRect(step, -1);
        if (cellRect.getRight() <= bounds.getRight()) {
            auto headerRect = juce::Rectangle<int>(
                cellRect.getX(), HEADER_HEIGHT - 14, CELL_SIZE, 14);
            g.drawText(juce::String(step + 1), headerRect, juce::Justification::centred);
        }
    }

    // Draw instrument labels
    g.setFont(FontManager::getInstance().getUIFont(11.0f));
    for (int inst = 0; inst < NUM_INSTRUMENTS; ++inst) {
        int y = HEADER_HEIGHT + inst * CELL_SIZE;
        auto labelRect = juce::Rectangle<int>(4, y, LABEL_WIDTH - 8, CELL_SIZE);
        g.setColour(DarkTheme::getColour(DarkTheme::TEXT_PRIMARY));
        g.drawText(instrumentNames_[static_cast<size_t>(inst)], labelRect,
                   juce::Justification::centredLeft);
    }

    // Draw grid cells
    for (int step = 0; step < NUM_STEPS; ++step) {
        for (int inst = 0; inst < NUM_INSTRUMENTS; ++inst) {
            auto cellRect = getCellRect(step, inst);
            if (cellRect.getRight() > bounds.getRight()) continue;

            // Background - alternate groups of 4
            bool isGroupB = (step / 4) % 2 == 1;
            g.setColour(isGroupB ? juce::Colour(0xFF2A2A2A) : juce::Colour(0xFF222222));
            g.fillRect(cellRect.reduced(1));

            // Active step highlight
            if (grid_[static_cast<size_t>(step)][static_cast<size_t>(inst)] > 0) {
                float alpha = static_cast<float>(
                    grid_[static_cast<size_t>(step)][static_cast<size_t>(inst)]) / 127.0f;
                g.setColour(DarkTheme::getColour(DarkTheme::ACCENT_BLUE).withAlpha(alpha * 0.8f + 0.2f));
                g.fillRoundedRectangle(cellRect.reduced(3).toFloat(), 2.0f);
            }

            // Current step indicator
            if (step == currentStep_ && playing_) {
                g.setColour(juce::Colours::white.withAlpha(0.15f));
                g.fillRect(cellRect);
            }

            // Cell border
            g.setColour(DarkTheme::getColour(DarkTheme::BORDER).withAlpha(0.3f));
            g.drawRect(cellRect);
        }
    }

    // Beat separators (every 4 steps)
    g.setColour(DarkTheme::getColour(DarkTheme::BORDER));
    for (int beat = 0; beat <= 4; ++beat) {
        int x = LABEL_WIDTH + beat * 4 * CELL_SIZE;
        g.drawVerticalLine(x, static_cast<float>(HEADER_HEIGHT),
                           static_cast<float>(HEADER_HEIGHT + NUM_INSTRUMENTS * CELL_SIZE));
    }
}

void BeatBoxContent::resized() {
    auto bounds = getLocalBounds();
    auto headerArea = bounds.removeFromTop(HEADER_HEIGHT).reduced(4, 4);

    playButton_.setBounds(headerArea.removeFromLeft(50));
    headerArea.removeFromLeft(6);
    clearButton_.setBounds(headerArea.removeFromLeft(50));
    headerArea.removeFromLeft(12);
    patternSelector_.setBounds(headerArea.removeFromLeft(120));
    headerArea.removeFromLeft(12);
    swingLabel_.setBounds(headerArea.removeFromLeft(40));
    swingSlider_.setBounds(headerArea.removeFromLeft(80));
}

void BeatBoxContent::mouseDown(const juce::MouseEvent& e) {
    // Determine which cell was clicked
    auto pos = e.getPosition();
    for (int step = 0; step < NUM_STEPS; ++step) {
        for (int inst = 0; inst < NUM_INSTRUMENTS; ++inst) {
            if (getCellRect(step, inst).contains(pos)) {
                toggleCell(step, inst);
                return;
            }
        }
    }
}

juce::Rectangle<int> BeatBoxContent::getCellRect(int step, int instrument) const {
    int x = LABEL_WIDTH + step * CELL_SIZE;
    int y = HEADER_HEIGHT + (instrument >= 0 ? instrument * CELL_SIZE : 0);
    return {x, y, CELL_SIZE, CELL_SIZE};
}

void BeatBoxContent::toggleCell(int step, int instrument) {
    auto& vel = grid_[static_cast<size_t>(step)][static_cast<size_t>(instrument)];
    vel = (vel > 0) ? 0 : 100;  // Toggle with default velocity 100

    if (onStepToggled) onStepToggled(step, instrument, vel);
    repaint();
}

void BeatBoxContent::setStep(int step, int instrument, int velocity) {
    if (step >= 0 && step < NUM_STEPS && instrument >= 0 && instrument < NUM_INSTRUMENTS) {
        grid_[static_cast<size_t>(step)][static_cast<size_t>(instrument)] = velocity;
        repaint();
    }
}

int BeatBoxContent::getStepVelocity(int step, int instrument) const {
    if (step >= 0 && step < NUM_STEPS && instrument >= 0 && instrument < NUM_INSTRUMENTS)
        return grid_[static_cast<size_t>(step)][static_cast<size_t>(instrument)];
    return 0;
}

void BeatBoxContent::clearAll() {
    for (auto& row : grid_) row.fill(0);
    repaint();
}

void BeatBoxContent::setPlaying(bool playing) {
    playing_ = playing;
    playButton_.setButtonText(playing_ ? "Stop" : "Play");
    if (!playing_) currentStep_ = -1;
    repaint();
}

void BeatBoxContent::setCurrentStep(int step) {
    if (currentStep_ != step) {
        currentStep_ = step;
        repaint();
    }
}

void BeatBoxContent::setSwing(float swing) {
    swing_ = juce::jlimit(0.0f, 1.0f, swing);
    swingSlider_.setValue(static_cast<double>(swing_), juce::dontSendNotification);
}

}  // namespace magda::daw::ui
