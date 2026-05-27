#pragma once

#include <juce_core/juce_core.h>
#include <atomic>
#include "agent_interface.hpp"

/**
 * @brief AI agent for generating Strudel/Tidal patterns
 *
 * Can generate live coding patterns based on user requests like:
 * "make a drum pattern", "bass line in C minor", "ambient pad"
 */
class StrudelAgent : public AgentInterface {
  public:
    StrudelAgent();
    ~StrudelAgent() override;

    std::string getId() const override { return "strudel-agent"; }
    std::string getName() const override { return "Tidal Pattern Agent"; }
    std::string getType() const override { return "strudel"; }
    std::map<std::string, std::string> getCapabilities() const override;

    bool start() override;
    void stop() override;
    bool isRunning() const override { return running_.load(); }

    std::string processMessage(const std::string& message) override;
    void setMessageCallback(
        std::function<void(const std::string&, const std::string&)> callback) override;

  private:
    std::atomic<bool> running_{false};
    std::function<void(const std::string&, const std::string&)> messageCallback_;

    std::string generatePattern(const std::string& request);
    std::string getSystemPrompt() const;
};
