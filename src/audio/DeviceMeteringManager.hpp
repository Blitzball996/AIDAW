#pragma once

#include <juce_core/juce_core.h>

#include <atomic>
#include <map>
#include <memory>

#ifdef AIDAW_HAS_TRACKTION
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion;
#endif

namespace aidaw {

using DeviceId = int;
using RackId = int;
constexpr DeviceId INVALID_DEVICE_ID = -1;

/**
 * @brief Manages per-device LevelMeasurer instances for peak metering
 *
 * Thread Safety:
 * - getOrCreateMeasurer(): called from message thread during graph building
 * - updateAllClients(): called from message thread (timer)
 * - getLatestLevels(): called from message thread (UI)
 */
class DeviceMeteringManager {
  public:
    DeviceMeteringManager() = default;
    ~DeviceMeteringManager() = default;

#ifdef AIDAW_HAS_TRACKTION
    te::LevelMeasurer& getOrCreateMeasurer(DeviceId deviceId);
    void removeMeasurer(DeviceId deviceId);
#endif

    void updateAllClients();

    struct DeviceMeterData {
        float peakL = 0.f;
        float peakR = 0.f;
    };

    bool getLatestLevels(DeviceId deviceId, DeviceMeterData& out) const;

    void setGain(DeviceId deviceId, float gainLinear);
    std::atomic<float>* getGainAtomic(DeviceId deviceId);

    void setDirectLevels(DeviceId deviceId, float peakL, float peakR);
    void ensureEntry(DeviceId deviceId);

    struct RealtimeTapStorage {
        std::atomic<float> peakL{0.f};
        std::atomic<float> peakR{0.f};
        std::atomic<float> gainLinear{1.0f};
    };

    struct RealtimeTap {
        std::shared_ptr<RealtimeTapStorage> storage;
        std::atomic<float>* peakL = nullptr;
        std::atomic<float>* peakR = nullptr;
        std::atomic<float>* gainLinear = nullptr;

        bool isValid() const {
            return storage != nullptr && peakL != nullptr && peakR != nullptr &&
                   gainLinear != nullptr;
        }
    };

    RealtimeTap getRealtimeTap(DeviceId deviceId);

    void setRackDirectLevels(RackId rackId, float peakL, float peakR);
    void ensureRackEntry(RackId rackId);
    bool getRackLatestLevels(RackId rackId, DeviceMeterData& out) const;

    void clear();

#ifdef AIDAW_HAS_TRACKTION
    static DeviceMeteringManager* getInstanceForEdit(te::Edit& edit);
    static void registerForEdit(te::Edit& edit, DeviceMeteringManager* mgr);
    static void unregisterForEdit(te::Edit& edit);
#endif

  private:
    struct Entry {
#ifdef AIDAW_HAS_TRACKTION
        te::LevelMeasurer measurer;
        te::LevelMeasurer::Client client;
#endif
        std::atomic<float> peakL{0.f};
        std::atomic<float> peakR{0.f};
        std::atomic<float> gainLinear{1.0f};
        std::shared_ptr<RealtimeTapStorage> realtimeTap;
        bool clientRegistered = false;
    };

    struct SimpleEntry {
        std::atomic<float> peakL{0.f};
        std::atomic<float> peakR{0.f};
    };

    std::map<DeviceId, std::unique_ptr<Entry>> entries_;
    std::map<RackId, std::unique_ptr<SimpleEntry>> rackEntries_;
    juce::CriticalSection lock_;

#ifdef AIDAW_HAS_TRACKTION
    static std::map<te::Edit*, DeviceMeteringManager*> editMap_;
    static juce::CriticalSection editMapLock_;
#endif
};

}  // namespace aidaw
