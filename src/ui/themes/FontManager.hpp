#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "DarkTheme.hpp"

namespace aidaw {

/**
 * Font manager singleton for consistent typography across the UI.
 * Provides Inter font family with multiple weights.
 */
class FontManager {
public:
    enum class Weight { Regular, Medium, SemiBold, Bold };

    static FontManager& getInstance();

    // Initialize fonts (call once at startup)
    bool initialize();

    // Shutdown and release fonts
    void shutdown();

    // Get font with specified weight and size
    juce::Font getFont(float size, Weight weight = Weight::Regular) const;

    // Convenience methods
    juce::Font getUIFont(float size = 14.0f) const;
    juce::Font getUIFontMedium(float size = 14.0f) const;
    juce::Font getUIFontBold(float size = 14.0f) const;
    juce::Font getHeadingFont(float size = 18.0f) const;
    juce::Font getMonoFont(float size = 13.0f) const;

    bool isInitialized() const { return initialized_; }

private:
    FontManager() = default;
    ~FontManager() = default;

    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    bool initialized_ = false;

    juce::Typeface::Ptr regularTypeface_;
    juce::Typeface::Ptr mediumTypeface_;
    juce::Typeface::Ptr semiBoldTypeface_;
    juce::Typeface::Ptr boldTypeface_;
    juce::Typeface::Ptr monoTypeface_;

    static constexpr const char* FALLBACK_FONT = "Helvetica";
};

}  // namespace aidaw
