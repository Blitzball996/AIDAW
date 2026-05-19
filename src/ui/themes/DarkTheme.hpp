#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace aidaw {

/**
 * Dark theme color palette for AIDAW.
 * Professional audio production interface colors.
 */
class DarkTheme {
public:
    // Background colors
    static constexpr auto BACKGROUND = 0xFF1A1A1A;
    static constexpr auto BACKGROUND_ALT = 0xFF1E1E22;
    static constexpr auto PANEL_BACKGROUND = 0xFF252530;
    static constexpr auto SURFACE = 0xFF2A2A35;
    static constexpr auto SURFACE_HOVER = 0xFF333333;

    // Transport and controls
    static constexpr auto TRANSPORT_BACKGROUND = 0xFF1E1E22;
    static constexpr auto BUTTON_NORMAL = 0xFF1A1A1A;
    static constexpr auto BUTTON_HOVER = 0xFF2A2A35;
    static constexpr auto BUTTON_PRESSED = 0xFF333333;
    static constexpr auto BUTTON_ACTIVE = 0xFF5588AA;
    static constexpr auto BUTTON_STROKE = 0xFF444444;

    // Text colors
    static constexpr auto TEXT_PRIMARY = 0xFFDDDDDD;
    static constexpr auto TEXT_SECONDARY = 0xFFAABBCC;
    static constexpr auto TEXT_DIM = 0xFF888888;
    static constexpr auto TEXT_DISABLED = 0xFF666666;

    // Accent colors
    static constexpr auto ACCENT_BLUE = 0xFF5588AA;
    static constexpr auto ACCENT_BLUE_LIGHT = 0xFF88AACC;
    static constexpr auto ACCENT_CYAN = 0xFF66AAFF;
    static constexpr auto ACCENT_GREEN = 0xFF33E680;
    static constexpr auto ACCENT_ORANGE = 0xFFFF8822;
    static constexpr auto ACCENT_PURPLE = 0xFF7777DD;

    // Status colors
    static constexpr auto STATUS_SUCCESS = 0xFF44AA44;
    static constexpr auto STATUS_WARNING = 0xFFFFAA44;
    static constexpr auto STATUS_ERROR = 0xFFAA4444;
    static constexpr auto STATUS_DANGER = 0xFFFF6644;

    // Track colors
    static constexpr auto TRACK_BACKGROUND = 0xFF1E1E22;
    static constexpr auto TRACK_SELECTED = 0xFF2A2A35;
    static constexpr auto TRACK_SEPARATOR = 0xFF1A1A1A;

    // Timeline and grid
    static constexpr auto TIMELINE_BACKGROUND = 0xFF1E1E22;
    static constexpr auto GRID_LINE = 0xFF383840;
    static constexpr auto BEAT_LINE = 0xFF484850;
    static constexpr auto BAR_LINE = 0xFF555555;

    // Borders and separators
    static constexpr auto BORDER = 0xFF444444;
    static constexpr auto SEPARATOR = 0xFF333333;
    static constexpr auto RESIZE_HANDLE = 0xFF555555;

    // Audio visualization
    static constexpr auto WAVEFORM_NORMAL = 0xFF33E680;
    static constexpr auto WAVEFORM_SELECTED = 0xFF66AAFF;
    static constexpr auto LEVEL_METER_GREEN = 0xFF44AA44;
    static constexpr auto LEVEL_METER_YELLOW = 0xFFFFAA44;
    static constexpr auto LEVEL_METER_RED = 0xFFAA4444;

    // Selection and loop regions
    static constexpr auto TIME_SELECTION = 0x335588AA;
    static constexpr auto LOOP_REGION = 0x08FFFFFF;
    static constexpr auto LOOP_MARKER = 0xFF44AA66;

    // Apply the theme to JUCE's LookAndFeel
    static void applyToLookAndFeel(juce::LookAndFeel_V4& laf);

    // Get color as JUCE Colour object
    static juce::Colour getColour(juce::uint32 colorValue) {
        return juce::Colour(colorValue);
    }

    // Helper methods
    static juce::Colour getBackgroundColour() { return getColour(BACKGROUND); }
    static juce::Colour getPanelBackgroundColour() { return getColour(PANEL_BACKGROUND); }
    static juce::Colour getTextColour() { return getColour(TEXT_PRIMARY); }
    static juce::Colour getSecondaryTextColour() { return getColour(TEXT_SECONDARY); }
    static juce::Colour getAccentColour() { return getColour(ACCENT_BLUE); }
    static juce::Colour getBorderColour() { return getColour(BORDER); }
};

}  // namespace aidaw
