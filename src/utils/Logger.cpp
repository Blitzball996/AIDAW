#include "Logger.hpp"
#include <juce_core/juce_core.h>

namespace aidaw {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < minLevel) return;

    const char* prefix = "";
    switch (level) {
        case LogLevel::Debug:   prefix = "[DEBUG] "; break;
        case LogLevel::Info:    prefix = "[INFO]  "; break;
        case LogLevel::Warning: prefix = "[WARN]  "; break;
        case LogLevel::Error:   prefix = "[ERROR] "; break;
    }

    juce::Logger::writeToLog(juce::String(prefix) + juce::String(message));
}

}  // namespace aidaw
