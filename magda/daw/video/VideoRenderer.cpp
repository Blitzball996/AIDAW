#include "VideoRenderer.hpp"

namespace magda
{

VideoRenderer::VideoRenderer()
{
    detectFFmpeg();
}

VideoRenderer::~VideoRenderer() = default;

//==============================================================================
void VideoRenderer::detectFFmpeg()
{
    // Try common locations and PATH
#if JUCE_WINDOWS
    const juce::StringArray candidates = {
        "ffmpeg.exe",
        "C:/ffmpeg/bin/ffmpeg.exe",
        "C:/Program Files/ffmpeg/bin/ffmpeg.exe"
    };
    const juce::StringArray probeCandidates = {
        "ffprobe.exe",
        "C:/ffmpeg/bin/ffprobe.exe",
        "C:/Program Files/ffmpeg/bin/ffprobe.exe"
    };
#else
    const juce::StringArray candidates = {
        "/usr/bin/ffmpeg",
        "/usr/local/bin/ffmpeg",
        "/opt/homebrew/bin/ffmpeg",
        "ffmpeg"
    };
    const juce::StringArray probeCandidates = {
        "/usr/bin/ffprobe",
        "/usr/local/bin/ffprobe",
        "/opt/homebrew/bin/ffprobe",
        "ffprobe"
    };
#endif

    for (auto& c : candidates)
    {
        juce::File f (c);
        if (f.existsAsFile())
        {
            ffmpegPath_ = f;
            break;
        }
    }

    for (auto& c : probeCandidates)
    {
        juce::File f (c);
        if (f.existsAsFile())
        {
            ffprobePath_ = f;
            break;
        }
    }
}

//==============================================================================
juce::File VideoRenderer::getFFmpegPath() const
{
    return ffmpegPath_;
}

void VideoRenderer::setFFmpegPath (const juce::File& path)
{
    ffmpegPath_ = path;

    // Derive ffprobe from the same directory
    auto dir = path.getParentDirectory();
#if JUCE_WINDOWS
    ffprobePath_ = dir.getChildFile ("ffprobe.exe");
#else
    ffprobePath_ = dir.getChildFile ("ffprobe");
#endif
}

//==============================================================================
VideoInfo VideoRenderer::probeVideo (const juce::File& videoFile) const
{
    VideoInfo info;
    info.path = videoFile.getFullPathName();

    if (ffprobePath_.getFullPathName().isEmpty())
        return info;

    // ffprobe -v quiet -print_format json -show_format -show_streams <file>
    juce::StringArray args;
    args.add (ffprobePath_.getFullPathName());
    args.add ("-v");
    args.add ("quiet");
    args.add ("-print_format");
    args.add ("json");
    args.add ("-show_format");
    args.add ("-show_streams");
    args.add (videoFile.getFullPathName());

    auto output = runProcess (args);

    if (output.getSize() == 0)
        return info;

    auto jsonStr = juce::String::fromUTF8 (
        static_cast<const char*> (output.getData()), (int) output.getSize());

    auto parsed = juce::JSON::parse (jsonStr);

    if (auto* format = parsed.getProperty ("format", {}).getDynamicObject())
    {
        info.duration = format->getProperty ("duration").toString().getDoubleValue();
    }

    if (auto* streams = parsed.getProperty ("streams", {}).getArray())
    {
        for (auto& stream : *streams)
        {
            if (stream.getProperty ("codec_type", "") == "video")
            {
                info.width  = (int) stream.getProperty ("width", 0);
                info.height = (int) stream.getProperty ("height", 0);
                info.codec  = stream.getProperty ("codec_name", "").toString();

                // Parse fps from r_frame_rate (e.g. "30/1")
                auto fpsStr = stream.getProperty ("r_frame_rate", "").toString();
                if (fpsStr.contains ("/"))
                {
                    auto num = fpsStr.upToFirstOccurrenceOf ("/", false, false).getDoubleValue();
                    auto den = fpsStr.fromFirstOccurrenceOf ("/", false, false).getDoubleValue();
                    if (den > 0.0)
                        info.fps = num / den;
                }
                else
                {
                    info.fps = fpsStr.getDoubleValue();
                }
                break;
            }
        }
    }

    return info;
}

//==============================================================================
juce::Image VideoRenderer::extractFrame (const juce::File& videoFile, double seconds) const
{
    auto key = makeCacheKey (videoFile, seconds);

    // Check cache first
    auto it = frameCache_.find (key);
    if (it != frameCache_.end())
        return it->second;

    if (ffmpegPath_.getFullPathName().isEmpty())
        return {};

    // ffmpeg -ss <time> -i <file> -frames:v 1 -f image2pipe -vcodec ppm -
    juce::StringArray args;
    args.add (ffmpegPath_.getFullPathName());
    args.add ("-ss");
    args.add (juce::String (seconds, 3));
    args.add ("-i");
    args.add (videoFile.getFullPathName());
    args.add ("-frames:v");
    args.add ("1");
    args.add ("-f");
    args.add ("image2pipe");
    args.add ("-vcodec");
    args.add ("ppm");
    args.add ("-");

    auto output = runProcess (args);

    if (output.getSize() == 0)
        return {};

    auto image = decodePPM (output);

    // Store in cache (evict oldest if full)
    if ((int) frameCache_.size() >= maxCacheSize_)
        frameCache_.erase (frameCache_.begin());

    frameCache_[key] = image;
    return image;
}

//==============================================================================
std::vector<juce::Image> VideoRenderer::getThumbnailStrip (
    const juce::File& videoFile,
    double startTime,
    double endTime,
    int numFrames) const
{
    std::vector<juce::Image> strip;
    strip.reserve ((size_t) numFrames);

    if (numFrames <= 0 || endTime <= startTime)
        return strip;

    double step = (endTime - startTime) / (double) numFrames;

    for (int i = 0; i < numFrames; ++i)
    {
        double t = startTime + step * (i + 0.5);
        strip.push_back (extractFrame (videoFile, t));
    }

    return strip;
}

//==============================================================================
void VideoRenderer::clearCache()
{
    frameCache_.clear();
}

void VideoRenderer::setMaxCacheSize (int maxFrames)
{
    maxCacheSize_ = juce::jmax (1, maxFrames);
}

//==============================================================================
juce::MemoryBlock VideoRenderer::runProcess (const juce::StringArray& args) const
{
    juce::MemoryBlock result;
    juce::ChildProcess process;

    if (process.start (args, juce::ChildProcess::wantStdOut))
    {
        const int bufSize = 65536;
        juce::HeapBlock<char> buffer (bufSize);

        while (process.isRunning())
        {
            int bytesRead = process.readProcessOutput (buffer.getData(), bufSize);
            if (bytesRead > 0)
                result.append (buffer.getData(), (size_t) bytesRead);
        }

        // Read any remaining data
        int bytesRead = process.readProcessOutput (buffer.getData(), bufSize);
        while (bytesRead > 0)
        {
            result.append (buffer.getData(), (size_t) bytesRead);
            bytesRead = process.readProcessOutput (buffer.getData(), bufSize);
        }
    }

    return result;
}

//==============================================================================
juce::int64 VideoRenderer::makeCacheKey (const juce::File& file, double seconds)
{
    // Combine file hash with quantised time (10ms resolution)
    auto fileHash = file.getFullPathName().hashCode64();
    auto timeKey  = (juce::int64) (seconds * 100.0);
    return fileHash ^ (timeKey * 2654435761LL);
}

//==============================================================================
juce::Image VideoRenderer::decodePPM (const juce::MemoryBlock& data)
{
    // PPM P6 format: "P6\n<width> <height>\n<maxval>\n<binary RGB data>"
    auto* ptr = static_cast<const char*> (data.getData());
    auto  end = ptr + data.getSize();

    if (data.getSize() < 10 || ptr[0] != 'P' || ptr[1] != '6')
        return {};

    ptr += 3; // Skip "P6\n"

    // Skip comments
    while (ptr < end && *ptr == '#')
    {
        while (ptr < end && *ptr != '\n') ++ptr;
        if (ptr < end) ++ptr;
    }

    // Read width and height
    int width = 0, height = 0;
    while (ptr < end && *ptr >= '0' && *ptr <= '9')
        width = width * 10 + (*ptr++ - '0');
    while (ptr < end && (*ptr == ' ' || *ptr == '\n')) ++ptr;
    while (ptr < end && *ptr >= '0' && *ptr <= '9')
        height = height * 10 + (*ptr++ - '0');
    while (ptr < end && *ptr != '\n') ++ptr;
    if (ptr < end) ++ptr; // skip newline

    // Skip maxval line
    while (ptr < end && *ptr != '\n') ++ptr;
    if (ptr < end) ++ptr;

    if (width <= 0 || height <= 0)
        return {};

    auto bytesNeeded = (size_t) width * (size_t) height * 3;
    if ((size_t) (end - ptr) < bytesNeeded)
        return {};

    juce::Image image (juce::Image::RGB, width, height, false);
    juce::Image::BitmapData bmp (image, juce::Image::BitmapData::writeOnly);

    auto* src = reinterpret_cast<const juce::uint8*> (ptr);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            auto r = *src++;
            auto g = *src++;
            auto b = *src++;
            bmp.setPixelColour (x, y, juce::Colour (r, g, b));
        }
    }

    return image;
}

} // namespace magda
