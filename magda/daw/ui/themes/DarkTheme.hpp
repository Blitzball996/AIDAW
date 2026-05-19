#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace magda {

/**
 * Logic Pro X inspired dark theme for AIDAW
 * Warm gray tones with subtle blue accents, professional and clean
 */
class DarkTheme {
  public:
    // ==========================================================================
    // Background colors (Logic Pro warm dark grays)
    // ==========================================================================
    static constexpr auto BACKGROUND = 0xFF232323;        // Main background (warm dark gray)
    static constexpr auto BACKGROUND_ALT = 0xFF2B2B2B;    // Slightly lighter
    static constexpr auto PANEL_BACKGROUND = 0xFF303030;  // Panel background
    static constexpr auto SURFACE = 0xFF383838;           // Elevated surface
    static constexpr auto SURFACE_HOVER = 0xFF424242;     // Hovered surface

    // ==========================================================================
    // Transport and controls
    // ==========================================================================
    static constexpr auto TRANSPORT_BACKGROUND = 0xFF2D2D2D;  // Transport bar
    static constexpr auto BUTTON_NORMAL = 0xFF3A3A3A;         // Normal button
    static constexpr auto BUTTON_HOVER = 0xFF4A4A4A;          // Hovered button
    static constexpr auto BUTTON_PRESSED = 0xFF555555;        // Pressed button
    static constexpr auto BUTTON_ACTIVE = 0xFF4A90D9;         // Active (Logic blue)
    static constexpr auto BUTTON_STROKE = 0xFF505050;         // Button border

    // ==========================================================================
    // Text colors
    // ==========================================================================
    static constexpr auto TEXT_PRIMARY = 0xFFE8E8E8;    // Primary text (bright white)
    static constexpr auto TEXT_SECONDARY = 0xFFB0B0B0;  // Secondary text
    static constexpr auto TEXT_DIM = 0xFF808080;        // Dimmed text
    static constexpr auto TEXT_DISABLED = 0xFF5A5A5A;   // Disabled text

    // ==========================================================================
    // Accent colors (Logic Pro palette)
    // ==========================================================================
    static constexpr auto ACCENT_BLUE = 0xFF4A90D9;          // Primary accent (Logic blue)
    static constexpr auto ACCENT_BLUE_LIGHT = 0xFF6AAFEF;    // Light blue
    static constexpr auto ACCENT_CYAN = 0xFF5AC8FA;          // Cyan (selection)
    static constexpr auto ACCENT_GREEN = 0xFF4CD964;         // Green (enabled, play)
    static constexpr auto ACCENT_ORANGE = 0xFFFF9500;        // Orange (automation)
    static constexpr auto ACCENT_PURPLE = 0xFFAF52DE;        // Purple accent
    static constexpr auto MASTER_TRACK_COLOUR = 0xFF8E8E93;  // Master track (gray)

    // ==========================================================================
    // Status colors
    // ==========================================================================
    static constexpr auto STATUS_SUCCESS = 0xFF4CD964;  // Success (iOS green)
    static constexpr auto STATUS_WARNING = 0xFFFF9500;  // Warning (iOS orange)
    static constexpr auto STATUS_ERROR = 0xFFFF3B30;    // Error (iOS red)
    static constexpr auto STATUS_DANGER = 0xFFFF3B30;   // Record red

    // Backwards compatibility aliases
    static constexpr auto ACCENT_RED = STATUS_DANGER;  // Alias for STATUS_DANGER

    // ==========================================================================
    // Track colors (Logic Pro style - lighter, warmer)
    // ==========================================================================
    static constexpr auto TRACK_BACKGROUND = 0xFF2D2D2D;  // Track background
    static constexpr auto TRACK_SELECTED = 0xFF3D3D3D;    // Selected track
    static constexpr auto TRACK_SEPARATOR = 0xFF1F1F1F;   // Track separator lines

    // ==========================================================================
    // Timeline and grid (Logic Pro subtle grid)
    // ==========================================================================
    static constexpr auto TIMELINE_BACKGROUND = 0xFF282828;  // Timeline background
    static constexpr auto GRID_LINE = 0xFF353535;            // Grid lines (subtle)
    static constexpr auto BEAT_LINE = 0xFF404040;            // Beat lines
    static constexpr auto BAR_LINE = 0xFF4A4A4A;             // Bar lines

    // ==========================================================================
    // Borders and separators
    // ==========================================================================
    static constexpr auto BORDER = 0xFF4A4A4A;         // General borders
    static constexpr auto SEPARATOR = 0xFF3A3A3A;      // Panel separators
    static constexpr auto RESIZE_HANDLE = 0xFF606060;  // Resize handles

    // ==========================================================================
    // Audio visualization (Logic Pro green waveforms)
    // ==========================================================================
    static constexpr auto WAVEFORM_NORMAL = 0xFF4CD964;     // Waveform (Logic green)
    static constexpr auto WAVEFORM_SELECTED = 0xFF5AC8FA;   // Selected waveform (cyan)
    static constexpr auto LEVEL_METER_GREEN = 0xFF4CD964;   // Level meter (low)
    static constexpr auto LEVEL_METER_YELLOW = 0xFFFF9500;  // Level meter (mid)
    static constexpr auto LEVEL_METER_RED = 0xFFFF3B30;     // Level meter (high)

    // ==========================================================================
    // Selection and loop regions
    // ==========================================================================
    static constexpr auto TIME_SELECTION = 0x334A90D9;  // Semi-transparent blue
    static constexpr auto LOOP_REGION = 0x20FFFF00;     // Yellow tint for loop
    static constexpr auto LOOP_MARKER = 0xFFFFCC00;     // Yellow loop markers (Logic style)
    static constexpr auto OFFSET_MARKER = 0xFFFF9500;   // Orange offset marker

    // Apply the theme to JUCE's LookAndFeel
    static void applyToLookAndFeel(juce::LookAndFeel_V4& laf);

    // Get color as JUCE Colour object
    static juce::Colour getColour(juce::uint32 colorValue) {
        return juce::Colour(colorValue);
    }

    // Helper methods for common color combinations
    static juce::Colour getBackgroundColour() {
        return getColour(BACKGROUND);
    }
    static juce::Colour getPanelBackgroundColour() {
        return getColour(PANEL_BACKGROUND);
    }
    static juce::Colour getTextColour() {
        return getColour(TEXT_PRIMARY);
    }
    static juce::Colour getSecondaryTextColour() {
        return getColour(TEXT_SECONDARY);
    }
    static juce::Colour getAccentColour() {
        return getColour(ACCENT_BLUE);
    }
    static juce::Colour getBorderColour() {
        return getColour(BORDER);
    }
};

}  // namespace magda
