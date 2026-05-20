#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include "VideoTrack.hpp"
#include <unordered_map>
#include <vector>

namespace magda
{

//==============================================================================
/**
    Extracts video frames for display using FFmpeg command-line tools.
    Maintains a frame cache for smooth scrubbing.
*/
class VideoRenderer
{
public:
    VideoRenderer();
    ~VideoRenderer();

    /// Probe a video file and return its metadata (uses ffprobe).
    VideoInfo probeVideo (const juce::File& videoFile) const;

    /// Extract a single frame at the given time position.
    juce::Image extractFrame (const juce::File& videoFile, double seconds) const;

    /// Generate a strip of thumbnail images across a time range.
    std::vector<juce::Image> getThumbnailStrip (const juce::File& videoFile,
                                                 double startTime,
                                                 double endTime,
                                                 int numFrames) const;

    /// Clear the frame cache.
    void clearCache();

    /// Set the maximum number of cached frames.
    void setMaxCacheSize (int maxFrames);

    /// Get the path to the FFmpeg executable (auto-detected or user-set).
    juce::File getFFmpegPath() const;

    /// Override the FFmpeg executable path.
    void setFFmpegPath (const juce::File& path);

private:
    juce::File ffmpegPath_;
    juce::File ffprobePath_;

    mutable std::unordered_map<juce::int64, juce::Image> frameCache_;
    int maxCacheSize_ = 256;

    /// Find FFmpeg/FFprobe on the system PATH.
    void detectFFmpeg();

    /// Run a command and capture stdout as raw bytes.
    juce::MemoryBlock runProcess (const juce::StringArray& args) const;

    /// Generate a cache key from file path and time.
    static juce::int64 makeCacheKey (const juce::File& file, double seconds);

    /// Decode raw PPM image data into a juce::Image.
    static juce::Image decodePPM (const juce::MemoryBlock& data);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoRenderer)
};

} // namespace magda
