#pragma once

#include <string>
#include <vector>

namespace aidaw {

/**
 * @brief Interface for controlling mixing parameters
 *
 * The MixerInterface provides methods for controlling volume, pan,
 * and effects routing for tracks.
 */
class MixerInterface {
  public:
    virtual ~MixerInterface() = default;

    virtual void setTrackVolume(const std::string& track_id, double volume) = 0;
    virtual double getTrackVolume(const std::string& track_id) const = 0;

    virtual void setTrackPan(const std::string& track_id, double pan) = 0;
    virtual double getTrackPan(const std::string& track_id) const = 0;

    virtual void setMasterVolume(double volume) = 0;
    virtual double getMasterVolume() const = 0;

    /**
     * @brief Add an effect to a track
     * @param track_id The track ID
     * @param effect_name Name of the effect to add
     * @return Effect instance ID
     */
    virtual std::string addEffect(const std::string& track_id, const std::string& effect_name) = 0;

    virtual void removeEffect(const std::string& effect_id) = 0;

    /**
     * @brief Set effect parameter
     * @param effect_id The effect instance ID
     * @param parameter_name The parameter name
     * @param value The parameter value (normalized 0.0-1.0)
     */
    virtual void setEffectParameter(const std::string& effect_id, const std::string& parameter_name,
                                    double value) = 0;
    virtual double getEffectParameter(const std::string& effect_id,
                                      const std::string& parameter_name) const = 0;

    virtual void setEffectEnabled(const std::string& effect_id, bool enabled) = 0;
    virtual bool isEffectEnabled(const std::string& effect_id) const = 0;

    virtual std::vector<std::string> getAvailableEffects() const = 0;
    virtual std::vector<std::string> getTrackEffects(const std::string& track_id) const = 0;
};

}  // namespace aidaw
