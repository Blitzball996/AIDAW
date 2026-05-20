#include "SendKnob.hpp"

#include "../../themes/DarkTheme.hpp"
#include "../../themes/FontManager.hpp"

namespace magda {

SendKnob::SendKnob() {
    setTooltip("Send level");
}

SendKnob::~SendKnob() = default;

void SendKnob::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    float knobSize = juce::jmin(bounds.getWidth(), bounds.getHeight() - 14.0f);
    auto knobArea = bounds.removeFromTop(knobSize).withSizeKeepingCentre(knobSize, knobSize);

    // Knob background
    g.setColour(DarkTheme::getColour(DarkTheme::SURFACE));
    g.fillEllipse(knobArea.reduced(2.0f));

    // Knob border
    g.setColour(DarkTheme::getColour(DarkTheme::BORDER));
    g.drawEllipse(knobArea.reduced(2.0f), 1.0f);

    // Arc showing level
    float startAngle = juce::MathConstants<float>::pi * 1.25f;
    float endAngle = juce::MathConstants<float>::pi * 2.75f;
    float currentAngle = startAngle + level_ * (endAngle - startAngle);

    juce::Path arcPath;
    arcPath.addCentredArc(knobArea.getCentreX(), knobArea.getCentreY(),
                          knobSize * 0.4f, knobSize * 0.4f,
                          0.0f, startAngle, currentAngle, true);

    g.setColour(level_ > 0.0f ? DarkTheme::getColour(DarkTheme::ACCENT_BLUE)
                               : DarkTheme::getColour(DarkTheme::TEXT_DISABLED));
    g.strokePath(arcPath, juce::PathStrokeType(2.5f));

    // Indicator line
    float indicatorLength = knobSize * 0.3f;
    float ix = knobArea.getCentreX() + std::sin(currentAngle - juce::MathConstants<float>::pi) * indicatorLength;
    float iy = knobArea.getCentreY() - std::cos(currentAngle - juce::MathConstants<float>::pi) * indicatorLength;
    g.setColour(DarkTheme::getColour(DarkTheme::TEXT_PRIMARY));
    g.drawLine(knobArea.getCentreX(), knobArea.getCentreY(), ix, iy, 1.5f);

    // Label below knob
    auto labelArea = getLocalBounds().removeFromBottom(14);
    g.setFont(FontManager::getInstance().getUIFont(9.0f));

    // Pre/Post indicator
    juce::String labelText = preFader_ ? "PRE" : "POST";
    g.setColour(preFader_ ? juce::Colour(0xFFFF9800) : DarkTheme::getColour(DarkTheme::TEXT_SECONDARY));
    g.drawText(labelText, labelArea, juce::Justification::centred);
}

void SendKnob::resized() {}

void SendKnob::mouseDown(const juce::MouseEvent& e) {
    if (e.mods.isRightButtonDown() || e.mods.isPopupMenu()) {
        juce::PopupMenu menu;
        menu.addItem(1, "Pre-Fader", true, preFader_);
        menu.addItem(2, "Post-Fader", true, !preFader_);
        menu.addSeparator();
        menu.addItem(3, "Remove Send");

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
            [this](int result) {
                if (result == 1) {
                    preFader_ = true;
                    if (onPreFaderToggled) onPreFaderToggled(true);
                    repaint();
                } else if (result == 2) {
                    preFader_ = false;
                    if (onPreFaderToggled) onPreFaderToggled(false);
                    repaint();
                } else if (result == 3) {
                    if (onRemoveRequested) onRemoveRequested();
                }
            });
        return;
    }

    // Check if click is on the label area (toggle pre/post)
    auto labelArea = getLocalBounds().removeFromBottom(14);
    if (labelArea.contains(e.getPosition())) {
        preFader_ = !preFader_;
        if (onPreFaderToggled) onPreFaderToggled(preFader_);
        repaint();
        return;
    }

    // Start knob drag
    dragStartLevel_ = level_;
    dragStartY_ = e.y;
    isDragging_ = true;
}

void SendKnob::mouseDrag(const juce::MouseEvent& e) {
    if (!isDragging_) return;

    float sensitivity = e.mods.isShiftDown() ? 400.0f : 100.0f;
    float delta = static_cast<float>(dragStartY_ - e.y) / sensitivity;
    float newLevel = juce::jlimit(0.0f, 1.0f, dragStartLevel_ + delta);

    if (std::abs(newLevel - level_) > 0.001f) {
        level_ = newLevel;
        if (onLevelChanged) onLevelChanged(level_);
        repaint();
    }
}

void SendKnob::mouseUp(const juce::MouseEvent&) {
    isDragging_ = false;
}

void SendKnob::mouseDoubleClick(const juce::MouseEvent&) {
    // Reset to 0
    level_ = 0.0f;
    if (onLevelChanged) onLevelChanged(level_);
    repaint();
}

void SendKnob::setLevel(float level) {
    level_ = juce::jlimit(0.0f, 1.0f, level);
    repaint();
}

void SendKnob::setPreFader(bool preFader) {
    preFader_ = preFader;
    repaint();
}

void SendKnob::setSendName(const juce::String& name) {
    sendName_ = name;
}

void SendKnob::setDestinationName(const juce::String& name) {
    destName_ = name;
    setTooltip("Send to: " + name);
}

}  // namespace magda
