#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace aidaw {

class TransportPanel : public juce::Component {
public:
    TransportPanel();
    void paint(juce::Graphics& g) override;
    void resized() override;

    void setPlaying(bool playing);
    void setRecording(bool recording);
    void setLooping(bool looping);
    void setTempo(double bpm);
    void setPosition(double beats);
    void setTimeSignature(int numerator, int denominator);

    std::function<void()> onPlay, onStop, onRecord, onLoop;
    std::function<void(double)> onTempoChange;

private:
    juce::TextButton playBtn{">"}, stopBtn{"[]"}, recBtn{"O"}, loopBtn{"L"};
    juce::TextButton rewindBtn{"|<"}, ffwdBtn{">|"};
    juce::Label tempoLabel, posLabel, timeSigLabel;
    bool isPlaying = false, isRecording = false, isLooping = false;
};

}  // namespace aidaw
