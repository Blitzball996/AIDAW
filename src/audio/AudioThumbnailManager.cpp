#include "AudioThumbnailManager.hpp"
#include "WaveformPeakCache.hpp"

namespace aidaw {

AudioThumbnailManager::AudioThumbnailManager() {
    formatManager_.registerBasicFormats();
    thumbnailCache_ = std::make_unique<juce::AudioThumbnailCache>(100);
}

AudioThumbnailManager& AudioThumbnailManager::getInstance() {
    static AudioThumbnailManager instance;
    return instance;
}

juce::AudioThumbnail* AudioThumbnailManager::getThumbnail(const juce::String& audioFilePath) {
    auto it = thumbnails_.find(audioFilePath);
    if (it != thumbnails_.end())
        return it->second.get();

    return createThumbnail(audioFilePath);
}

juce::AudioThumbnail* AudioThumbnailManager::createThumbnail(const juce::String& audioFilePath) {
    juce::File audioFile(audioFilePath);
    if (!audioFile.existsAsFile())
        return nullptr;

    auto thumbnail = std::make_unique<juce::AudioThumbnail>(512, formatManager_, *thumbnailCache_);

    auto* reader = formatManager_.createReaderFor(audioFile);
    if (reader == nullptr)
        return nullptr;

    thumbnail->setReader(reader, audioFile.hashCode64());

    // Kick off peak cache load in background
    requestPeakCacheLoad(audioFilePath);

    auto* ptr = thumbnail.get();
    thumbnails_[audioFilePath] = std::move(thumbnail);
    return ptr;
}

juce::ThreadPool& AudioThumbnailManager::getOrCreateBackgroundPool() {
    if (!backgroundThreadPool_)
        backgroundThreadPool_ = std::make_unique<juce::ThreadPool>(1);
    return *backgroundThreadPool_;
}

void AudioThumbnailManager::requestPeakCacheLoad(const juce::String& audioFilePath) {
    if (peakCaches_.count(audioFilePath) > 0)
        return;
    if (pendingPeakComputes_.count(audioFilePath) > 0)
        return;

    pendingPeakComputes_.insert(audioFilePath);

    juce::File sourceFile(audioFilePath);
    auto& pool = getOrCreateBackgroundPool();

    pool.addJob([this, audioFilePath, sourceFile]() {
        // Try loading from disk first
        auto cache = WaveformPeakCache::loadFromDisk(sourceFile);
        if (!cache) {
            // Compute from scratch
            juce::AudioFormatManager fm;
            fm.registerBasicFormats();
            std::unique_ptr<juce::AudioFormatReader> reader(fm.createReaderFor(sourceFile));
            if (reader)
                cache = WaveformPeakCache::computeAndWrite(sourceFile, *reader);
        }

        if (cache) {
            auto sharedCache = std::shared_ptr<WaveformPeakCache>(cache.release());
            juce::MessageManager::callAsync([this, audioFilePath, sharedCache]() {
                peakCaches_[audioFilePath] = sharedCache;
                pendingPeakComputes_.erase(audioFilePath);
            });
        } else {
            juce::MessageManager::callAsync([this, audioFilePath]() {
                pendingPeakComputes_.erase(audioFilePath);
            });
        }
    });
}

void AudioThumbnailManager::drawWaveform(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                                          const juce::String& audioFilePath, double startTime,
                                          double endTime, const juce::Colour& colour,
                                          float verticalZoom, bool useHighRes, bool thick) {
    if (useHighRes) {
        auto* reader = getOrCreateReader(audioFilePath);
        if (reader) {
            const WaveformPeakCache* peakCache = nullptr;
            auto cacheIt = peakCaches_.find(audioFilePath);
            if (cacheIt != peakCaches_.end())
                peakCache = cacheIt->second.get();
            drawWaveformFromSamples(g, bounds, reader, peakCache, startTime, endTime, colour,
                                    verticalZoom, thick);
            return;
        }
    }

    auto* thumbnail = getThumbnail(audioFilePath);
    if (!thumbnail)
        return;

    g.setColour(colour);
    thumbnail->drawChannels(g, bounds, startTime, endTime, verticalZoom);
}

juce::AudioFormatReader* AudioThumbnailManager::getOrCreateReader(
    const juce::String& audioFilePath) {
    auto indexIt = readerIndex_.find(audioFilePath);
    if (indexIt != readerIndex_.end()) {
        // Move to front (most recently used)
        readerLru_.splice(readerLru_.begin(), readerLru_, indexIt->second);
        return indexIt->second->reader.get();
    }

    juce::File file(audioFilePath);
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager_.createReaderFor(file));
    if (!reader)
        return nullptr;

    // Evict LRU if at capacity
    if (readerLru_.size() >= MAX_CACHED_READERS) {
        auto& back = readerLru_.back();
        readerIndex_.erase(back.path);
        readerLru_.pop_back();
    }

    readerLru_.push_front({audioFilePath, std::move(reader)});
    readerIndex_[audioFilePath] = readerLru_.begin();
    return readerLru_.front().reader.get();
}

