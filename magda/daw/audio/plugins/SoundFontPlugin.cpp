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
    soundFontFileValue.referTo(state, juce::Identifier("soundFontFile"), um, "GeneralUser_GS.sf2");

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
    juce::String fileName = soundFontFileValue.get();
    if (fileName.isEmpty())
        fileName = "GeneralUser_GS.sf2";

    auto sf2File = getSoundFontsDirectory().getChildFile(fileName);
    loadSoundFontFromFile(sf2File);
}

void SoundFontPlugin::loadSoundFontFromFile(const juce::File& sf2File) {
    if (soundFont_) {
        tsf_close(soundFont_);
        soundFont_ = nullptr;
    }

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
            int ch = juce::jmax(0, m.getChannel() - 1);
            if (m.isNoteOn()) {
                tsf_channel_note_on(soundFont_, ch, m.getNoteNumber(),
                                    m.getFloatVelocity());
            } else if (m.isNoteOff()) {
                tsf_channel_note_off(soundFont_, ch, m.getNoteNumber());
            } else if (m.isPitchWheel()) {
                tsf_channel_set_pitchwheel(soundFont_, ch, m.getPitchWheelValue());
            } else if (m.isController()) {
                tsf_channel_midi_control(soundFont_, ch, m.getControllerNumber(),
                                         m.getControllerValue());
            } else if (m.isAllNotesOff() || m.isAllSoundOff()) {
                tsf_channel_note_off_all(soundFont_, ch);
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

    if (v.hasProperty("soundFontFile"))
        soundFontFileValue = v.getProperty("soundFontFile").toString();

    for (auto p : getAutomatableParameters())
        p->updateFromAttachedValue();

    loadSoundFont();
}

juce::File SoundFontPlugin::getSoundFontsDirectory() const {
    auto exeDir = juce::File::getSpecialLocation(
        juce::File::currentExecutableFile).getParentDirectory();
    return exeDir.getChildFile("soundfonts");
}

juce::StringArray SoundFontPlugin::getAvailableSoundFonts() const {
    juce::StringArray result;
    auto dir = getSoundFontsDirectory();
    if (dir.isDirectory()) {
        for (const auto& entry : juce::RangedDirectoryIterator(dir, false, "*.sf2;*.sf3")) {
            result.add(entry.getFile().getFileName());
        }
    }
    result.sort(true);
    return result;
}

juce::String SoundFontPlugin::getCurrentSoundFontName() const {
    return soundFontFileValue.get();
}

void SoundFontPlugin::loadSoundFontByName(const juce::String& name) {
    soundFontFileValue = name;
    loadSoundFont();
}

}  // namespace magda::daw::audio
