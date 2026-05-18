#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace aidaw {

/**
 * @brief Base interface for all AI agents in AIDAW.
 *
 * Provides a lightweight framework for AI agents that can interact
 * with the DAW and communicate with each other via message callbacks.
 */
class AgentInterface {
public:
    virtual ~AgentInterface() = default;

    /** Get the unique identifier for this agent. */
    virtual std::string getId() const = 0;

    /** Get the human-readable name of this agent. */
    virtual std::string getName() const = 0;

    /** Get the type/category of this agent (e.g., "mixer", "composition", "effects"). */
    virtual std::string getType() const = 0;

    /** Get agent capabilities as key-value pairs. */
    virtual std::map<std::string, std::string> getCapabilities() const = 0;

    /** Start the agent (called when agent is registered). */
    virtual bool start() = 0;

    /** Stop the agent (called when agent is unregistered). */
    virtual void stop() = 0;

    /** Check if the agent is currently running. */
    virtual bool isRunning() const = 0;

    /**
     * Process a message/command from the DAW or other agents.
     * @return Response message (empty if no response needed)
     */
    virtual std::string processMessage(const std::string& message) = 0;

    /**
     * Set callback for sending messages to the DAW or other agents.
     */
    void setMessageCallback(
        std::function<void(const std::string& fromAgent, const std::string& message)> callback) {
        messageCallback_ = std::move(callback);
    }

protected:
    /** Send a message to the DAW or other agents. */
    void sendMessage(const std::string& message) {
        if (messageCallback_) {
            messageCallback_(getId(), message);
        }
    }

    std::function<void(const std::string&, const std::string&)> messageCallback_;
};

}  // namespace aidaw
