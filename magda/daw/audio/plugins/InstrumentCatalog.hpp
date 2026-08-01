#pragma once

#include <juce_core/juce_core.h>

#include <vector>

namespace magda::audio {

/**
 * @brief Single source of truth for the instruments the app can offer.
 *
 * This catalog used to exist only as a literal inside the plugin browser UI,
 * while the AI agent carried its own hand-written copy listing nine of the GM
 * programs. The two drifted, and the agent could only ever reach the handful
 * of sounds someone had remembered to type into its prompt.
 *
 * Both the browser and the agent prompt are now generated from this table, so
 * adding an instrument here makes it visible to the user and usable by the AI
 * in the same edit.
 */

struct GMInstrument {
    const char* name;
    const char* category;  // Piano, Guitar, Bass, Strings, Brass, Woodwind, …
    int program;           // GM program number, bank 0
};

struct DrumKit {
    const char* name;
    int program;  // GM program number, bank 128
};

struct InternalInstrument {
    const char* name;
    const char* pluginId;  // alias used by TRACK FX / FX
    const char* category;
    const char* description;  // shown to the AI so it can pick sensibly
};

/** GM melodic instruments, bank 0. Referenced as "soundfont:<program>". */
const std::vector<GMInstrument>& getGMInstruments();

/** GM drum kits, bank 128. Referenced as "soundfont:128:<program>". */
const std::vector<DrumKit>& getDrumKits();

/** Built-in synths/samplers, referenced by their alias. */
const std::vector<InternalInstrument>& getInternalInstruments();

/** Effects available to FX, in the order they should be offered. */
const std::vector<const char*>& getEffectAliases();

/** MIDI processors available to FX. */
const std::vector<const char*>& getMidiProcessorAliases();

/**
 * @brief Render the catalog as the INSTRUMENTS section of an AI system prompt.
 *
 * Grouped by category and kept terse — this is prompt budget, not documentation.
 */
juce::String buildInstrumentPromptSection();

}  // namespace magda::audio
