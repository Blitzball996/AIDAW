#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

namespace magda
{

/// Metadata about a loaded video file.
struct VideoInfo
{
    juce::String path;
    double       duration = 0.0;   ///< Duration in seconds
    double       fps      = 0.0;   ///< Frames per second
    int          width    = 0;
    int          height   = 0;
    juce::String codec;
};

//==============================================================================
/**
    Manages a video file on the timeline, synchronised with audio transport.
    Frame extraction is delegated to VideoRenderer.
*/
class VideoTrack
{
public:
    VideoTrack();
    ~VideoTrack();

    /// Load a video file and extract its metadata.
    bool loadVideo (const juce::File& filePath);

    /// Unload the current video.
    void unloadVideo();

    /// Returns true if a video is currently loaded.
    bool isLoaded() const noexcept;

    /// Get metadata for the loaded video.
    const VideoInfo& getVideoInfo() const noexcept;

    /// Retrieve a thumbnail image for the given time position (seconds).
    juce::Image getFrameAtTime (double seconds) const;

    /// Sync the internal playback position with the audio transport.
    void setPlaybackPosition (double seconds);

    /// Get the current playback position.
    double getPlaybackPosition() const noexcept;

    /// Set the timeline offset (where the video starts on the timeline).
    void setTimelineOffset (double seconds);
    double getTimelineOffset() const noexcept;

private:
    VideoInfo videoInfo_;
    double    playbackPosition_ = 0.0;
    double    timelineOffset_   = 0.0;
    bool      loaded_           = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoTrack)
};

} // namespace magda
