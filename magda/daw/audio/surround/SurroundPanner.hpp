#pragma once

#include <juce_core/juce_core.h>

#include <array>
#include <vector>

namespace magda {

/**
 * @brief Supported surround sound formats.
 */
enum class SurroundFormat {
    Stereo,      ///< 2.0 — L, R
    Quad,        ///< 4.0 — L, R, Ls, Rs
    Surround51,  ///< 5.1 — L, R, C, LFE, Ls, Rs
    Surround71,  ///< 7.1 — L, R, C, LFE, Ls, Rs, Lrs, Rrs
    Atmos714     ///< 7.1.4 — 7.1 + Ltf, Rtf, Ltr, Rtr (12 channels)
};

/**
 * @brief Returns the number of output channels for a given surround format.
 */
inline int getChannelCountForFormat(SurroundFormat format)
{
    switch (format)
    {
        case SurroundFormat::Stereo:     return 2;
        case SurroundFormat::Quad:       return 4;
        case SurroundFormat::Surround51: return 6;
        case SurroundFormat::Surround71: return 8;
        case SurroundFormat::Atmos714:   return 12;
    }
    return 2;
}

/**
 * @brief Speaker channel indices for 7.1.4 (superset layout).
 */
namespace SpeakerChannel {
    constexpr int Left         = 0;
    constexpr int Right        = 1;
    constexpr int Centre       = 2;
    constexpr int LFE          = 3;
    constexpr int LeftSurround = 4;
    constexpr int RightSurround = 5;
    constexpr int LeftRearSurround  = 6;
    constexpr int RightRearSurround = 7;
    // Height channels (Atmos 7.1.4)
    constexpr int LeftTopFront  = 8;
    constexpr int RightTopFront = 9;
    constexpr int LeftTopRear   = 10;
    constexpr int RightTopRear  = 11;
}  // namespace SpeakerChannel

/**
 * @brief 3D position in spherical coordinates for surround panning.
 */
struct SurroundPosition {
    float azimuth   = 0.0f;  ///< Horizontal angle in degrees (-180 to +180, 0 = front)
    float elevation = 0.0f;  ///< Vertical angle in degrees (-90 to +90, 0 = ear level)
    float distance  = 1.0f;  ///< Normalised distance (0 = centre, 1 = speaker ring)
};

/**
 * @brief Pans a mono or stereo source into a surround speaker layout.
 *
 * Uses Vector Base Amplitude Panning (VBAP) principles to distribute
 * a source signal across the available channels based on its 3D position.
 * Supports LFE routing with a configurable send level.
 */
class SurroundPanner {
  public:
    explicit SurroundPanner(SurroundFormat format = SurroundFormat::Surround51);
    ~SurroundPanner() = default;

    // --- Configuration ---

    /** Set the surround format (recalculates gains). */
    void setFormat(SurroundFormat format);

    /** Get the current surround format. */
    SurroundFormat getFormat() const { return format_; }

    // --- Position ---

    /** Set the source position in 3D space. */
    void setPosition(float azimuth, float elevation, float distance);

    /** Set position from a SurroundPosition struct. */
    void setPosition(const SurroundPosition& pos);

    /** Get the current source position. */
    SurroundPosition getPosition() const { return position_; }

    // --- LFE ---

    /** Set the LFE send level (0.0 to 1.0). */
    void setLFESend(float level);

    /** Get the current LFE send level. */
    float getLFESend() const { return lfeSend_; }

    // --- Output ---

    /**
     * @brief Get the per-channel gain coefficients.
     * @return Vector of gains, one per output channel in the current format.
     *         Gains are normalised so that total power is preserved.
     */
    std::vector<float> getGains() const;

    /** Get the number of output channels for the current format. */
    int getNumOutputChannels() const { return getChannelCountForFormat(format_); }

  private:
    void recalculateGains();

    SurroundFormat format_;
    SurroundPosition position_;
    float lfeSend_ = 0.0f;
    std::vector<float> gains_;
};

}  // namespace magda
