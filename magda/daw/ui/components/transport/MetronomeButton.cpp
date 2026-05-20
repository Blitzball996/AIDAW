#include "MetronomeButton.hpp"

#include "../../themes/DarkTheme.hpp"
#include "../../themes/FontManager.hpp"

namespace magda {

MetronomeButton::MetronomeButton() {
    setTooltip("Metronome (right-click for settings)");
}

MetronomeButton::~MetronomeButton() = default;

void MetronomeButton::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    // Background
    if (enabled_) {
        g.setColour(DarkTheme::getColour(DarkTheme::ACCENT_BLUE).withAlpha(0.3f));
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(DarkTheme::getColour(DarkTheme::ACCENT_BLUE));
    } else {
        g.setColour(DarkTheme::getColour(DarkTheme::SURFACE));
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(DarkTheme::getColour(DarkTheme::TEXT_SECONDARY));
    }

    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

    // Draw metronome icon (simplified triangle/pendulum)
    auto iconBounds = bounds.reduced(4.0f);
    float cx = iconBounds.getCentreX();
    float bottom = iconBounds.getBottom();
    float top = iconBounds.getY();

    // Triangle base
    juce::Path triangle;
    triangle.addTriangle(cx - 6.0f, bottom, cx + 6.0f, bottom, cx, top + 2.0f);
    g.strokePath(triangle, juce::PathStrokeType(1.5f));

    // Pendulum line
    float pendulumAngle = enabled_ ? 0.3f : 0.0f;
    float px = cx + std::sin(pendulumAngle) * (bottom - top) * 0.5f;
    float py = top + (bottom - top) * 0.3f;
    g.drawLine(cx, bottom - 3.0f, px, py, 1.5f);

    // Dot at pendulum tip
    g.fillEllipse(px - 2.0f, py - 2.0f, 4.0f, 4.0f);
}

void MetronomeButton::resized() {}

void MetronomeButton::mouseDown(const juce::MouseEvent& e) {
    if (e.mods.isRightButtonDown() || e.mods.isPopupMenu()) {
        showSettingsMenu();
    } else {
        enabled_ = !enabled_;
        if (onToggle) onToggle(enabled_);
        repaint();
    }
}

void MetronomeButton::setEnabled(bool enabled) {
    if (enabled_ != enabled) {
        enabled_ = enabled;
        repaint();
    }
}

void MetronomeButton::setVolume(float volume) {
    volume_ = juce::jlimit(0.0f, 1.0f, volume);
}

void MetronomeButton::setCountIn(int bars) {
    countInBars_ = juce::jmax(0, bars);
}

void MetronomeButton::setSubdivision(int subdivision) {
    subdivision_ = juce::jlimit(1, 4, subdivision);
}

void MetronomeButton::showSettingsMenu() {
    juce::PopupMenu menu;

    // Volume submenu
    juce::PopupMenu volumeMenu;
    volumeMenu.addItem(100, "25%", true, volume_ < 0.3f);
    volumeMenu.addItem(101, "50%", true, volume_ >= 0.3f && volume_ < 0.6f);
    volumeMenu.addItem(102, "75%", true, volume_ >= 0.6f && volume_ < 0.9f);
    volumeMenu.addItem(103, "100%", true, volume_ >= 0.9f);
    menu.addSubMenu("Volume", volumeMenu);

    // Count-in submenu
    juce::PopupMenu countInMenu;
    countInMenu.addItem(200, "Off", true, countInBars_ == 0);
    countInMenu.addItem(201, "1 Bar", true, countInBars_ == 1);
    countInMenu.addItem(202, "2 Bars", true, countInBars_ == 2);
    countInMenu.addItem(203, "4 Bars", true, countInBars_ == 4);
    menu.addSubMenu("Count-In", countInMenu);

    // Subdivision submenu
    juce::PopupMenu subdivMenu;
    subdivMenu.addItem(300, "Quarter Notes", true, subdivision_ == 1);
    subdivMenu.addItem(301, "8th Notes", true, subdivision_ == 2);
    subdivMenu.addItem(302, "16th Notes", true, subdivision_ == 4);
    menu.addSubMenu("Subdivision", subdivMenu);

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
        [this](int result) {
            switch (result) {
                case 100: volume_ = 0.25f; break;
                case 101: volume_ = 0.5f; break;
                case 102: volume_ = 0.75f; break;
                case 103: volume_ = 1.0f; break;
                case 200: countInBars_ = 0; break;
                case 201: countInBars_ = 1; break;
                case 202: countInBars_ = 2; break;
                case 203: countInBars_ = 4; break;
                case 300: subdivision_ = 1; break;
                case 301: subdivision_ = 2; break;
                case 302: subdivision_ = 4; break;
                default: return;
            }

            if (result >= 100 && result < 200 && onVolumeChanged)
                onVolumeChanged(volume_);
            else if (result >= 200 && result < 300 && onCountInChanged)
                onCountInChanged(countInBars_);
            else if (result >= 300 && onSubdivisionChanged)
                onSubdivisionChanged(subdivision_);
        });
}

}  // namespace magda
