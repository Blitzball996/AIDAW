#pragma once

#include "SurroundPanner.hpp"

#include <juce_audio_basics/juce_audio_basics.h>

#include <memory>
#include <mutex>
#include <vector>

namespace magda {

/**
 * @brief Describes the speaker layout for a surround bus.
 */
struct SpeakerLayoutInfo {
    SurroundFormat format;
    int numChannels;
    juce::StringArray channelNames;

    static SpeakerLayoutInfo create(SurroundFormat format)
    {
        SpeakerLayoutInfo info;
        info.format = format;
        info.numChannels = getChannelCountForFormat(format);

        switch (format)
        {
            case SurroundFormat::Stereo:
                info.channelNames = { "L", "R" };
                break;
            case SurroundFormat::Quad:
                info.channelNames = { "L", "R", "Ls", "Rs" };
                break;
            case SurroundFormat::Surround51:
                info.channelNames = { "L", "R", "C", "LFE", "Ls", "Rs" };
                break;
            case SurroundFormat::Surround71:
                info.channelNames = { "L", "R", "C", "LFE", "Ls", "Rs", "Lrs", "Rrs" };
                break;
            case SurroundFormat::Atmos714:
                info.channelNames = { "L", "R", "C", "LFE", "Ls", "Rs",
                                      "Lrs", "Rrs", "Ltf", "Rtf", "Ltr", "Rtr" };
                break;
        }

        return info;
    }
};

/**
 * @brief An input source connected to the surround bus with its own panner.
 */
struct SurroundBusInput {
    int id = -1;
    juce::String name;
    std::unique_ptr<SurroundPanner> panner;
};

/**
 * @brief Multi-channel surround bus supporting up to 7.1.4 (12 channels).
 *
 * Manages a collection of input sources, each with its own SurroundPanner,
 * and sums them into a multi-channel output buffer.
 */
class SurroundBus {
  public:
    explicit SurroundBus(SurroundFormat format = SurroundFormat::Surround51);
    ~SurroundBus() = default;

    // --- Configuration ---

    /** Set the bus surround format. Resizes internal buffers. */
    void setFormat(SurroundFormat format);

    /** Get the current surround format. */
    SurroundFormat getFormat() const { return format_; }

    /** Get the speaker layout description. */
    SpeakerLayoutInfo getSpeakerLayout() const;

    /** Get the number of output channels. */
    int getNumChannels() const { return getChannelCountForFormat(format_); }

    // --- Input management ---

    /**
     * @brief Add an input source with its own panner.
     * @param name Display name for the input.
     * @return The ID of the newly added input.
     */
    int addInput(const juce::String& name);

    /**
     * @brief Add an input source with a pre-configured panner.
     * @param name Display name for the input.
     * @param panner Pre-configured SurroundPanner (ownership transferred).
     * @return The ID of the newly added input.
     */
    int addInput(const juce::String& name, std::unique_ptr<SurroundPanner> panner);

    /** Remove an input by ID. */
    void removeInput(int inputId);

    /** Get the number of active inputs. */
    int getNumInputs() const;

    /** Get the panner for a specific input (nullptr if not found). */
    SurroundPanner* getPannerForInput(int inputId);

    // --- Processing ---

    /**
     * @brief Process a mono input buffer through its panner into the surround output.
     * @param inputId The input source ID.
     * @param monoInput The mono audio buffer to pan.
     * @param output The multi-channel output buffer to sum into.
     */
    void processInput(int inputId,
                      const juce::AudioBuffer<float>& monoInput,
                      juce::AudioBuffer<float>& output);

    /** Clear the output buffer (call before processing a new block). */
    void clearOutput(juce::AudioBuffer<float>& output);

  private:
    SurroundFormat format_;
    std::vector<SurroundBusInput> inputs_;
    mutable std::mutex mutex_;
    int nextInputId_ = 1;
};

}  // namespace magda