void AudioThumbnailManager::drawWaveformFromSamples(
    juce::Graphics& g, const juce::Rectangle<int>& bounds, juce::AudioFormatReader* reader,
    const WaveformPeakCache* peakCache, double startTime, double endTime,
    const juce::Colour& colour, float verticalZoom, bool thick) {
    if (!reader || bounds.getWidth() <= 0 || bounds.getHeight() <= 0)
        return;

    const double sampleRate = reader->sampleRate;
    const juce::int64 startSample = static_cast<juce::int64>(startTime * sampleRate);
    const juce::int64 endSample = static_cast<juce::int64>(endTime * sampleRate);
    const int width = bounds.getWidth();
    const double samplesPerPixel = static_cast<double>(endSample - startSample) / width;

    g.setColour(colour);
    const float midY = bounds.getCentreY();
    const float halfHeight = bounds.getHeight() * 0.5f * verticalZoom;

    for (int x = 0; x < width; ++x) {
        const juce::int64 s0 = startSample + static_cast<juce::int64>(x * samplesPerPixel);
        const juce::int64 s1 = startSample + static_cast<juce::int64>((x + 1) * samplesPerPixel);

        float minVal = 0.0f, maxVal = 0.0f;

        if (peakCache && samplesPerPixel >= WaveformPeakCache::SAMPLES_PER_PEAK) {
            auto mm = peakCache->getMinMaxForRange(0, s0, s1);
            minVal = mm.min;
            maxVal = mm.max;
        } else {
            // Read raw samples for this column
            const int numSamples = static_cast<int>(std::max<juce::int64>(1, s1 - s0));
            juce::AudioBuffer<float> buffer(1, numSamples);
            reader->read(&buffer, 0, numSamples, s0, true, false);
            auto range = buffer.findMinMax(0, 0, numSamples);
            minVal = range.getStart();
            maxVal = range.getEnd();
        }

        const float top = midY - maxVal * halfHeight;
        const float bottom = midY - minVal * halfHeight;
        const float lineWidth = thick ? 2.0f : 1.0f;
        g.drawVerticalLine(bounds.getX() + x, top, std::max(bottom, top + lineWidth));
    }
}

double AudioThumbnailManager::detectBPM(const juce::String& filePath) {
#ifdef AIDAW_HAS_TRACKTION
    // Use Tracktion's TempoDetect if available
    juce::File file(filePath);
    if (!file.existsAsFile())
        return 0.0;

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager_.createReaderFor(file));
    if (!reader)
        return 0.0;

    tracktion::TempoDetect detector(static_cast<int>(reader->numChannels), reader->sampleRate);

    constexpr int blockSize = 8192;
    juce::AudioBuffer<float> buffer(static_cast<int>(reader->numChannels), blockSize);
    juce::int64 pos = 0;

    while (pos < reader->lengthInSamples) {
        const int toRead = static_cast<int>(
            std::min<juce::int64>(blockSize, reader->lengthInSamples - pos));
        buffer.clear();
        reader->read(&buffer, 0, toRead, pos, true, true);
        detector.processSection(buffer, toRead);
        pos += toRead;
    }

    double bpm = detector.finishAndDetect();
    if (bpm > 40.0 && bpm < 300.0) {
        bpmCache_[filePath] = bpm;
        return bpm;
    }
#else
    juce::ignoreUnused(filePath);
#endif
    return 0.0;
}

double AudioThumbnailManager::getCachedBPM(const juce::String& filePath) const {
    auto it = bpmCache_.find(filePath);
    return it != bpmCache_.end() ? it->second : 0.0;
}

void AudioThumbnailManager::cacheBPM(const juce::String& filePath, double bpm) {
    bpmCache_[filePath] = bpm;
}

void AudioThumbnailManager::requestBPMDetection(const juce::String& filePath,
                                                  std::function<void(double)> onComplete) {
    // Check cache first
    auto cached = getCachedBPM(filePath);
    if (cached > 0.0) {
        if (onComplete)
            onComplete(cached);
        return;
    }

    // Dedupe concurrent requests
    auto& callbacks = pendingBpmCallbacks_[filePath];
    callbacks.push_back(std::move(onComplete));
    if (callbacks.size() > 1)
        return;  // Already in flight

    auto& pool = getOrCreateBackgroundPool();
    pool.addJob([this, filePath]() {
        double bpm = detectBPM(filePath);
        juce::MessageManager::callAsync([this, filePath, bpm]() {
            auto it = pendingBpmCallbacks_.find(filePath);
            if (it != pendingBpmCallbacks_.end()) {
                for (auto& cb : it->second) {
                    if (cb)
                        cb(bpm);
                }
                pendingBpmCallbacks_.erase(it);
            }
        });
    });
}

const juce::Array<double>* AudioThumbnailManager::getCachedTransients(
    const juce::String& filePath) const {
    auto it = transientCache_.find(filePath);
    return it != transientCache_.end() ? &it->second : nullptr;
}

void AudioThumbnailManager::cacheTransients(const juce::String& filePath,
                                             const juce::Array<double>& times) {
    transientCache_[filePath] = times;
}

void AudioThumbnailManager::clearCachedTransients(const juce::String& filePath) {
    transientCache_.erase(filePath);
}

void AudioThumbnailManager::clearCache() {
    thumbnails_.clear();
    readerLru_.clear();
    readerIndex_.clear();
    peakCaches_.clear();
    pendingPeakComputes_.clear();
}

void AudioThumbnailManager::shutdown() {
    if (backgroundThreadPool_) {
        backgroundThreadPool_->removeAllJobs(true, 5000);
        backgroundThreadPool_.reset();
    }
    clearCache();
    bpmCache_.clear();
    transientCache_.clear();
    pendingBpmCallbacks_.clear();
}

}  // namespace aidaw
