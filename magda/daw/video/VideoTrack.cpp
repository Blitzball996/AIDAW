#include "VideoTrack.hpp"
#include "VideoRenderer.hpp"

namespace magda
{

VideoTrack::VideoTrack() = default;
VideoTrack::~VideoTrack() = default;

bool VideoTrack::loadVideo (const juce::File& filePath)
{
    if (! filePath.existsAsFile())
        return false;

    // Use VideoRenderer to probe the file metadata via FFmpeg.
    VideoRenderer renderer;
    auto info = renderer.probeVideo (filePath);

    if (info.duration <= 0.0)
        return false;

    videoInfo_ = std::move (info);
    loaded_    = true;
    playbackPosition_ = 0.0;
    return true;
}

void VideoTrack::unloadVideo()
{
    videoInfo_ = {};
    loaded_    = false;
    playbackPosition_ = 0.0;
}

bool VideoTrack::isLoaded() const noexcept
{
    return loaded_;
}

const VideoInfo& VideoTrack::getVideoInfo() const noexcept
{
    return videoInfo_;
}

juce::Image VideoTrack::getFrameAtTime (double seconds) const
{
    if (! loaded_)
        return {};

    VideoRenderer renderer;
    return renderer.extractFrame (juce::File (videoInfo_.path), seconds);
}

void VideoTrack::setPlaybackPosition (double seconds)
{
    playbackPosition_ = seconds;
}

double VideoTrack::getPlaybackPosition() const noexcept
{
    return playbackPosition_;
}

void VideoTrack::setTimelineOffset (double seconds)
{
    timelineOffset_ = seconds;
}

double VideoTrack::getTimelineOffset() const noexcept
{
    return timelineOffset_;
}

} // namespace magda
