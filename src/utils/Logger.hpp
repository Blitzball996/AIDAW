#pragma once

#include <string>

namespace aidaw {

enum class LogLevel { Debug, Info, Warning, Error };

class Logger {
public:
    static Logger& getInstance();
    void log(LogLevel level, const std::string& message);
    void setMinLevel(LogLevel level) { minLevel = level; }

private:
    Logger() = default;
    LogLevel minLevel = LogLevel::Info;
};

}  // namespace aidaw
