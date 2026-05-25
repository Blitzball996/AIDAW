#include "plugins/SfizzPlugin.hpp"

#include <sfizz.hpp>

#include "core/AppPaths.hpp"

namespace magda::daw::audio {

const char* SfizzPlugin::xmlTypeName = "sfizz";

SfizzPlugin::SfizzPlugin(const te::PluginCreationInfo& info) : te::Plugin(info) {
    auto um = getUndoManager();
    sfzFileValue.referTo(state, juce::Identifier("sfzFile"), um, "");

    volumeParam = addParam("volume", TRANS("Volume"), {0.0f, 1.0f},
                           [](float v) { return juce::String(v, 2); },
                           [](const juce::String& s) { return s.getFloatValue(); });
    volumeParam->setParameter(0.8f, juce::dontSendNotification);

    synth_ = std::make_unique<sfz::Sfizz>();
    synth_->setSampleRate(static_cast<float>(sampleRate_));
    synth_->setSamplesPerBlock(blockSize_);

    loadSfzFile();
}

SfizzPlugin::~SfizzPlugin() = default;

void SfizzPlugin::initialise(const te::PluginInitialisationInfo& info) {
    sampleRate_ = info.sampleRate;
    blockSize_ = info.blockSizeSamples;
    if (synth_) {
        synth_->setSampleRate(static_cast<float>(sampleRate_));
        synth_->setSamplesPerBlock(blockSize_);
    }
}

void SfizzPlugin::deinitialise() {}

void SfizzPlugin::reset() {
    if (synth_)
        synth_->allSoundOff();
}

void SfizzPlugin::applyToBuffer(const te::PluginRenderContext& rc) {
    if (!synth_ || rc.destBuffer == nullptr)
        return;

    float volume = volumeParam->getCurrentValue();
    int numSamples = rc.bufferNumSamples;
    int startSample = rc.bufferStartSample;

    // Process MIDI events
    if (rc.bufferForMidiMessages != nullptr) {
        for (auto& m : *rc.bufferForMidiMessages) {
            int samplePos = static_cast<int>(m.getTimeStamp());
            samplePos = juce::jlimit(0, numSamples - 1, samplePos);

            if (m.isNoteOn()) {
                synth_->noteOn(samplePos, m.getNoteNumber(), m.getVelocity());
            } else if (m.isNoteOff()) {
                synth_->noteOff(samplePos, m.getNoteNumber(), 0);
            } else if (m.isPitchWheel()) {
                synth_->pitchWheel(samplePos, m.getPitchWheelValue());
            } else if (m.isController()) {
                synth_->cc(samplePos, m.getControllerNumber(), m.getControllerValue());
            } else if (m.isAllNotesOff() || m.isAllSoundOff()) {
                synth_->allSoundOff();
            }
        }
    }

    // Render audio
    int numChannels = rc.destBuffer->getNumChannels();
    if (numChannels < 2) return;

    float* outputs[2] = {
        rc.destBuffer->getWritePointer(0) + startSample,
        rc.destBuffer->getWritePointer(1) + startSample
    };

    // sfizz renders into provided buffers (additive)
    // We need temp buffers since sfizz overwrites
    juce::AudioBuffer<float> tempBuffer(2, numSamples);
    tempBuffer.clear();
    float* tempPtrs[2] = {tempBuffer.getWritePointer(0), tempBuffer.getWritePointer(1)};

    synth_->renderBlock(tempPtrs, static_cast<size_t>(numSamples), 2);

    // Mix into output with volume
    for (int i = 0; i < numSamples; ++i) {
        outputs[0][i] += tempPtrs[0][i] * volume;
        outputs[1][i] += tempPtrs[1][i] * volume;
    }
}

void SfizzPlugin::restorePluginStateFromValueTree(const juce::ValueTree& v) {
    if (v.hasProperty("sfzFile"))
        sfzFileValue = v.getProperty("sfzFile").toString();

    for (auto p : getAutomatableParameters())
        p->updateFromAttachedValue();

    loadSfzFile();
}

juce::File SfizzPlugin::getSfzDirectory() const {
    auto exeDir = juce::File::getSpecialLocation(
        juce::File::currentExecutableFile).getParentDirectory();
    return exeDir.getChildFile("sfz");
}

void SfizzPlugin::loadSfzFile() {
    if (!synth_) return;

    juce::String fileName = sfzFileValue.get();
    if (fileName.isEmpty()) return;

    auto sfzFile = getSfzDirectory().getChildFile(fileName);
    if (!sfzFile.existsAsFile()) {
        DBG("SfizzPlugin: SFZ not found at " << sfzFile.getFullPathName());
        return;
    }

    if (synth_->loadSfzFile(sfzFile.getFullPathName().toStdString())) {
        DBG("SfizzPlugin: Loaded " << sfzFile.getFullPathName());
    } else {
        DBG("SfizzPlugin: Failed to load " << sfzFile.getFullPathName());
    }
}

juce::StringArray SfizzPlugin::getAvailableSfzFiles() const {
    juce::StringArray result;
    auto dir = getSfzDirectory();
    if (dir.isDirectory()) {
        for (const auto& entry : juce::RangedDirectoryIterator(dir, true, "*.sfz")) {
            auto relative = entry.getFile().getRelativePathFrom(dir);
            result.add(relative);
        }
    }
    result.sort(true);
    return result;
}

juce::String SfizzPlugin::getCurrentSfzFile() const {
    return sfzFileValue.get();
}

void SfizzPlugin::loadSfzByName(const juce::String& name) {
    sfzFileValue = name;
    loadSfzFile();
}

}  // namespace magda::daw::audio
