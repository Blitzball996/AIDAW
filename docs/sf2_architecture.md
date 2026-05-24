# SF2 SoundFont Player — Architecture

## Overview

Integrate a General MIDI SoundFont (SF2) player into Blitz as a native instrument plugin. Uses **TinySoundFont** (tsf.h) — a single-header MIT-licensed C library (~2000 lines) that handles all SF2 parsing, preset selection, and audio rendering.

## Approach

- **Library**: TinySoundFont (github.com/schellingb/TinySoundFont)
- **Integration**: Drop `tsf.h` into `third_party/tsf/`, `#define TSF_IMPLEMENTATION` in one .cpp
- **Plugin pattern**: Follow `MagdaSamplerPlugin` — extends `te::Plugin`, registered via `InternalPluginRegistry`
- **No custom SF2 parser** — tsf.h handles all SF2 format details internally

## File Structure

```
third_party/tsf/
  tsf.h                          # TinySoundFont single-header library

magda/daw/audio/plugins/
  SoundFontPlugin.hpp            # Plugin class declaration
  SoundFontPlugin.cpp            # Plugin implementation (TSF_IMPLEMENTATION here)

magda/daw/audio/processors/internal/
  SoundFontProcessor.hpp         # DeviceProcessor wrapper (in NativeDeviceProcessors.hpp)

assets/soundfonts/
  GeneralUser_GS.sf2             # Default GM soundfont (already present)
```

## Plugin Class: SoundFontPlugin

```cpp
class SoundFontPlugin : public te::Plugin {
public:
    SoundFontPlugin(const te::PluginCreationInfo&);
    ~SoundFontPlugin() override;

    static const char* getPluginName() { return "SoundFont"; }
    static const char* xmlTypeName;  // = "soundfont"

    // te::Plugin overrides
    void initialise(const te::PluginInitialisationInfo&) override;
    void deinitialise() override;
    void reset() override;
    void applyToBuffer(const te::PluginRenderContext&) override;

    bool takesMidiInput() override { return true; }
    bool takesAudioInput() override { return false; }
    bool isSynth() override { return true; }
    bool producesAudioWhenNoAudioInput() override { return true; }

    void restorePluginStateFromValueTree(const juce::ValueTree&) override;

    // SF2-specific
    void loadSoundFont(const juce::File& sf2File);
    void setPreset(int bank, int program);

    // Automatable parameters
    te::AutomatableParameter::Ptr programParam;   // 0-127
    te::AutomatableParameter::Ptr bankParam;      // 0-128
    te::AutomatableParameter::Ptr volumeParam;    // 0.0-1.0
    te::AutomatableParameter::Ptr reverbParam;    // 0.0-1.0 reverb send

    // Persisted state
    juce::CachedValue<float> programValue, bankValue, volumeValue, reverbValue;
    juce::CachedValue<juce::String> sf2PathValue;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;  // Pimpl hides tsf* from header

    double sampleRate = 44100.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundFontPlugin)
};
```

## Internal State (Impl)

```cpp
struct SoundFontPlugin::Impl {
    tsf* soundFont = nullptr;
    int currentBank = 0;
    int currentProgram = 0;
    juce::File loadedFile;
    juce::SpinLock lock;  // Protects tsf* during preset changes
};
```

## Audio Processing (applyToBuffer)

The `applyToBuffer` method:
1. Iterates incoming MIDI messages from `PluginRenderContext::bufferForMidiMessages`
2. Dispatches note-on/off/CC to tsf via `tsf_note_on`, `tsf_note_off`, `tsf_channel_midi_control`
3. Calls `tsf_render_float(impl->soundFont, outputBuffer, numSamples, 0)` to fill audio
4. Applies volume parameter as gain

Key detail: tsf renders interleaved stereo. We deinterleave into JUCE's planar buffer format.

## Registration

### 1. InternalDeviceKind enum (core/InternalDeviceKind.hpp)

Add after `MagdaSampler`:
```cpp
SoundFont,
```

### 2. classifyInternalDevice (core/InternalDeviceKind.cpp)

Add case for `"soundfont"` returning `InternalDeviceKind::SoundFont`.

