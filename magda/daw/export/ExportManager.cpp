#include "ExportManager.hpp"

namespace magda {

ExportManager::ExportManager() = default;
ExportManager::~ExportManager() = default;

bool ExportManager::exportToFile(const ExportSettings& settings,
                                 ProgressCallback callback) {
    if (exporting_.load())
        return false;

    exporting_ = true;
    cancelRequested_ = false;
    progress_ = 0.0f;

    juce::File outputFile(settings.outputPath);
    outputFile.getParentDirectory().createDirectory();

    auto writer = createWriter(settings, outputFile, 2);
    if (!writer) {
        exporting_ = false;
        return false;
    }

    bool success = renderToWriter(writer.get(), settings, callback);

    progress_ = success ? 1.0f : 0.0f;
    exporting_ = false;
    return success;
}

std::vector<juce::String> ExportManager::exportStems(
    const ExportSettings& settings, const std::vector<int>& /*trackIndices*/,
    ProgressCallback callback) {
    std::vector<juce::String> exportedFiles;

    if (exporting_.load())
        return exportedFiles;

    exporting_ = true;
    cancelRequested_ = false;
    progress_ = 0.0f;

    juce::File outputDir(settings.outputPath);
    outputDir.createDirectory();

    // TODO: Iterate tracks from the engine and render each individually.
    // For now, this is a structural placeholder that will be wired to
    // the TracktionEngine render pipeline.

    if (callback)
        callback(1.0f);

    progress_ = 1.0f;
    exporting_ = false;
    return exportedFiles;
}

void ExportManager::cancelExport() {
    cancelRequested_ = true;
}

juce::String ExportManager::getFileExtension(ExportFormat format) {
    switch (format) {
        case ExportFormat::WAV_16:
        case ExportFormat::WAV_24:
        case ExportFormat::WAV_32F:
            return ".wav";
        case ExportFormat::FLAC:
            return ".flac";
        case ExportFormat::MP3_128:
        case ExportFormat::MP3_192:
        case ExportFormat::MP3_320:
            return ".mp3";
        case ExportFormat::OGG:
            return ".ogg";
        case ExportFormat::AIFF:
            return ".aiff";
    }
    return ".wav";
}

int ExportManager::getBitDepth(ExportFormat format) {
    switch (format) {
        case ExportFormat::WAV_16:
            return 16;
        case ExportFormat::WAV_24:
        case ExportFormat::FLAC:
        case ExportFormat::AIFF:
            return 24;
        case ExportFormat::WAV_32F:
            return 32;
        case ExportFormat::MP3_128:
        case ExportFormat::MP3_192:
        case ExportFormat::MP3_320:
        case ExportFormat::OGG:
            return 16;  // Lossy formats use their own encoding
    }
    return 24;
}

std::unique_ptr<juce::AudioFormatWriter> ExportManager::createWriter(
    const ExportSettings& settings, const juce::File& outputFile,
    int numChannels) {
    std::unique_ptr<juce::AudioFormat> format;

    switch (settings.format) {
        case ExportFormat::WAV_16:
        case ExportFormat::WAV_24:
        case ExportFormat::WAV_32F:
            format = std::make_unique<juce::WavAudioFormat>();
            break;
        case ExportFormat::FLAC:
            format = std::make_unique<juce::FlacAudioFormat>();
            break;
        case ExportFormat::AIFF:
            format = std::make_unique<juce::AiffAudioFormat>();
            break;
        case ExportFormat::OGG:
            format = std::make_unique<juce::OggVorbisAudioFormat>();
            break;
        case ExportFormat::MP3_128:
        case ExportFormat::MP3_192:
        case ExportFormat::MP3_320:
            // MP3 encoding requires a third-party encoder (LAME).
            // Fall back to WAV if unavailable.
            format = std::make_unique<juce::WavAudioFormat>();
            break;
    }

    if (!format)
        return nullptr;

    auto stream = outputFile.createOutputStream();
    if (!stream)
        return nullptr;

    int bitDepth = getBitDepth(settings.format);

    auto* writer = format->createWriterFor(
        stream.release(), static_cast<double>(settings.sampleRate),
        static_cast<unsigned int>(numChannels), bitDepth, {}, 0);

    return std::unique_ptr<juce::AudioFormatWriter>(writer);
}

bool ExportManager::renderToWriter(juce::AudioFormatWriter* /*writer*/,
                                   const ExportSettings& /*settings*/,
                                   ProgressCallback callback) {
    // TODO: Wire to TracktionEngine's render pipeline.
    // This will iterate through the audio graph, rendering blocks
    // and writing them to the writer.

    if (cancelRequested_.load())
        return false;

    if (callback)
        callback(1.0f);

    return true;
}

}  // namespace magda
