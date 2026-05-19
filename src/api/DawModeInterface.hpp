#pragma once

#include <functional>
#include <string>
#include <vector>

namespace aidaw {

/**
 * @brief Interface for managing DAW modes and configurations
 *
 * This interface handles the dual-mode system:
 * 1. View Mode: Performance vs Arrangement (like Ableton Live)
 * 2. Audio Mode: Live vs Studio (buffer sizes and CPU optimization)
 */
class DawModeInterface {
  public:
    virtual ~DawModeInterface() = default;

    // === View Mode Management (Performance vs Arrangement) ===

    enum class ViewMode {
        Arrangement,  // Traditional timeline view
        Performance   // Session/clip launcher view
    };

    virtual void setViewMode(ViewMode mode) = 0;
    virtual ViewMode getViewMode() const = 0;
    virtual bool isArrangementMode() const = 0;
    virtual bool isPerformanceMode() const = 0;

    // === Audio Mode Management (Live vs Studio) ===

    enum class AudioMode {
        Live,   // Low latency, shorter buffers, real-time focus
        Studio  // Higher quality, larger buffers, CPU intensive
    };

    virtual void setAudioMode(AudioMode mode) = 0;
    virtual AudioMode getAudioMode() const = 0;
    virtual bool isLiveMode() const = 0;
    virtual bool isStudioMode() const = 0;

    // === Audio Configuration ===

    virtual int getBufferSize() const = 0;
    virtual int getSampleRate() const = 0;
    virtual double getLatencyMs() const = 0;
    virtual double getCpuUsage() const = 0;

    // === Performance Mode Specific ===

    /**
     * @brief Launch a clip in performance mode
     * @param clip_id The clip to launch
     * @param quantize_beats Quantization in beats (0 = immediate, 1 = next beat, etc.)
     */
    virtual void launchClip(const std::string& clip_id, double quantize_beats = 1.0) = 0;

    /**
     * @brief Stop a clip in performance mode
     * @param clip_id The clip to stop
     * @param quantize_beats Quantization in beats
     */
    virtual void stopClip(const std::string& clip_id, double quantize_beats = 0.0) = 0;

    virtual std::vector<std::string> getPerformanceClips() const = 0;
    virtual std::vector<std::string> getPlayingClips() const = 0;

    // === Mode Change Events ===

    using ViewModeChangedCallback = std::function<void(ViewMode new_mode)>;
    virtual void onViewModeChanged(ViewModeChangedCallback callback) = 0;

    using AudioModeChangedCallback = std::function<void(AudioMode new_mode)>;
    virtual void onAudioModeChanged(AudioModeChangedCallback callback) = 0;
};

}  // namespace aidaw
