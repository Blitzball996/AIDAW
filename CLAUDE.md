# AIDAW — Project Instructions

## Overview

AIDAW is a dual-mode DAW (Digital Audio Workstation):
- **Professional Mode**: Full DAW with multi-track, mixing, plugin hosting
- **AI Minimal Mode**: Voice/text-driven AI composition

## Tech Stack

- C++20, CMake 3.24+
- JUCE (UI + audio utilities)
- Tracktion Engine (audio engine, plugin hosting)
- llama.cpp (local LLM inference)
- Cloud API (Claude/GPT/custom relay)

## Project Structure

```
src/app/     — Application entry, config
src/core/    — DAW engine (tracks, clips, mixer, transport, plugins)
src/modes/   — Dual-mode system (Professional ↔ AI Minimal)
src/ai/      — AI composition (MusicAgent, LLM clients, voice input)
src/ui/      — JUCE UI (pro/ and ai/ subdirectories)
src/utils/   — Logger, JSON, string utilities
tests/       — Unit tests
third_party/ — JUCE, Tracktion Engine, llama.cpp (submodules)
```

## Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Or on Windows: `build.bat`

## Rules

- Never allocate memory on the audio thread
- Never block the audio thread (no locks, no I/O)
- Use `std::atomic` for audio↔UI communication
- Pre-allocate buffers in `initialise()`, not in `applyToBuffer()`
- All LLM calls must run on background threads
- InstructionExecutor modifies DAW state — message thread only
- Keep files under 500 lines
- Namespace: `aidaw`

## Testing

```bash
cmake --build build --target aidaw_tests
ctest --test-dir build
```