### 3. InternalPluginRegistry.cpp

Add spec entry:
```cpp
{InternalDeviceKind::SoundFont, SoundFontPlugin::xmlTypeName, "SoundFont", "Synth",
 "General MIDI SoundFont instrument for orchestral, keyboard, and GM playback.",
 InternalPluginCreateMode::FreshValueTree, true, true, kSoundFontAliases,
 std::size(kSoundFontAliases), matches<SoundFontPlugin>,
 makeProcessor<SoundFontProcessor>},
```

### 4. MagdaEngineBehaviour.hpp — createCustomPlugin

Add:
```cpp
if (type == daw::audio::SoundFontPlugin::xmlTypeName) {
    return new daw::audio::SoundFontPlugin(info);
}
```

### 5. PluginBrowserContent.cpp — getInternalPlugins

Add:
```cpp
list.push_back(PluginBrowserInfo::createInternal(
    audio::SoundFontPlugin::getPluginName(),
    audio::SoundFontPlugin::xmlTypeName, true, "Synth"));
```

## SoundFontProcessor (DeviceProcessor wrapper)

```cpp
class SoundFontProcessor : public AutomatablePluginProcessor {
public:
    SoundFontProcessor(DeviceId deviceId, te::Plugin::Ptr plugin);
};
```

Follows the same pattern as `MagdaSamplerProcessor` — maps automatable parameters by index for the UI parameter panel.

## Asset Loading

The SF2 file is located at: `assets/soundfonts/GeneralUser_GS.sf2`

CMake copies it to the build output:
```cmake
add_custom_command(TARGET magda POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:magda>/soundfonts"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/assets/soundfonts/GeneralUser_GS.sf2"
        "$<TARGET_FILE_DIR:magda>/soundfonts/GeneralUser_GS.sf2"
)
```

At runtime, the plugin resolves the path via:
```cpp
auto sf2Path = juce::File::getSpecialLocation(
    juce::File::currentExecutableFile).getParentDirectory()
    .getChildFile("soundfonts/GeneralUser_GS.sf2");
```

## Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Program   | 0-127 | 0       | GM instrument preset |
| Bank      | 0-128 | 0       | Bank select (0 = GM) |
| Volume    | 0.0-1.0 | 0.8   | Output level |
| Reverb    | 0.0-1.0 | 0.3   | Reverb send amount (tsf built-in) |

## MIDI Handling

tsf handles polyphony internally (up to 256 voices). The plugin forwards:
- Note On/Off → `tsf_note_on` / `tsf_note_off`
- CC 1 (Mod) → `tsf_channel_midi_control`
- CC 7 (Volume) → mapped to volume param
- CC 10 (Pan) → `tsf_channel_midi_control`
- CC 64 (Sustain) → `tsf_channel_midi_control`
- CC 91 (Reverb) → mapped to reverb param
- Program Change → `tsf_channel_set_presetnumber` + update programParam
- Pitch Bend → `tsf_channel_set_pitchwheel`

## Initialization Sequence

1. Plugin constructed → Impl created, tsf* = nullptr
2. `initialise()` called → load SF2 from persisted path (or default)
3. `tsf_load_filename()` → parse SF2 into memory
4. `tsf_set_output()` → configure for stereo float, match host sample rate
5. `setPreset()` → apply persisted bank/program
6. Ready for `applyToBuffer` calls

## Thread Safety

- `tsf*` is only accessed on the audio thread after `initialise()`
- Preset changes from UI thread use `juce::SpinLock` (very short critical section — just sets an int in tsf)
- File loading happens on message thread, swaps `tsf*` pointer atomically with audio thread paused (via `te::Plugin::suspendProcessing`)

## Build Integration

Add to the relevant CMakeLists.txt:
```cmake
target_include_directories(magda_daw PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/tsf)

target_sources(magda_daw PRIVATE
    audio/plugins/SoundFontPlugin.hpp
    audio/plugins/SoundFontPlugin.cpp
    audio/processors/internal/SoundFontProcessor.hpp)
```

No additional link dependencies — tsf.h is header-only with `TSF_IMPLEMENTATION` defined in SoundFontPlugin.cpp.
