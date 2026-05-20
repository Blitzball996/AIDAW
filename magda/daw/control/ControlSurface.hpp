#pragma once

#include <juce_core/juce_core.h>

#include <functional>
#include <vector>

namespace magda {

/**
 * @brief Abstract base class for all control surfaces.
 *
 * Defines the interface for hardware controllers (MIDI, OSC, HUI, MCU, etc.)
 * to communicate with the DAW. Subclasses implement the transport layer
 * and protocol-specific message parsing.
 */
class ControlSurface {
  public:
    virtual ~ControlSurface() = default;

    // --- Identification ---

    /** Get the display name of this control surface. */
    virtual juce::String getName() const = 0;

    /** Get the protocol type (e.g., "MIDI", "OSC", "HUI"). */
    virtual juce::String getProtocol() const = 0;

    /** Check if the surface is currently connected. */
    virtual bool isConnected() const = 0;

    // --- Lifecycle ---

    /** Open the connection to the hardware. */
    virtual bool open() = 0;

    /** Close the connection. */
    virtual void close() = 0;

    // --- Incoming events (from hardware to DAW) ---

    /**
     * @brief Called when a fader is moved on the surface.
     * @param channel Channel/strip number (0-based).
     * @param value Normalised fader value (0.0 to 1.0).
     */
    virtual void onFaderMove(int channel, float value) = 0;

    /**
     * @brief Called when a button is pressed on the surface.
     * @param id Button identifier.
     */
    virtual void onButtonPress(int id) = 0;

    /**
     * @brief Called when a rotary encoder is turned.
     * @param id Encoder identifier.
     * @param delta Relative change (-1 or +1 for detented, or continuous).
     */
    virtual void onEncoderTurn(int id, int delta) = 0;

    // --- Outgoing feedback (from DAW to hardware) ---

    /**
     * @brief Send fader position feedback to the surface (motorised faders).
     * @param channel Channel/strip number.
     * @param value Normalised position (0.0 to 1.0).
     */
    virtual void sendFeedback(int channel, float value) = 0;

    /**
     * @brief Send LED state to the surface.
     * @param id LED/button identifier.
     * @param state true = on, false = off.
     */
    virtual void sendLED(int id, bool state) = 0;

    // --- Callbacks ---

    using FaderCallback   = std::function<void(int channel, float value)>;
    using ButtonCallback  = std::function<void(int id)>;
    using EncoderCallback = std::function<void(int id, int delta)>;

    void setFaderCallback(FaderCallback cb)     { faderCallback_ = std::move(cb); }
    void setButtonCallback(ButtonCallback cb)   { buttonCallback_ = std::move(cb); }
    void setEncoderCallback(EncoderCallback cb) { encoderCallback_ = std::move(cb); }

  protected:
    /** Dispatch a fader event to the registered callback. */
    void dispatchFader(int channel, float value)
    {
        if (faderCallback_)
            faderCallback_(channel, value);
    }

    /** Dispatch a button event to the registered callback. */
    void dispatchButton(int id)
    {
        if (buttonCallback_)
            buttonCallback_(id);
    }

    /** Dispatch an encoder event to the registered callback. */
    void dispatchEncoder(int id, int delta)
    {
        if (encoderCallback_)
            encoderCallback_(id, delta);
    }

  private:
    FaderCallback faderCallback_;
    ButtonCallback buttonCallback_;
    EncoderCallback encoderCallback_;
};

}  // namespace magda
