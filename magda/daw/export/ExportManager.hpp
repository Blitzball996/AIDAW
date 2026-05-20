#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

#include <atomic>
#include <functional>
#include <vector>

#include "ExportFormat.hpp"

namespace magda {

/**
 * @brief Manages audio export operations (mixdown, stems)
 *
 * Handles rendering the project or individual tracks to audio files
 * with progress reporting and cancellation support.
 */
class ExportManager {
  public:
    using ProgressCallback = std::function<void(float progress)>;

    ExportManager();
    ~ExportManager();

    /**
     * @brief Export the full mix to a single file
     * @param settings Export configuration
     * @param callback Optional progress callback (0.0 to 1.0)
     * @return true if export completed successfully
     */
    bool exportToFile(const ExportSettings& settings,
                      ProgressCallback callback = nullptr);

    /**
     * @brief Export each track as a separate stem file
     * @param settings Base export settings (outputPath used as directory)
     * @param trackIndices Which tracks to export (empty = all)
     * @param callback Optional progress callback
     * @return List of exported file paths
     */
    std::vector<juce::String> exportStems(const ExportSettings& settings,
                                          const std::vector<int>& trackIndices = {},
                                          ProgressCallback callback = nullptr);

    /**
     * @brief Get current export progress (0.0 to 1.0)
     */
    float getProgress() const { return progress_.load(); }

    /**
     * @brief Cancel an in-progress export
     */
    void cancelExport();

    /**
     * @brief Check if an export is currently running
     */
    bool isExporting() const { return exporting_.load(); }

    /**
     * @brief Get the file extension for a given format
     */
    static juce::String getFileExtension(ExportFormat format);

    /**
     * @brief Get bit depth for a given format
     */
    static int getBitDepth(ExportFormat format);

  private:
    std::unique_ptr<juce::AudioFormatWriter> createWriter(
        const ExportSettings& settings, const juce::File& outputFile,
        int numChannels);

    bool renderToWriter(juce::AudioFormatWriter* writer,
                        const ExportSettings& settings,
                        ProgressCallback callback);

    std::atomic<float> progress_{0.0f};
    std::atomic<bool> exporting_{false};
    std::atomic<bool> cancelRequested_{false};
};

}  // namespace magda
