#include "VoiceInput.hpp"

namespace aidaw {

void VoiceInput::startListening(VoiceCallback /*onResult*/) {
    // TODO: integrate speech-to-text (Whisper or platform API)
    listening = true;
}

void VoiceInput::stopListening() {
    listening = false;
}

}  // namespace aidaw
