#pragma once

#include <string>
#include <sstream>
#include <vector>

namespace aidaw {

class StringUtils {
public:
    static std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::istringstream stream(str);
        std::string token;
        while (std::getline(stream, token, delimiter)) {
            if (!token.empty()) tokens.push_back(token);
        }
        return tokens;
    }

    static std::string trim(const std::string& str) {
        auto start = str.find_first_not_of(" \t\n\r");
        auto end = str.find_last_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        return str.substr(start, end - start + 1);
    }
};

}  // namespace aidaw
