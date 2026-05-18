# AIDAW

A dual-mode Digital Audio Workstation with AI-powered composition.

## Modes

- **Professional Mode** — Full DAW with multi-track recording, mixing, plugin hosting
- **AI Minimal Mode** — Voice/text-driven AI composition with minimal UI

## Tech Stack

- C++20
- JUCE (UI framework)
- Tracktion Engine (audio engine)
- llama.cpp (local AI inference)
- Cloud API support (Claude, GPT, custom relay)

## Build

### Prerequisites

- CMake 3.24+
- Ninja (recommended)
- C++20 compiler (MSVC 2022, GCC 12+, Clang 15+)

### Clone dependencies

```bash
git submodule add https://github.com/juce-framework/JUCE.git third_party/juce
git submodule add https://github.com/Tracktion/tracktion_engine.git third_party/tracktion_engine
git submodule add https://github.com/ggerganov/llama.cpp.git third_party/llama.cpp
```

### Build (Windows)

```bat
build.bat
```

### Build (Linux/Mac)

```bash
chmod +x setup.sh
./setup.sh
```

## Project Structure

```
src/
├── app/       — Application entry point
├── core/      — DAW engine (tracks, clips, mixer, transport)
├── modes/     — Dual-mode system (Professional / AI Minimal)
├── ai/        — AI composition engine (LLM clients, music agent)
├── ui/        — JUCE UI (pro views + AI chat interface)
└── utils/     — Utilities (logger, JSON, strings)
```

## AI Backends

| Backend | Config Key | Description |
|---------|-----------|-------------|
| Local | `modelPath` | llama.cpp with GGUF models |
| Cloud | `cloudApiUrl` + `cloudApiKey` | OpenAI/Anthropic compatible |
| Relay | `relayUrl` | Custom proxy/relay server |
