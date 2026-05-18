#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace aidaw {

class WaveformPeakCache;

/**
 * @brief Manages audio waveform thumbnails for visualization
 *
 * Provides caching and rendering of audio waveforms using JUCE's AudioThumbnail.
 * Thumbnails are cached by file path for efficient reuse across clips.
 */
class AudioThumbnailManager {
  public:
    static AudioThumbnailManager& getInstance();

    juce::AudioThumbnail* getThumbnail(const juce::String& audioFilePath);

    void drawWaveform(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                      const juce::String& audioFilePath, double startTime, double endTime,
                      const juce::Colour& colour, float verticalZoom = 1.0f,
                      bool useHighRes = false, bool thick = false);

    double detectBPM(const juce::String& filePath);
    double getCachedBPM(const juce::String& filePath) const;
    void cacheBPM(const juce::String& filePath, double bpm);
    void requestBPMDetection(const juce::String& filePath, std::function<void(double)> onComplete);

    const juce::Array<double>* getCachedTransients(const juce::String& filePath) const;
    void cacheTransients(const juce::String& filePath, const juce::Array<double>& times);
    void clearCachedTransients(const juce::String& filePath);

    void clearCache();
    void shutdown();

  private:
    AudioThumbnailManager();
    ~AudioThumbnailManager() = default;

    juce::AudioFormatManager formatManager_;
    std::unique_ptr<juce::AudioThumbnailCache> thumbnailCache_;
    std::map<juce::String, std::unique_ptr<juce::AudioThumbnail>> thumbnails_;

    juce::AudioThumbnail* createThumbnail(const juce::String& audioFilePath);

    std::map<juce::String, double> bpmCache_;

    std::unique_ptr<juce::ThreadPool> backgroundThreadPool_;
    juce::ThreadPool& getOrCreateBackgroundPool();

    std::map<juce::String, std::vector<std::function<void(double)>>> pendingBpmCallbacks_;
    std::map<juce::String, juce::Array<double>> transientCache_;

    static constexpr size_t MAX_CACHED_READERS = 16;

    struct ReaderEntry {
        juce::String path;
        std::unique_ptr<juce::AudioFormatReader> reader;
    };

    std::list<ReaderEntry> readerLru_;
    std::map<juce::String, std::list<ReaderEntry>::iterator> readerIndex_;

    juce::AudioFormatReader* getOrCreateReader(const juce::String& audioFilePath);

    std::map<juce::String, std::shared_ptr<WaveformPeakCache>> peakCaches_;
    std::set<juce::String> pendingPeakComputes_;

    void requestPeakCacheLoad(const juce::String& audioFilePath);

    void drawWaveformFromSamples(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                                 juce::AudioFormatReader* reader,
                                 const WaveformPeakCache* peakCache, double startTime,
                                 double endTime, const juce::Colour& colour, float verticalZoom,
                                 bool thick = false);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioThumbnailManager)
};

}  // namespace aidaw
