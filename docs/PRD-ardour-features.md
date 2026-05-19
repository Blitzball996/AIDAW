# AIDAW — Product Requirements Document (PRD)

## 1. Overview

AIDAW is a dual-mode DAW combining professional audio production with AI-powered composition. This PRD defines features to add beyond the current magda-core baseline, referencing Ardour and other major DAWs.

## 2. Current State (magda-core baseline)

Already implemented:
- Multi-track audio/MIDI recording via Tracktion Engine
- VST3/AU plugin hosting
- AI agents (Music, Automation, Command, Sound Design, Faust)
- Session view (clip launcher)
- Arrangement view with timeline
- Mixer with channel strips
- Piano roll / drum grid editors
- Chord engine and scale detection
- Lua scripting
- Project save/load

## 3. Features to Add (from Ardour analysis)

### 3.1 Audio Processing (Priority: HIGH)

| Feature | Ardour Reference | Description |
|---------|-----------------|-------------|
| **Convolution Reverb** | `libs/ardour/convolver.h` | IR-based reverb with impulse response loading |
| **Strip Silence** | `libs/ardour/strip_silence.h` | Auto-detect and remove silent regions |
| **Normalize** | audio region operations | Normalize audio to target level |
| **Reverse** | `libs/ardour/reverse.h` | Reverse audio regions |
| **Time Stretch** | `libs/ardour/stretch.h` | Elastique/RubberBand time stretching |
| **Pitch Shift** | `libs/ardour/pitch.h` | Independent pitch shifting |
| **DSP Filters** | `libs/ardour/dsp_filter.h` | Built-in DSP filter library |
| **Loudness Metering** | `libs/ardour/lufs_meter.h` | LUFS/EBU R128 loudness measurement |

### 3.2 Surround & Spatial Audio (Priority: MEDIUM)

| Feature | Ardour Reference | Description |
|---------|-----------------|-------------|
| **Surround Panning** | `surround_pannable.h` | Multi-channel surround panning |
| **Surround Send/Return** | `surround_send.h`, `surround_return.h` | Surround bus routing |
| **Dolby Atmos** | Ardour 8.x | Object-based spatial audio |

### 3.3 Trigger/Clip Launching (Priority: HIGH)

| Feature | Ardour Reference | Description |
|---------|-----------------|-------------|
| **Trigger Box** | `libs/ardour/triggerbox.h` | Ableton-style clip triggering per track |
| **Clip Library** | `libs/ardour/clip_library.h` | Browse and manage audio/MIDI clips |
| **Cue/Scene Launch** | trigger system | Launch entire scenes (rows of clips) |
| **Beat Box** | `libs/ardour/beatbox.h` | Built-in beat/pattern sequencer |

### 3.4 Export System (Priority: HIGH)

| Feature | Ardour Reference | Description |
|---------|-----------------|-------------|
| **Multi-format Export** | `export_format_*.h` (12 files) | WAV/AIFF/FLAC/MP3/OGG export |
| **Export Presets** | `export_preset.h` | Save/recall export configurations |
| **Stem Export** | `export_channel_configuration.h` | Export individual tracks/buses |
| **Export Timespan** | `export_timespan.h` | Export specific time ranges |
| **Export Analysis** | `export_analysis.h` | Post-export loudness/peak analysis |

### 3.5 Advanced MIDI (Priority: MEDIUM)

| Feature | Ardour Reference | Description |
|---------|-----------------|-------------|
| **MIDI Quantize** | `libs/ardour/quantize.h` | Quantize with strength/swing |
| **MIDI Stretch** | `libs/ardour/midi_stretch.h` | Time-stretch MIDI regions |
| **MIDI Scene Changes** | `midi_scene_change.h` | Program changes at markers |
| **Groove Templates** | quantize system | Apply groove from audio/MIDI |

### 3.6 Routing & Monitoring (Priority: HIGH)

| Feature | Ardour Reference | Description |
|---------|-----------------|-------------|
| **VCA Faders** | `libs/ardour/vca.h`, `vca_manager.h` | Virtual Control Amplifiers |
| **Route Groups** | `route_group.h` | Group tracks for linked control |
| **Monitor Section** | `monitor_processor.h` | Dedicated monitor control |
| **Sidechain** | `libs/ardour/sidechain.h` | Sidechain routing for any plugin |
| **Internal Send/Return** | `internal_send.h`, `internal_return.h` | Flexible internal routing |
| **Latency Compensation** | latency system | Automatic PDC |

### 3.7 Locations & Navigation (Priority: MEDIUM)

| Feature | Ardour Reference | Description |
|---------|-----------------|-------------|
| **Locations/Markers** | `libs/ardour/location.h` | Named markers on timeline |
| **Range Markers** | location system | Define named ranges |
| **Punch In/Out** | location-based | Automatic punch recording |
| **Loop Range** | location-based | Loop playback region |
| **CD Markers** | location system | Red Book CD track markers |

### 3.8 Control Surfaces (Priority: LOW)

| Feature | Ardour Reference | Description |
|---------|-----------------|-------------|
| **OSC Control** | `libs/ctrl-interface/` | Open Sound Control protocol |
| **MIDI Control** | control protocol system | Generic MIDI controller mapping |
| **Mackie Control** | control surfaces | MCU/HUI protocol support |

### 3.9 Video (Priority: LOW)

| Feature | Ardour Reference | Description |
|---------|-----------------|-------------|
| **Video Timeline** | `add_video_dialog.h` | Video track for scoring |
| **Video Import** | FFmpeg integration | Import video files |
| **Video Sync** | LTC/MTC | Sync to video timecode |

## 4. Technical Selection

### 4.1 Implementation Strategy

| Feature Group | Approach | Dependencies |
|---------------|----------|--------------|
| Audio Processing | Use Tracktion Engine's built-in + custom processors | TE already has most |
| Surround | Extend TE's panner system | TE supports multi-channel |
| Trigger/Clip | Already have Session View from magda-core, extend it | Existing code |
| Export | Build on TE's AudioFileWriter + custom pipeline | JUCE audio formats |
| MIDI | Extend existing MidiNoteCommands + TE MIDI | Existing code |
| Routing | TE already supports sends/returns/groups | TE API |
| Locations | New module: `src/core/LocationManager` | Simple data model |
| Control Surfaces | New module: `src/control/` | OSC lib + MIDI |
| Video | FFmpeg integration | External FFmpeg |

### 4.2 Priority Phases

**Phase 1 (Next sprint):**
- Export system (multi-format, stems, presets)
- VCA faders + route groups
- Locations/markers system
- Strip silence + normalize + reverse

**Phase 2:**
- Trigger box (enhance session view)
- Clip library browser
- Advanced MIDI quantize with groove
- Loudness metering (LUFS)

**Phase 3:**
- Surround/spatial audio
- Convolution reverb
- Time stretch / pitch shift improvements
- Control surface protocols

**Phase 4:**
- Video timeline
- Dolby Atmos
- Beat box
- Advanced DSP filters

## 5. Success Criteria

- All Phase 1 features compile and function
- Export produces valid audio files in WAV/FLAC/MP3
- VCA faders correctly control linked tracks
- Markers visible on timeline and navigable
- Zero regressions in existing magda-core functionality
