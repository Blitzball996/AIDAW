#pragma once

#include <functional>
#include <string>

namespace aidaw {

using VoiceCallback = std::function<void(const std::string& text)>;

class VoiceInput {
public:
    void startListening(VoiceCallback onResult);
    void stopListening();
    bool isListening() const { return listening; }

private:
    bool listening = false;
};

}  // namespace aidaw
