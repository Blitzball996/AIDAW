#pragma once

#include <array>

namespace aidaw {
class MidiNoteStrip;
}

namespace aidaw::daw::ui {

class DeviceCustomUIManager;
struct DeviceSlotTraits;

void refreshDeviceSlotMidiActivity(const DeviceSlotTraits& traits,
                                   const DeviceCustomUIManager& customUI,
                                   aidaw::MidiNoteStrip& midiNoteStrip, int& lastSingleNote,
                                   std::array<int, 32>& lastChordNotes, int& lastChordCount);

}  // namespace aidaw::daw::ui
