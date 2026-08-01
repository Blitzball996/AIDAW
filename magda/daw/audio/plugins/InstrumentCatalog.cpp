#include "InstrumentCatalog.hpp"

#include "InternalPluginRegistry.hpp"

namespace magda::audio {

namespace {

// Full GM 1 melodic set, bank 0. Categories are the browser's subcategory
// names so the same value drives the browser badge and the AI prompt grouping.
const std::vector<GMInstrument> kGM = {
    // Piano 0-7
    {"Acoustic Grand Piano", "Piano", 0},
    {"Bright Acoustic Piano", "Piano", 1},
    {"Electric Grand Piano", "Piano", 2},
    {"Honky-tonk Piano", "Piano", 3},
    {"Electric Piano 1", "Piano", 4},
    {"Electric Piano 2", "Piano", 5},
    {"Harpsichord", "Piano", 6},
    {"Clavinet", "Piano", 7},
    // Chromatic percussion 8-15
    {"Celesta", "Chromatic", 8},
    {"Glockenspiel", "Chromatic", 9},
    {"Music Box", "Chromatic", 10},
    {"Vibraphone", "Chromatic", 11},
    {"Marimba", "Chromatic", 12},
    {"Xylophone", "Chromatic", 13},
    {"Tubular Bells", "Chromatic", 14},
    {"Dulcimer", "Chromatic", 15},
    // Organ 16-23
    {"Drawbar Organ", "Organ", 16},
    {"Percussive Organ", "Organ", 17},
    {"Rock Organ", "Organ", 18},
    {"Church Organ", "Organ", 19},
    {"Reed Organ", "Organ", 20},
    {"Accordion", "Organ", 21},
    {"Harmonica", "Organ", 22},
    {"Tango Accordion", "Organ", 23},
    // Guitar 24-31
    {"Nylon Guitar", "Guitar", 24},
    {"Steel Guitar", "Guitar", 25},
    {"Jazz Guitar", "Guitar", 26},
    {"Clean Electric Guitar", "Guitar", 27},
    {"Muted Guitar", "Guitar", 28},
    {"Overdriven Guitar", "Guitar", 29},
    {"Distortion Guitar", "Guitar", 30},
    {"Guitar Harmonics", "Guitar", 31},
    // Bass 32-39
    {"Acoustic Bass", "Bass", 32},
    {"Finger Bass", "Bass", 33},
    {"Pick Bass", "Bass", 34},
    {"Fretless Bass", "Bass", 35},
    {"Slap Bass 1", "Bass", 36},
    {"Slap Bass 2", "Bass", 37},
    {"Synth Bass 1", "Bass", 38},
    {"Synth Bass 2", "Bass", 39},
    // Solo strings 40-47
    {"Violin", "Strings", 40},
    {"Viola", "Strings", 41},
    {"Cello", "Strings", 42},
    {"Contrabass", "Strings", 43},
    {"Tremolo Strings", "Strings", 44},
    {"Pizzicato Strings", "Strings", 45},
    {"Orchestral Harp", "Strings", 46},
    {"Timpani", "Percussion", 47},
    // Ensemble 48-55
    {"String Ensemble 1", "Ensemble", 48},
    {"String Ensemble 2", "Ensemble", 49},
    {"Synth Strings 1", "Ensemble", 50},
    {"Synth Strings 2", "Ensemble", 51},
    {"Choir Aahs", "Ensemble", 52},
    {"Voice Oohs", "Ensemble", 53},
    {"Synth Voice", "Ensemble", 54},
    {"Orchestra Hit", "Ensemble", 55},
    // Brass 56-63
    {"Trumpet", "Brass", 56},
    {"Trombone", "Brass", 57},
    {"Tuba", "Brass", 58},
    {"Muted Trumpet", "Brass", 59},
    {"French Horn", "Brass", 60},
    {"Brass Section", "Brass", 61},
    {"Synth Brass 1", "Brass", 62},
    {"Synth Brass 2", "Brass", 63},
    // Reed 64-71
    {"Soprano Sax", "Woodwind", 64},
    {"Alto Sax", "Woodwind", 65},
    {"Tenor Sax", "Woodwind", 66},
    {"Baritone Sax", "Woodwind", 67},
    {"Oboe", "Woodwind", 68},
    {"English Horn", "Woodwind", 69},
    {"Bassoon", "Woodwind", 70},
    {"Clarinet", "Woodwind", 71},
    // Pipe 72-79
    {"Piccolo", "Woodwind", 72},
    {"Flute", "Woodwind", 73},
    {"Recorder", "Woodwind", 74},
    {"Pan Flute", "Woodwind", 75},
    {"Blown Bottle", "Woodwind", 76},
    {"Shakuhachi", "Woodwind", 77},
    {"Whistle", "Woodwind", 78},
    {"Ocarina", "Woodwind", 79},
    // Synth lead 80-87
    {"Square Lead", "Synth", 80},
    {"Sawtooth Lead", "Synth", 81},
    {"Calliope Lead", "Synth", 82},
    {"Chiff Lead", "Synth", 83},
    {"Charang Lead", "Synth", 84},
    {"Voice Lead", "Synth", 85},
    {"Fifths Lead", "Synth", 86},
    {"Bass + Lead", "Synth", 87},
    // Synth pad 88-95
    {"New Age Pad", "Pad", 88},
    {"Warm Pad", "Pad", 89},
    {"Polysynth Pad", "Pad", 90},
    {"Choir Pad", "Pad", 91},
    {"Bowed Pad", "Pad", 92},
    {"Metallic Pad", "Pad", 93},
    {"Halo Pad", "Pad", 94},
    {"Sweep Pad", "Pad", 95},
    // Synth effects 96-103
    {"Rain FX", "Pad", 96},
    {"Soundtrack FX", "Pad", 97},
    {"Crystal FX", "Pad", 98},
    {"Atmosphere FX", "Pad", 99},
    {"Brightness FX", "Pad", 100},
    {"Goblins FX", "Pad", 101},
    {"Echoes FX", "Pad", 102},
    {"Sci-Fi FX", "Pad", 103},
    // Ethnic 104-111
    {"Sitar", "Ethnic", 104},
    {"Banjo", "Ethnic", 105},
    {"Shamisen", "Ethnic", 106},
    {"Koto", "Ethnic", 107},
    {"Kalimba", "Ethnic", 108},
    {"Bagpipe", "Ethnic", 109},
    {"Fiddle", "Ethnic", 110},
    {"Shanai", "Ethnic", 111},
    // Percussive 112-119
    {"Tinkle Bell", "Percussion", 112},
    {"Agogo", "Percussion", 113},
    {"Steel Drums", "Percussion", 114},
    {"Woodblock", "Percussion", 115},
    {"Taiko Drum", "Percussion", 116},
    {"Melodic Tom", "Percussion", 117},
    {"Synth Drum", "Percussion", 118},
    {"Reverse Cymbal", "Percussion", 119},
    // Sound effects 120-127
    {"Guitar Fret Noise", "SFX", 120},
    {"Breath Noise", "SFX", 121},
    {"Seashore", "SFX", 122},
    {"Bird Tweet", "SFX", 123},
    {"Telephone Ring", "SFX", 124},
    {"Helicopter", "SFX", 125},
    {"Applause", "SFX", 126},
    {"Gunshot", "SFX", 127},
};

const std::vector<DrumKit> kDrumKits = {
    {"Standard Kit", 0}, {"Room Kit", 8},   {"Power Kit", 16}, {"Electronic Kit", 24},
    {"TR-808 Kit", 25},  {"Jazz Kit", 32},  {"Brush Kit", 40}, {"Orchestra Kit", 48},
};

const std::vector<InternalInstrument> kInternal = {
    {"4OSC Synth", "4osc", "Synth", "Subtractive synth, 4 oscillators — leads, basses, pads"},
    {"SoundFont Player", "soundfont", "Multi", "SF2 multi-timbral player (see GM programs below)"},
    {"MAGDA Sampler", "magdasampler", "Sampler", "Sample-based instrument, load any audio file"},
    {"Drum Grid", "drumgrid", "Drums", "Drum machine with a step sequencer"},
    {"Sfizz", "sfizz", "Sampler", "High-quality SFZ player, load any SFZ library"},
};

/** Category display order — keeps related sounds adjacent in the prompt. */
const char* const kCategoryOrder[] = {
    "Piano", "Chromatic", "Organ",  "Guitar",     "Bass", "Strings", "Ensemble", "Brass",
    "Synth", "Pad",       "Ethnic", "Percussion", "SFX",
};

}  // namespace

const std::vector<GMInstrument>& getGMInstruments() {
    return kGM;
}

const std::vector<DrumKit>& getDrumKits() {
    return kDrumKits;
}

const std::vector<InternalInstrument>& getInternalInstruments() {
    return kInternal;
}

const std::vector<const char*>& getEffectAliases() {
    // Built once from the plugin registry so the prompt cannot drift from what
    // FX will actually accept.
    static const std::vector<const char*> aliases = [] {
        std::vector<const char*> out;
        for (const auto* spec : daw::audio::getAllInternalPluginSpecs()) {
            if (spec == nullptr || spec->loadAliasCount <= 0 || spec->loadAliases == nullptr)
                continue;
            juce::String category(spec->browserCategory != nullptr ? spec->browserCategory : "");
            if (category == "MIDI" || category == "Instrument")
                continue;
            out.push_back(spec->loadAliases[0]);
        }
        return out;
    }();
    return aliases;
}

const std::vector<const char*>& getMidiProcessorAliases() {
    static const std::vector<const char*> aliases = [] {
        std::vector<const char*> out;
        for (const auto* spec : daw::audio::getAllInternalPluginSpecs()) {
            if (spec == nullptr || spec->loadAliasCount <= 0 || spec->loadAliases == nullptr)
                continue;
            if (juce::String(spec->browserCategory != nullptr ? spec->browserCategory : "")
                != "MIDI")
                continue;
            out.push_back(spec->loadAliases[0]);
        }
        return out;
    }();
    return aliases;
}

juce::String buildInstrumentPromptSection() {
    juce::String out;

    out << "AVAILABLE INSTRUMENTS (use with TRACK FX or FX):\n";
    for (const auto& inst : kInternal)
        out << "  " << inst.pluginId << "  - " << inst.name << " (" << inst.description << ")\n";

    out << "\nGM PROGRAMS (use as soundfont:<program>) — all 128 are loadable:\n";
    for (const char* category : kCategoryOrder) {
        juce::String line;
        for (const auto& gm : kGM) {
            if (juce::String(gm.category) != category)
                continue;
            if (line.isNotEmpty())
                line << ", ";
            line << gm.program << "=" << gm.name;
        }
        if (line.isNotEmpty())
            out << "  " << category << ": " << line << "\n";
    }

    out << "\nDRUM KITS (use as drums:<program>):\n  ";
    bool first = true;
    for (const auto& dk : kDrumKits) {
        if (!first)
            out << ", ";
        out << dk.program << "=" << dk.name;
        first = false;
    }
    out << "\n";

    out << "\nAVAILABLE EFFECTS (use with FX):\n  ";
    first = true;
    for (const char* alias : getEffectAliases()) {
        if (!first)
            out << ", ";
        out << alias;
        first = false;
    }
    out << "\n";

    const auto& midiAliases = getMidiProcessorAliases();
    if (!midiAliases.empty()) {
        out << "\nMIDI PROCESSORS (use with FX):\n  ";
        first = true;
        for (const char* alias : midiAliases) {
            if (!first)
                out << ", ";
            out << alias;
            first = false;
        }
        out << "\n";
    }

    return out;
}

}  // namespace magda::audio
