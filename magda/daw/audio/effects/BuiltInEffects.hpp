#pragma once

#include <juce_core/juce_core.h>

#include <array>

namespace magda {

/**
 * @brief Enumeration of built-in effect processors available in the DAW.
 */
enum class BuiltInEffect {
    ConvolutionReverb,
    Compressor,
    EQ,
    Delay,
    Chorus,
    Phaser,
    Limiter
};

/**
 * @brief Metadata for a built-in effect.
 */
struct BuiltInEffectInfo {
    BuiltInEffect type;
    juce::String name;
    juce::String category;
    juce::String description;
};

/**
 * @brief Registry of built-in effect processors.
 *
 * Provides a central catalogue of all native effects available in the DAW,
 * with metadata for UI display and instantiation.
 */
class BuiltInEffectRegistry {
  public:
    /** Get the singleton registry instance. */
    static const BuiltInEffectRegistry& getInstance()
    {
        static BuiltInEffectRegistry instance;
        return instance;
    }

    /** Get all registered effects. */
    const std::array<BuiltInEffectInfo, 7>& getAllEffects() const { return effects_; }

    /** Get info for a specific effect type. */
    const BuiltInEffectInfo& getEffectInfo(BuiltInEffect type) const
    {
        return effects_[static_cast<size_t>(type)];
    }

    /** Get the number of registered effects. */
    int getNumEffects() const { return static_cast<int>(effects_.size()); }

    /** Find an effect by name (case-insensitive). Returns nullptr if not found. */
    const BuiltInEffectInfo* findByName(const juce::String& name) const
    {
        for (const auto& effect : effects_)
        {
            if (effect.name.equalsIgnoreCase(name))
                return &effect;
        }
        return nullptr;
    }

  private:
    BuiltInEffectRegistry()
    {
        effects_ = {{
            { BuiltInEffect::ConvolutionReverb, "Convolution Reverb", "Reverb",
              "IR-based convolution reverb with pre-delay and decay controls" },
            { BuiltInEffect::Compressor, "Compressor", "Dynamics",
              "Dynamic range compressor with threshold, ratio, attack, and release" },
            { BuiltInEffect::EQ, "Parametric EQ", "EQ",
              "Multi-band parametric equalizer with variable filter types" },
            { BuiltInEffect::Delay, "Delay", "Time",
              "Stereo delay with tempo sync, feedback, and filtering" },
            { BuiltInEffect::Chorus, "Chorus", "Modulation",
              "Stereo chorus with rate, depth, and feedback controls" },
            { BuiltInEffect::Phaser, "Phaser", "Modulation",
              "Multi-stage phaser with rate and depth controls" },
            { BuiltInEffect::Limiter, "Limiter", "Dynamics",
              "Brickwall limiter with ceiling and release controls" },
        }};
    }

    std::array<BuiltInEffectInfo, 7> effects_;
};

}  // namespace magda
