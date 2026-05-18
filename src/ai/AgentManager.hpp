#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "AgentInterface.hpp"

namespace aidaw {

/**
 * @brief Manages all AI agents in AIDAW.
 *
 * The AgentManager coordinates communication between agents and the DAW,
 * handles agent lifecycle, and provides a simple message routing system.
 */
class AgentManager {
public:
    AgentManager();
    ~AgentManager();

    /** Register a new agent. Returns true if successful. */
    bool registerAgent(std::shared_ptr<AgentInterface> agent);

    /** Unregister an agent by ID. Returns true if successful. */
    bool unregisterAgent(const std::string& agentId);

    /** Get an agent by ID, or nullptr if not found. */
    std::shared_ptr<AgentInterface> getAgent(const std::string& agentId);

    /** Get all registered agents. */
    std::vector<std::shared_ptr<AgentInterface>> getAllAgents() const;

    /** Send a message to a specific agent. Returns the agent's response. */
    std::string sendToAgent(const std::string& agentId, const std::string& message);

    /** Broadcast a message to all running agents. */
    void broadcastMessage(const std::string& message);

    /** Get the number of registered agents. */
    size_t getAgentCount() const;

    /** Start all registered agents. */
    void startAllAgents();

    /** Stop all running agents. */
    void stopAllAgents();

private:
    void handleAgentMessage(const std::string& fromAgent, const std::string& message);

    mutable std::mutex agentsMutex_;
    std::map<std::string, std::shared_ptr<AgentInterface>> agents_;
};

}  // namespace aidaw
