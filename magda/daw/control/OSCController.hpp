#pragma once

#include "ControlSurface.hpp"

#include <juce_osc/juce_osc.h>

#include <map>
#include <memory>
#include <mutex>

namespace magda {

/**
 * @brief OSC-based control surface implementation.
 *
 * Listens for OSC messages on a configurable UDP port and maps
 * OSC addresses to DAW parameters. Supports bidirectional communication
 * for feedback to touch controllers (e.g., TouchOSC, Lemur).
 *
 * Default OSC address scheme:
 *   /track/N/volume  — fader (float 0-1)
 *   /track/N/pan     — pan (float -1 to +1)
 *   /track/N/mute    — mute toggle (int 0/1)
 *   /track/N/solo    — solo toggle (int 0/1)
 *   /transport/play  — play (trigger)
 *   /transport/stop  — stop (trigger)
 *   /transport/record — record (trigger)
 *   /master/volume   — master fader (float 0-1)
 */
class OSCController : public ControlSurface,
                      private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback> {
  public:
    /** Default OSC listen port. */
    static constexpr int kDefaultPort = 8000;

    explicit OSCController(int listenPort = kDefaultPort);
    ~OSCController() override;

    // --- ControlSurface interface ---

    juce::String getName() const override { return name_; }
    juce::String getProtocol() const override { return "OSC"; }
    bool isConnected() const override { return connected_; }

    bool open() override;
    void close() override;

    void onFaderMove(int channel, float value) override;
    void onButtonPress(int id) override;
    void onEncoderTurn(int id, int delta) override;

    void sendFeedback(int channel, float value) override;
    void sendLED(int id, bool state) override;

    // --- OSC-specific configuration ---

    /** Set the listen port (must call open() again to take effect). */
    void setListenPort(int port) { listenPort_ = port; }

    /** Get the current listen port. */
    int getListenPort() const { return listenPort_; }

    /** Set the remote host/port for feedback messages. */
    void setFeedbackTarget(const juce::String& host, int port);

    /** Set the controller display name. */
    void setName(const juce::String& name) { name_ = name; }

    /**
     * @brief Add a custom OSC address mapping.
     * @param address OSC address pattern (e.g., "/custom/fader1").
     * @param channel The DAW channel to map to.
     */
    void addMapping(const juce::String& address, int channel);

    /** Remove a custom mapping. */
    void removeMapping(const juce::String& address);

  private:
    // juce::OSCReceiver::Listener
    void oscMessageReceived(const juce::OSCMessage& message) override;
    void oscBundleReceived(const juce::OSCBundle& bundle) override;

    void handleTrackMessage(const juce::String& address, const juce::OSCMessage& message);
    void handleTransportMessage(const juce::String& address, const juce::OSCMessage& message);

    juce::String name_ = "OSC Controller";
    int listenPort_ = kDefaultPort;
    bool connected_ = false;

    juce::OSCReceiver receiver_;
    juce::OSCSender sender_;

    juce::String feedbackHost_ = "127.0.0.1";
    int feedbackPort_ = 9000;
    bool feedbackEnabled_ = false;

    // Custom address-to-channel mappings
    std::map<juce::String, int> customMappings_;
    mutable std::mutex mutex_;
};

}  // namespace magda
