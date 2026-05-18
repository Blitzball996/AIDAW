---
name: ai-composition
description: AI music composition system architecture. Use when working on the AI engine, music agents, LLM integration, or the instruction execution pipeline.
---

# AI Composition System

## Architecture

```
User Input (voice/text)
    │
    ▼
VoiceInput (speech-to-text)
    │
    ▼
PromptParser (direct commands or LLM prompt building)
    │
    ├── Direct commands ("tempo 120") → MusicInstruction immediately
    │
    └── Complex requests → MusicAgent → LLM → parse output → MusicInstruction[]
                                │
                                ▼
                    InstructionExecutor → DAW operations (TrackManager, ClipManager, etc.)
```

## Music Instruction Format

The LLM outputs instructions in a line-based format:

```
TEMPO <bpm>
KEY <key> <scale>
TIME <numerator>/<denominator>
TRACK <name> <instrument>
CHORD <root> <quality> <bar> <beat> <duration>
NOTE <pitch> <bar> <beat> <duration> <velocity>
DRUM <pattern_name> <bar> <duration>
ARP <root> <quality> <pattern> <bar> <beat> <duration>
```

## LLM Backend Selection

Three backends available via `LlmClientFactory`:

| Backend | Class | When to Use |
|---------|-------|-------------|
| Local | `LocalLlmClient` | Offline, privacy, GPU available |
| Cloud | `CloudLlmClient` | Best quality, needs internet |
| Relay | `RelayLlmClient` | Custom proxy, rate limiting, caching |

## Key Files

- `src/ai/AiEngine.hpp` — Top-level AI coordinator
- `src/ai/MusicAgent.hpp` — LLM-powered music generation
- `src/ai/MusicInstruction.hpp` — Instruction IR (intermediate representation)
- `src/ai/InstructionExecutor.hpp` — Converts instructions to DAW operations
- `src/ai/llm/LlmClient.hpp` — Abstract LLM interface
- `src/ai/llm/LlmClientFactory.hpp` — Backend selection

## Adding New Instruction Types

1. Add enum value to `InstructionType` in `MusicInstruction.hpp`
2. Add parsing case in `MusicAgent::parseOutput()`
3. Add handler in `InstructionExecutor`
4. Update the system prompt in `MusicAgent::getSystemPrompt()`

## Thread Safety

- LLM calls are blocking — always run on a background thread
- Use `MusicAgent::requestCancel()` to abort long-running inference
- `InstructionExecutor` modifies DAW state — call from message thread only
