#pragma once

#include <juce_core/juce_core.h>

namespace magda {

enum class ExportFormat {
    WAV_16,
    WAV_24,
    WAV_32F,
    FLAC,
    MP3_128,
    MP3_192,
    MP3_320,
    OGG,
    AIFF
};

struct ExportSettings {
    ExportFormat format = ExportFormat::WAV_24;
    int sampleRate = 44100;
    bool normalize = false;
    bool trimSilence = false;
    bool dithering = false;
    double startTime = 0.0;
    double endTime = -1.0;  // -1 = end of project
    juce::String outputPath;
};

}  // namespace magda
