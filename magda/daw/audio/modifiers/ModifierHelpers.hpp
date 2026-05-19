#pragma once

#include <tracktion_engine/tracktion_engine.h>

#include <unordered_map>

#include "../../core/ModInfo.hpp"
#include "modifiers/CurveSnapshot.hpp"

namespace magda {

namespace te = tracktion;

struct ModifierAssignmentMapping {
    float value = 0.0f;
    float offset = 0.0f;
};

inline ModifierAssignmentMapping mapLinkAssignment(float amount, bool bipolar) {
    return {bipolar ? amount * 2.0f : amount, bipolar ? -amount : 0.0f};
}

template <typename Link>
inline te::AutomatableParameter::ModifierAssignment::Ptr addLinkModifier(
    te::AutomatableParameter& param, te::AutomatableParameter::ModifierSource& source,
    const Link& link) {
    const auto mapping = mapLinkAssignment(link.amount, link.bipolar);
    return param.addModifier(source, mapping.value, mapping.offset);
}

inline float mapWaveform(LFOWaveform waveform) {
    switch (waveform) {
        case LFOWaveform::Sine:
            return 0.0f;
        case LFOWaveform::Triangle:
            return 1.0f;
        case LFOWaveform::Saw:
            return 2.0f;
        case LFOWaveform::ReverseSaw:
            return 3.0f;
        case LFOWaveform::Square:
            return 4.0f;
        case LFOWaveform::Custom:
            return 0.0f;
    }
    return 0.0f;
}

inline float mapSyncDivision(SyncDivision div) {
    using RT = te::ModifierCommon::RateType;
    static const std::unordered_map<SyncDivision, RT> mapping = {
        // {SyncDivision::SixteenBars, RT::sixteenBars}, // not in this TE version
        // {SyncDivision::EightBars, RT::eightBars}, // not in this TE version
        {SyncDivision::FourBars, RT::fourBars},
        {SyncDivision::TwoBars, RT::twoBars},
        {SyncDivision::Whole, RT::bar},
        {SyncDivision::Half, RT::half},
        {SyncDivision::Quarter, RT::quarter},
        {SyncDivision::Eighth, RT::eighth},
        {SyncDivision::Sixteenth, RT::sixteenth},
        {SyncDivision::ThirtySecond, RT::thirtySecond},
        {SyncDivision::DottedHalf, RT::halfD},
        {SyncDivision::DottedQuarter, RT::quarterD},
        {SyncDivision::DottedEighth, RT::eighthD},
        {SyncDivision::DottedSixteenth, RT::sixteenthD},
        {SyncDivision::DottedThirtySecond, RT::thirtySecondD},
        {SyncDivision::TripletHalf, RT::halfT},
        {SyncDivision::TripletQuarter, RT::quarterT},
        {SyncDivision::TripletEighth, RT::eighthT},
        {SyncDivision::TripletSixteenth, RT::sixteenthT},
        {SyncDivision::TripletThirtySecond, RT::thirtySecondT},
    };
    auto it = mapping.find(div);
    return static_cast<float>(it != mapping.end() ? it->second : RT::quarter);
}

/**
 * @brief Map MAGDA trigger/sync settings to TE syncType
 *
 * TE syncType: 0=free (Hz rate), 1=transport (tempo-synced), 2=note (MIDI retrigger)
 * Note mode can use either Hz rate (rateType=hertz) or musical divisions
 * (rateType=bar/quarter/etc.) depending on whether tempoSync is enabled.
 */
inline float mapSyncType(const ModInfo& modInfo) {
    // MIDI and Audio triggers → TE note mode (2): resets phase on triggerNoteOn()
    // Both are gate-triggered — the only difference is the trigger source.
    if (modInfo.triggerMode == LFOTriggerMode::MIDI || modInfo.triggerMode == LFOTriggerMode::Audio)
        return 2.0f;
    // Transport trigger or tempo sync both use transport mode (1)
    if (modInfo.tempoSync || modInfo.triggerMode == LFOTriggerMode::Transport)
        return 1.0f;
    // Free running in Hz
    return 0.0f;
}

inline void applyLFOProperties(te::LFOModifier* lfo, const ModInfo& modInfo,
                               CurveSnapshotHolder* holder = nullptr) {
    float syncType = mapSyncType(modInfo);

    // rateType determines Hz vs musical divisions in TE's LFO timer.
    // Only use musical divisions when tempoSync is explicitly enabled.
    // MIDI trigger (syncType=2) can work with either Hz or musical rate —
    // it just resets the phase on note-on regardless of rateType.
    float rateType = modInfo.tempoSync ? mapSyncDivision(modInfo.syncDivision)
                                       : static_cast<float>(te::ModifierCommon::hertz);

    if (modInfo.waveform == LFOWaveform::Custom && holder) {
        // Custom waveform — requires magda TE fork APIs (waveCustomCallback, customWaveFunction)
        // TODO: re-enable when using magda's TE fork
        holder->update(modInfo);
        lfo->depthParam->setParameter(1.0f, juce::dontSendNotification);
    } else {
        lfo->waveParam->setParameter(mapWaveform(modInfo.waveform),
                                             juce::dontSendNotification);
        lfo->depthParam->setParameter(1.0f, juce::dontSendNotification);
    }

    float teRate = modInfo.tempoSync ? 1.0f : modInfo.rate;
    lfo->rateParam->setParameter(teRate, juce::dontSendNotification);
    lfo->phaseParam->setParameter(modInfo.phaseOffset, juce::dontSendNotification);
    lfo->syncTypeParam->setParameter(syncType, juce::dontSendNotification);
    lfo->rateTypeParam->setParameter(rateType, juce::dontSendNotification);

    // TODO: setGated/setGateOnTriggerSource not in upstream TE — magda fork only
    (void)modInfo;
}

// TODO: triggerLFONoteOnWithReset and clearLFOCustomWaveCallbacks require magda TE fork
inline void triggerLFONoteOnWithReset(te::LFOModifier* /*lfo*/, bool /*forceZeroValue*/ = true) {}
inline void clearLFOCustomWaveCallbacks(const std::vector<te::Modifier::Ptr>& /*modifiers*/) {}

template <typename ModMap> inline void clearLFOCustomWaveCallbacks(const ModMap& modifierMap) {
    for (auto& [id, mod] : modifierMap) {
        if (auto* lfo = dynamic_cast<te::LFOModifier*>(mod.get())) {
            // lfo->customWaveFunction.store(nullptr, std::memory_order_release);
            // lfo->customWaveUserData.store(nullptr, std::memory_order_release);
        }
    }
}

/**
 * @brief Move CurveSnapshotHolders to a deferred-deletion list before destroying their owner.
 *
 * After clearing LFO callback pointers (clearLFOCustomWaveCallbacks), the audio thread
 * may still be mid-call inside evaluateCallback with a pointer loaded before the null
 * store was visible. Deferring destruction ensures the holder memory stays valid until
 * the next sync cycle, by which time the audio thread has moved on.
 */
inline void deferCurveSnapshots(std::map<ModId, std::unique_ptr<CurveSnapshotHolder>>& snapshots,
                                std::vector<std::unique_ptr<CurveSnapshotHolder>>& deferred) {
    for (auto& [id, holder] : snapshots) {
        if (holder)
            deferred.push_back(std::move(holder));
    }
}

}  // namespace magda
