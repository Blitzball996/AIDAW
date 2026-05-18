# AIDAW AI Minimal Mode — Feature Specification

## Reference: magda-core AI Architecture

Based on analysis of magda-core's agent system:

### Agents in magda-core

| Agent | Purpose | Input → Output |
|-------|---------|----------------|
| `RouterAgent` | Classifies intent: COMMAND, MUSIC, or BOTH | text → intent |
| `DAWAgent` | Full DAW control via compact IR | text → track/clip operations |
| `MusicAgent` | Pure music generation (chords, notes, arps) | text → CHORD/NOTE/ARP |
| `AutomationAgent` | Automation curve generation | text → AUTO shapes |
| `SoundDesignAgent` | Per-device preset design | text → device parameters |
| `FaustAgent` | DSP code generation (Faust language) | text → Faust code |
| `CoderAgent` | General code generation for devices | text → code |
| `CommandAgent` | Direct DAW commands (no music) | text → DAW operations |
| `FourOscAgent` | 4-oscillator synth preset design | text → synth params |

### magda-core Instruction Format (Compact IR)

```
CHORD <root> <quality> <beat> <length> [velocity]
NOTE <pitch> <beat> <length> [velocity]
ARP <root> <quality> <beat> <step> [beats]
AUTO <shape> <start_beat> <end_beat> <start_val> <end_val>
```

### magda-core LLM Integration

- Uses `juce-llm` module for HTTP client abstraction
- Supports local (llama.cpp) and cloud (OpenAI-compatible) backends
- Router agent uses cheap/fast model for intent classification
- Music/DAW agents use capable model for generation
- Streaming supported with token callbacks
- Cancel via atomic flag

---

## AIDAW AI Minimal Mode — Feature Design

### P0: Must-Have (MVP)

**Text Input Composition:**
- Chat panel with message history
- Natural language → music instructions
- Direct commands: "tempo 120", "key C major"
- Complex requests: "write a jazz chord progression in Bb"
- Context-aware: knows current tracks, tempo, key

**Core AI Agents:**
- `RouterAgent` — classify intent (command vs music vs both)
- `MusicAgent` — generate chords, notes, arpeggios
- `CommandAgent` — DAW operations (create track, set tempo, delete clip)
- `InstructionExecutor` — convert IR to DAW operations

**Instruction Types:**
- CHORD — chord with root, quality, position, duration
- NOTE — single note with pitch, position, duration, velocity
- ARP — arpeggio pattern
- TEMPO — set BPM
- KEY — set key signature
- TRACK — create/delete tracks
- INSTRUMENT — assign instrument to track

**LLM Backends:**
- Local llama.cpp (offline, privacy)
- Cloud API (Claude/GPT, best quality)
- Custom relay (user's own proxy)
- Hot-switchable at runtime

**UI:**
- Minimal dark interface
- Chat panel (scrollable message history)
- Mode toggle button (switch to Pro mode)
- Current project status display (tempo, key, tracks)

### P1: Important (Phase 2)

**Voice Input:**
- Push-to-talk voice recording
- Speech-to-text (Whisper integration or platform API)
- Voice → text → AI pipeline
- Visual feedback during listening

**Advanced Composition:**
- Full arrangement generation ("write a 4-bar verse")
- Drum pattern generation
- Bass line generation
- Melody over chord progression
- Style/genre specification ("make it funky", "bossa nova feel")
- Variation generation ("make it more complex")

**Automation Agent:**
- Generate automation curves from text
- Shape-based: "fade in over 4 bars", "swell on the chorus"
- Sine, triangle, saw, exponential shapes

**Sound Design Agent:**
- "Design a warm pad sound"
- "Make a punchy bass"
- Per-instrument preset generation
- Parameter mapping from text descriptions

**Context Awareness:**
- Read current project state before generating
- Reference existing tracks ("add harmony to track 2")
- Understand musical context (key, scale, chord progression)
- Suggest next steps based on what exists

### P2: Nice-to-Have (Future)

**Advanced AI Features:**
- Multi-turn conversation with memory
- "Undo that" / "change the last chord to minor"
- Style transfer ("make this sound like Radiohead")
- Reference track analysis ("compose something like this audio file")
- Lyrics-to-melody generation
- Collaborative AI (AI suggests, user approves/modifies)

**Faust/DSP Agent:**
- Generate custom DSP effects from description
- "Create a phaser with slow rate"
- Compile and load Faust code in real-time

**Learning:**
- Learn user preferences over time
- Adapt to user's musical style
- Remember past sessions and preferences

**Integration:**
- Seamless switch between AI and Pro mode
- AI-generated content editable in Pro mode
- AI can explain what it did and why
- Export AI session as MIDI/audio

---

## AI Pipeline Architecture

```
┌─────────────────────────────────────────────────────┐
│                   User Input                         │
│            (text or voice)                           │
└──────────────────────┬──────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────┐
│              VoiceInput (STT)                        │
│         (if voice, convert to text)                  │
└──────────────────────┬──────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────┐
│             RouterAgent (fast model)                 │
│     Classify: COMMAND | MUSIC | BOTH                 │
└────────┬─────────────┬──────────────┬───────────────┘
         │             │              │
    COMMAND         MUSIC           BOTH
         │             │              │
         ▼             ▼              ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────────┐
│ CommandAgent │ │ MusicAgent   │ │ DAWAgent         │
│ (DAW ops)    │ │ (notes/chords│ │ (combined)       │
└──────┬───────┘ └──────┬───────┘ └────────┬─────────┘
       │                │                   │
       └────────────────┴───────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────┐
│           InstructionExecutor                        │
│     (IR → TrackManager/ClipManager/Transport)        │
└─────────────────────────────────────────────────────┘
```

## System Prompt Strategy

Based on magda-core's approach:
- **Compact format** for local models (fewer tokens, faster)
- **DSL format** for frontier models (more expressive, descriptions)
- **Router prompt** kept minimal for speed (classify only)
- **Music prompt** includes examples for few-shot learning
- **Context injection** prepends current project state to user message
