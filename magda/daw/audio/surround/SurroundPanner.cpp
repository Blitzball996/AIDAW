#include "SurroundPanner.hpp"

#include <algorithm>
#include <cmath>

namespace magda {

namespace {

constexpr float kPi = 3.14159265358979323846f;

/** Convert degrees to radians. */
inline float degToRad(float degrees) { return degrees * (kPi / 180.0f); }

/**
 * Speaker azimuth angles for each format (degrees, 0 = front centre).
 * Only bed-layer speakers; height channels handled separately.
 */
struct SpeakerLayout {
    float azimuth;
    float elevation;
};

// 7.1.4 full layout (superset)
static const std::array<SpeakerLayout, 12> kAtmos714Layout = {{
    {  -30.0f,  0.0f },  // L
    {   30.0f,  0.0f },  // R
    {    0.0f,  0.0f },  // C
    {    0.0f,  0.0f },  // LFE (not spatially positioned)
    { -110.0f,  0.0f },  // Ls
    {  110.0f,  0.0f },  // Rs
    { -150.0f,  0.0f },  // Lrs
    {  150.0f,  0.0f },  // Rrs
    {  -45.0f, 45.0f },  // Ltf
    {   45.0f, 45.0f },  // Rtf
    { -135.0f, 45.0f },  // Ltr
    {  135.0f, 45.0f },  // Rtr
}};

/**
 * Compute angular distance between source and speaker in 3D.
 */
float angularDistance(float srcAz, float srcEl, float spkAz, float spkEl)
{
    float srcAzRad = degToRad(srcAz);
    float srcElRad = degToRad(srcEl);
    float spkAzRad = degToRad(spkAz);
    float spkElRad = degToRad(spkEl);

    // Spherical law of cosines
    float cosAngle = std::sin(srcElRad) * std::sin(spkElRad) +
                     std::cos(srcElRad) * std::cos(spkElRad) *
                     std::cos(srcAzRad - spkAzRad);

    return std::acos(std::clamp(cosAngle, -1.0f, 1.0f));
}

}  // namespace

// ============================================================================

SurroundPanner::SurroundPanner(SurroundFormat format)
    : format_(format)
{
    gains_.resize(static_cast<size_t>(getChannelCountForFormat(format_)), 0.0f);
    recalculateGains();
}

void SurroundPanner::setFormat(SurroundFormat format)
{
    format_ = format;
    gains_.resize(static_cast<size_t>(getChannelCountForFormat(format_)), 0.0f);
    recalculateGains();
}

void SurroundPanner::setPosition(float azimuth, float elevation, float distance)
{
    position_.azimuth   = std::clamp(azimuth, -180.0f, 180.0f);
    position_.elevation = std::clamp(elevation, -90.0f, 90.0f);
    position_.distance  = std::clamp(distance, 0.0f, 1.0f);
    recalculateGains();
}

void SurroundPanner::setPosition(const SurroundPosition& pos)
{
    setPosition(pos.azimuth, pos.elevation, pos.distance);
}

void SurroundPanner::setLFESend(float level)
{
    lfeSend_ = std::clamp(level, 0.0f, 1.0f);
    recalculateGains();
}

std::vector<float> SurroundPanner::getGains() const
{
    return gains_;
}

void SurroundPanner::recalculateGains()
{
    const int numChannels = getChannelCountForFormat(format_);
    gains_.assign(static_cast<size_t>(numChannels), 0.0f);

    // Distance attenuation (inverse distance, clamped)
    float distAtten = (position_.distance > 0.01f)
                          ? 1.0f / (1.0f + (position_.distance - 1.0f) * 0.5f)
                          : 1.0f;
    distAtten = std::clamp(distAtten, 0.0f, 1.0f);

    // Compute raw gains using inverse-distance weighting from angular distance
    float totalWeight = 0.0f;
    std::vector<float> rawGains(static_cast<size_t>(numChannels), 0.0f);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        // Skip LFE in spatial calculation (handled separately)
        if (format_ != SurroundFormat::Stereo && format_ != SurroundFormat::Quad &&
            ch == SpeakerChannel::LFE)
        {
            continue;
        }

        float spkAz = kAtmos714Layout[static_cast<size_t>(ch)].azimuth;
        float spkEl = kAtmos714Layout[static_cast<size_t>(ch)].elevation;

        // For formats without height, skip height channels
        if (format_ != SurroundFormat::Atmos714 && ch >= 8)
            continue;
        if (format_ == SurroundFormat::Surround71 && ch >= 8)
            continue;
        if (format_ == SurroundFormat::Surround51 && ch >= 6)
            continue;
        if (format_ == SurroundFormat::Quad && ch >= 4)
            continue;

        // Remap for Quad: channels are L, R, Ls, Rs (no C/LFE)
        if (format_ == SurroundFormat::Quad)
        {
            static const float quadAz[] = { -30.0f, 30.0f, -110.0f, 110.0f };
            spkAz = quadAz[ch];
            spkEl = 0.0f;
        }

        // Remap for Stereo
        if (format_ == SurroundFormat::Stereo)
        {
            static const float stereoAz[] = { -30.0f, 30.0f };
            spkAz = stereoAz[ch];
            spkEl = 0.0f;
        }

        float angle = angularDistance(position_.azimuth, position_.elevation, spkAz, spkEl);

        // Use cosine-law panning: gain = cos(angle/2)^2 for smooth falloff
        float gain = std::cos(angle * 0.5f);
        gain = std::max(0.0f, gain);
        gain *= gain;  // Square for energy preservation

        rawGains[static_cast<size_t>(ch)] = gain;
        totalWeight += gain * gain;  // Sum of squared gains for normalisation
    }

    // Normalise to preserve total power
    float normFactor = (totalWeight > 0.0001f)
                           ? 1.0f / std::sqrt(totalWeight)
                           : 0.0f;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        gains_[static_cast<size_t>(ch)] = rawGains[static_cast<size_t>(ch)] * normFactor * distAtten;
    }

    // Apply LFE send (independent of spatial position)
    if (numChannels > SpeakerChannel::LFE &&
        format_ != SurroundFormat::Stereo &&
        format_ != SurroundFormat::Quad)
    {
        gains_[SpeakerChannel::LFE] = lfeSend_ * distAtten;
    }
}

}  // namespace magda
