#pragma once

#include <juce_core/juce_core.h>

namespace aidaw {

// Default tempo/time-signature constants (standalone, no external dependency)
inline constexpr double kDefaultBPM = 120.0;
inline constexpr int kDefaultTimeSignatureNumerator = 4;
inline constexpr int kDefaultTimeSignatureDenominator = 4;
inline constexpr const char* kAidawVersion = "0.1.0";

/**
 * @brief Project metadata and settings
 *
 * Contains all project-level information including tempo, time signature,
 * loop settings, and file path.
 */
struct ProjectInfo {
    juce::String name;
    juce::String filePath;

    // Playback settings
    double tempo = kDefaultBPM;
    int timeSignatureNumerator = kDefaultTimeSignatureNumerator;
    int timeSignatureDenominator = kDefaultTimeSignatureDenominator;
    double projectLength = 240.0;  // seconds
    double sampleRate = 44100.0;

    // Key signature
    int keyRoot = -1;    // 0=C, 1=C#, ..., 11=B; -1=none
    int keyQuality = 0;  // 0=major, 1=minor

    // Loop settings (beats are authoritative, seconds derived from tempo)
    bool loopEnabled = false;
    double loopStartBeats = 0.0;
    double loopEndBeats = 0.0;

    // Zoom/scroll state
    double horizontalZoom = -1.0;  // Pixels per beat (-1 = use default)
    double verticalZoom = 1.0;     // Track height multiplier
    int scrollX = 0;               // Horizontal scroll position
    int scrollY = 0;               // Vertical scroll position

    // Active view (0=Live/Session, 1=Arrange, 2=Mix, 3=Master)
    int activeView = 1;  // Default to Arrange

    // Version tracking
    juce::String version = kAidawVersion;
    juce::Time lastModified;

    // Parameter aliases (UserProject layer, opaque JSON blob managed by AliasRegistry)
    juce::var paramAliases;

    // Project-scope bindings (opaque JSON blob managed by BindingRegistry)
    juce::var projectBindings;

    // Default constructor
    ProjectInfo() : lastModified(juce::Time::getCurrentTime()) {}

    // Helper to update modification time
    void touch() {
        lastModified = juce::Time::getCurrentTime();
    }
};

}  // namespace aidaw
