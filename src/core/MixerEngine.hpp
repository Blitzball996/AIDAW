#pragma once

namespace aidaw {

class MixerEngine {
public:
    void setMasterVolume(float volume) { masterVolume = volume; }
    float getMasterVolume() const { return masterVolume; }

private:
    float masterVolume = 1.0f;
};

}  // namespace aidaw
