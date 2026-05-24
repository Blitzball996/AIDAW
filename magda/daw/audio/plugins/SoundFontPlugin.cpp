#define TSF_IMPLEMENTATION
#include "SoundFontPlugin.hpp"

#include <tsf.h>

#include "core/AppPaths.hpp"

namespace magda::daw::audio {

const char* SoundFontPlugin::xmlTypeName = "soundfont";

SoundFontPlugin::SoundFontPlugin(const te::PluginCreationInfo& info) : te::Plugin(info) {
    auto um = getUndoManager();

    programValue.referTo(state, juce::Identifier("program"), um, 0);
    bankValue.referTo(state, juce::Identifier("bank"), um, 0);

    volumeParam = addParam("volume", TRANS("Volume"), {0.0f, 1.0f},
                           [](float v) { return juce::String(v, 2); },
                           [](const juce::String& s) { return s.getFloatValue(); });
    volumeParam->setParameter(0.8f, juce::dontSendNotification);

    loadSoundFont();
}

SoundFontPlugin::~SoundFontPlugin() {
    if (soundFont_) {
        tsf_close(soundFont_);
        soundFont_ = nullptr;
    }
}

void SoundFontPlugin::loadSoundFont() {
    auto exeDir = juce::File::getSpecialLocation(
        juce::File::currentExecutableFile).getParentDirectory();
    auto sf2File = exeDir.getChildFile("soundfonts").getChildFile("GeneralUser_GS.sf2");

    if (!sf2File.existsAsFile()) {
        DBG("SoundFontPlugin: SF2 not found at " << sf2File.getFullPathName());
        return;
    }

    soundFont_ = tsf_load_filename(sf2File.getFullPathName().toRawUTF8());
    if (soundFont_) {
        tsf_set_output(soundFont_, TSF_STEREO_INTERLEAVED, (int)sampleRate_, 0.0f);
        applyProgramChange();
        DBG("SoundFontPlugin: Loaded " << sf2File.getFullPathName());
    } else {
        DBG("SoundFontPlugin: Failed to parse SF2");
    }
}

void SoundFontPlugin::applyProgramChange() {
    if (!soundFont_)
        return;

    int program = programValue.get();
    int bank = bankValue.get();

    for (int ch = 0; ch < 16; ++ch) {
        tsf_channel_set_bank(soundFont_, ch, bank);
        tsf_channel_set_presetnumber(soundFont_, ch, program, bank != 0 ? 1 : 0);
    }

    lastProgram_ = program;
    lastBank_ = bank;
}

void SoundFontPlugin::initialise(const te::PluginInitialisationInfo& info) {
    sampleRate_ = info.sampleRate;
    if (soundFont_) {
        tsf_set_output(soundFont_, TSF_STEREO_INTERLEAVED, (int)sampleRate_, 0.0f);
        tsf_reset(soundFont_);
    }
}

void SoundFontPlugin::deinitialise() {}

void SoundFontPlugin::reset() {
    if (soundFont_)
        tsf_reset(soundFont_);
}

void SoundFontPlugin::applyToBuffer(const te::PluginRenderContext& rc) {
    if (!soundFont_ || rc.destBuffer == nullptr)
        return;

    int program = programValue.get();
    float volume = volumeParam->getCurrentValue();

    if (rc.bufferForMidiMessages != nullptr) {
        for (auto& m : *rc.bufferForMidiMessages) {
            if (m.isNoteOn()) {
                tsf_note_on(soundFont_, program, m.getNoteNumber(),
                            m.getFloatVelocity());
            } else if (m.isNoteOff()) {
                tsf_note_off(soundFont_, program, m.getNoteNumber());
            } else if (m.isAllNotesOff() || m.isAllSoundOff()) {
                tsf_reset(soundFont_);
            }
        }
    }

    int numSamples = rc.bufferNumSamples;
    int startSample = rc.bufferStartSample;
    int numChannels = rc.destBuffer->getNumChannels();

    juce::HeapBlock<float> interleaved(static_cast<size_t>(numSamples * 2));
    tsf_render_float(soundFont_, interleaved.getData(), numSamples, 0);

    for (int i = 0; i < numSamples; ++i) {
        float l = interleaved[static_cast<size_t>(i * 2)] * volume;
        float r = interleaved[static_cast<size_t>(i * 2 + 1)] * volume;
        if (numChannels >= 1)
            rc.destBuffer->addSample(0, startSample + i, l);
        if (numChannels >= 2)
            rc.destBuffer->addSample(1, startSample + i, r);
    }
}

void SoundFontPlugin::restorePluginStateFromValueTree(const juce::ValueTree& v) {
    te::copyPropertiesToCachedValues(v, programValue, bankValue);

    for (auto p : getAutomatableParameters())
        p->updateFromAttachedValue();

    if (soundFont_)
        applyProgramChange();
}

}  // namespace magda::daw::audio
