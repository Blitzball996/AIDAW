#include "music_agent.hpp"

#include "../daw/core/Config.hpp"
#include "llm_client_factory.hpp"
#include "llm_presets.hpp"
#include "music_memory.hpp"
#include "harmony_theory.hpp"

namespace magda {

// ============================================================================
// System prompts
// ============================================================================

const char* MusicAgent::getCompactSystemPrompt() {
    return R"PROMPT(You are a music theory assistant. Generate musical content using compact notation.
Respond ONLY with instructions. No prose. No markdown. One instruction per line.

INSTRUCTIONS:
  CHORD <root> <quality> <beat> <length> [velocity]  - Add a chord
  NOTE <pitch> <beat> <length> [velocity]             - Add a single note
  ARP <root> <quality> <beat> <step> [beats]          - Add an arpeggio

ROOTS: C3, C#4, Db4, D4, Eb4, E4, F4, F#4, G4, Ab4, A4, Bb4, B4 (octave 3-5)
QUALITIES: major, min, dim, aug, sus2, sus4, dom7, maj7, min7, dim7, dom9, min9, maj9
BEAT: position in beats (0 = start, 4 = beat 5)
LENGTH: duration in beats
VELOCITY: 1-127 (optional, default 100)

EXAMPLES:
"C major chord progression" ->
CHORD C4 major 0 4
CHORD F4 major 4 4
CHORD G4 major 8 4
CHORD C4 major 12 4

"jazzy ii-V-I in Bb" ->
CHORD C4 min7 0 4
CHORD F4 dom7 4 4
CHORD Bb3 maj7 8 4

"blues in G" ->
CHORD G3 dom7 0 4
CHORD G3 dom7 4 4
CHORD C4 dom7 8 4
CHORD G3 dom7 12 4
CHORD D4 dom7 16 4
CHORD C4 dom7 20 4
CHORD G3 dom7 24 4
CHORD G3 dom7 28 4

"arpeggiate Am" ->
ARP A3 min 0 0.5

"bass line in E minor" ->
NOTE E2 0 1
NOTE G2 1 0.5
NOTE A2 1.5 0.5
NOTE B2 2 1
NOTE E2 3 1)PROMPT";
}

const char* MusicAgent::getDSLSystemPrompt() {
    return R"PROMPT(You are a music theory assistant. Generate musical content using DSL notation.
Your output must start with a DESCRIPTION line, then DSL note operations. No other prose.

FORMAT:
DESCRIPTION: <one sentence describing what you generated and why>
<DSL note operations, one per line>

NOTE OPERATIONS:
- notes.add(pitch=C4, beat=0, length=1, velocity=100) - Add a single note
- notes.add_chord(root=C4, quality=major, beat=0, length=1, velocity=100, inversion=0) - Add a chord
- notes.add_arpeggio(root=C4, quality=major, beat=0, step=0.5, beats=8, pattern=up) - Add an arpeggio
- notes.repeat(from_beat=0, to_beat=16, paste_at=16, times=3) - Copy a beat range and paste it N times consecutively

MULTI-TRACK:
When generating content for multiple tracks, use [track: Name] headers to separate sections:
[track: Piano]
notes.add_chord(root=C4, quality=major, beat=0, length=4)
[track: Bass]
notes.add(pitch=C2, beat=0, length=2)

PITCH: C3, C#4, Db4, D4, Eb4, E4, F4, F#4, G4, Ab4, A4, Bb4, B4 (octave 2-6)
QUALITIES: major, min, dim, aug, sus2, sus4, dom7, maj7, min7, dim7, dom9, min9, maj9, 6, min6, 7b5, 7sharp5, half_dim, power
BEAT: position in beats (0 = start, 4 = beat 5). At 4/4 time, 1 bar = 4 beats.
LENGTH: duration in beats
VELOCITY: 1-127 (default 100)
INVERSION: 0=root, 1=first, 2=second (default 0)
PATTERN: up, down, updown (default up)

LENGTH COMPLIANCE (CRITICAL):
- If the user requests N bars, you MUST generate content covering N*4 beats (at 4/4).
- For long pieces (>8 bars), use notes.repeat() to efficiently fill the requested length.
- Example: User asks for 32 bars. Generate 8 bars of unique content, then repeat(from_beat=0, to_beat=32, paste_at=32, times=3) to fill 32 bars.
- NEVER generate less content than requested. If asked for 100 bars, generate patterns and repeat them to fill 400 beats.

EXAMPLES:
"C major chord progression" ->
DESCRIPTION: Classic I-IV-V-I progression in C major, 4 beats per chord
notes.add_chord(root=C4, quality=major, beat=0, length=4)
notes.add_chord(root=F4, quality=major, beat=4, length=4)
notes.add_chord(root=G4, quality=major, beat=8, length=4)
notes.add_chord(root=C4, quality=major, beat=12, length=4)

"32 bars of pop chords in G major" ->
DESCRIPTION: 32-bar pop progression in G major (I-V-vi-IV pattern repeated 8 times)
notes.add_chord(root=G4, quality=major, beat=0, length=4)
notes.add_chord(root=D4, quality=major, beat=4, length=4)
notes.add_chord(root=E4, quality=min, beat=8, length=4)
notes.add_chord(root=C4, quality=major, beat=12, length=4)
notes.repeat(from_beat=0, to_beat=16, paste_at=16, times=7)

"create piano and bass tracks with jazz chords" ->
DESCRIPTION: Jazz ii-V-I in C major on piano with walking bass
[track: Piano]
notes.add_chord(root=D4, quality=min7, beat=0, length=4)
notes.add_chord(root=G4, quality=dom7, beat=4, length=4)
notes.add_chord(root=C4, quality=maj7, beat=8, length=8)
[track: Bass]
notes.add(pitch=D2, beat=0, length=1)
notes.add(pitch=F2, beat=1, length=1)
notes.add(pitch=A2, beat=2, length=1)
notes.add(pitch=C3, beat=3, length=1)
notes.add(pitch=G2, beat=4, length=1)
notes.add(pitch=B2, beat=5, length=1)
notes.add(pitch=D3, beat=6, length=1)
notes.add(pitch=F2, beat=7, length=1)
notes.add(pitch=C2, beat=8, length=2)
notes.add(pitch=E2, beat=10, length=2)
notes.add(pitch=G2, beat=12, length=2)
notes.add(pitch=C3, beat=14, length=2)

CRITICAL: Always start with DESCRIPTION. Then only DSL operations. No other text.
CRITICAL: Obey the user's requested length exactly. Use notes.repeat() for efficiency.
CRITICAL: ALWAYS use [track: Name] headers, even for single-track output. Never output notes without a preceding [track: X] header.
CRITICAL: Do NOT wrap output in markdown code fences. Output raw DSL only.

FULL SONG GENERATION:
When asked to generate a full song/project, you MUST:
1. Use [track: X] headers for EVERY track (e.g. [track: Piano], [track: Bass], [track: Drums])
2. Generate notes for ALL tracks — no empty tracks
3. Use the appropriate structure template for the genre
4. Generate unique content for each section (don't just repeat the same 4 bars)
5. Use notes.repeat() to extend sections that should loop (e.g. a 4-bar chord pattern repeated 4x = 16 bars)
6. Vary velocity and rhythm between sections for dynamics

Song structure templates by genre:
- Pop/R&B: Intro(4-8 bars) → Verse(8-16) → Pre-Chorus(4-8) → Chorus(8-16) → Verse2 → Chorus → Bridge(8) → Chorus → Outro(4-8)
- EDM/Electronic: Intro(8-16) → Build(8) → Drop(16-32) → Break(8) → Build(8) → Drop(16-32) → Outro(8-16)
- Hip-Hop: Intro(4-8) → Verse(16) → Hook(8) → Verse2(16) → Hook(8) → Bridge(8) → Hook(8) → Outro(4)
- Rock: Intro(4-8) → Verse(8-16) → Chorus(8-16) → Verse2 → Chorus → Solo(8-16) → Chorus → Outro(8)
- Jazz: Head(8-16) → Solo sections(16-32 each) → Head out(8-16)

MINIMUM SONG EXAMPLE (pop, 16 bars):
DESCRIPTION: 16-bar pop song with piano chords and bass
[track: Piano]
notes.add_chord(root=C4, quality=major, beat=0, length=4)
notes.add_chord(root=G4, quality=major, beat=4, length=4)
notes.add_chord(root=A4, quality=min, beat=8, length=4)
notes.add_chord(root=F4, quality=major, beat=12, length=4)
notes.repeat(from_beat=0, to_beat=16, paste_at=16, times=3)
[track: Bass]
notes.add(pitch=C2, beat=0, length=2)
notes.add(pitch=C2, beat=2, length=2)
notes.add(pitch=G2, beat=4, length=2)
notes.add(pitch=G2, beat=6, length=2)
notes.add(pitch=A2, beat=8, length=2)
notes.add(pitch=A2, beat=10, length=2)
notes.add(pitch=F2, beat=12, length=2)
notes.add(pitch=F2, beat=14, length=2)
notes.repeat(from_beat=0, to_beat=16, paste_at=16, times=3))PROMPT";
}

// ============================================================================
// DSL parser — converts DSL note operations into IR instructions
// ============================================================================

namespace {

/** Wrap a project snapshot in the instructions that turn "compose something"
    into "revise what is already here".

    Without this the agent only ever sees the user's sentence, so every request
    reads as a blank-page brief and it rewrites the arrangement from scratch.
    Returns an empty string when there is no project yet, which keeps the
    from-nothing behaviour for a new session. */
juce::String buildEditingContext(const std::string& projectContext) {
    if (projectContext.empty())
        return {};

    return juce::String::fromUTF8(projectContext.c_str())
           + R"PROMPT(
WORKING WITH THE EXISTING PROJECT (CRITICAL):
- The arrangement above already exists. Treat the user's message as an edit
  request against it, not as a brief for a new piece.
- Only emit operations for what actually changes. Leave every other track and
  every other bar alone - notes you do not mention are kept.
- Match what is already there: stay in the established key and tempo, and keep
  new parts rhythmically consistent with the existing ones unless asked not to.
- Target the track the user means with [track: Name], using the names above
  exactly. A name that does not appear above creates a new track.
- When asked to change something ("make the bass busier", "swap the 3rd chord
  for a minor"), write the replacement notes for that region only.
- Only compose a whole arrangement from scratch when the project is empty or
  the user explicitly asks for a new song.
)PROMPT";
}

/** Extract key=value pairs from a parameter string like "root=C4, quality=major, beat=0". */
juce::StringPairArray parseParams(const juce::String& paramStr) {
    juce::StringPairArray params;
    auto pairs = juce::StringArray::fromTokens(paramStr, ",", "\"");
    for (auto& pair : pairs) {
        auto eqPos = pair.indexOf("=");
        if (eqPos > 0) {
            auto key = pair.substring(0, eqPos).trim();
            auto val = pair.substring(eqPos + 1).trim();
            params.set(key, val);
        }
    }
    return params;
}

}  // namespace

std::vector<Instruction> MusicAgent::parseDSL(const juce::String& text,
                                              std::string& outDescription) {
    std::vector<Instruction> instructions;
    auto lines = juce::StringArray::fromLines(text);

    for (auto& line : lines) {
        auto trimmed = line.trim();
        if (trimmed.isEmpty())
            continue;

        // Skip markdown code fences (```  or ```dsl etc.)
        if (trimmed.startsWith("```"))
            continue;

        // Extract description
        if (trimmed.startsWith("DESCRIPTION:")) {
            outDescription = trimmed.substring(12).trim().toStdString();
            continue;
        }

        // Skip comments
        if (trimmed.startsWith("//") || trimmed.startsWith("#"))
            continue;

        // Robustness: after several chat turns local models sometimes wrap a
        // valid DSL call in stray prose (e.g. "Here: notes.add_chord(...) done").
        // Strip any leading text before a known DSL token and any trailing text
        // after the final ')' so the strict checks below still match instead of
        // silently dropping the line (which left clips empty).
        {
            static const char* kTokens[] = {
                "notes.add_chord(", "notes.add_arpeggio(", "notes.repeat(",
                "notes.add("};
            int best = -1;
            for (const char* tok : kTokens) {
                int idx = trimmed.indexOf(tok);
                if (idx >= 0 && (best < 0 || idx < best))
                    best = idx;
            }
            if (best > 0)
                trimmed = trimmed.substring(best);
            if (best >= 0) {
                int lastParen = trimmed.lastIndexOf(")");
                if (lastParen >= 0 && lastParen < trimmed.length() - 1)
                    trimmed = trimmed.substring(0, lastParen + 1);
            }
        }

        // Parse [track: Name] section header
        if (trimmed.startsWith("[track:") && trimmed.endsWith("]")) {
            auto name = trimmed.substring(7, trimmed.length() - 1).trim();
            TrackSwitchOp op;
            op.name = name;
            instructions.push_back({OpCode::TrackSwitch, std::move(op)});
            continue;
        }

        // Parse notes.repeat(from_beat=0, to_beat=16, paste_at=16, times=3)
        if (trimmed.startsWith("notes.repeat(") && trimmed.endsWith(")")) {
            auto paramStr = trimmed.substring(13, trimmed.length() - 1);
            auto params = parseParams(paramStr);

            RepeatOp op;
            op.fromBeat = params.getValue("from_beat", "0").getDoubleValue();
            op.toBeat = params.getValue("to_beat", "0").getDoubleValue();
            op.pasteAt = params.getValue("paste_at", "0").getDoubleValue();
            op.times = params.getValue("times", "1").getIntValue();

            instructions.push_back({OpCode::Repeat, std::move(op)});
            continue;
        }

        // Parse notes.add_chord(...)
        if (trimmed.startsWith("notes.add_chord(") && trimmed.endsWith(")")) {
            auto paramStr = trimmed.substring(16, trimmed.length() - 1);
            auto params = parseParams(paramStr);

            ChordOp op;
            op.root = params.getValue("root", "C4");
            op.quality = params.getValue("quality", "major");
            op.beat = params.getValue("beat", "0").getDoubleValue();
            op.length = params.getValue("length", "1").getDoubleValue();
            auto vel = params.getValue("velocity", "");
            if (vel.isNotEmpty())
                op.velocity = vel.getIntValue();
            auto inv = params.getValue("inversion", "");
            if (inv.isNotEmpty())
                op.inversion = inv.getIntValue();

            instructions.push_back({OpCode::Chord, std::move(op)});
            continue;
        }

        // Parse notes.add_arpeggio(...)
        if (trimmed.startsWith("notes.add_arpeggio(") && trimmed.endsWith(")")) {
            auto paramStr = trimmed.substring(19, trimmed.length() - 1);
            auto params = parseParams(paramStr);

            ArpOp op;
            op.root = params.getValue("root", "C4");
            op.quality = params.getValue("quality", "major");
            op.beat = params.getValue("beat", "0").getDoubleValue();
            op.step = params.getValue("step", "0.5").getDoubleValue();
            auto beats = params.getValue("beats", "");
            if (beats.isNotEmpty())
                op.beats = beats.getDoubleValue();
            auto inv = params.getValue("inversion", "");
            if (inv.isNotEmpty())
                op.inversion = inv.getIntValue();
            op.pattern = params.getValue("pattern", "");

            instructions.push_back({OpCode::Arp, std::move(op)});
            continue;
        }

        // Parse notes.add(...)
        if (trimmed.startsWith("notes.add(") && trimmed.endsWith(")")) {
            auto paramStr = trimmed.substring(10, trimmed.length() - 1);
            auto params = parseParams(paramStr);

            NoteOp op;
            op.pitch = params.getValue("pitch", "C4");
            op.beat = params.getValue("beat", "0").getDoubleValue();
            op.length = params.getValue("length", "1").getDoubleValue();
            auto vel = params.getValue("velocity", "");
            if (vel.isNotEmpty())
                op.velocity = vel.getIntValue();

            instructions.push_back({OpCode::Note, std::move(op)});
            continue;
        }
    }

    return instructions;
}

namespace {

/** Log the resolved LLM config before a request — makes provider/endpoint/model
    mismatches obvious in the console. Does NOT log the API key. */
void logMusicAgentConfig(const Config::AgentLLMConfig& agentConfig, const llm::ProviderConfig& pc,
                         bool useCompact) {
    DBG("MAGDA MusicAgent config:");
    DBG("  provider (string) = " + juce::String(agentConfig.provider));
    DBG("  provider (enum)   = " + juce::String(static_cast<int>(pc.provider)) +
        " (0=OpenAIChat, 1=OpenAIResponses, 2=Anthropic, 3=Gemini)");
    DBG("  model             = " + pc.model);
    DBG("  baseUrl           = " + pc.baseUrl);
    DBG("  apiKey present    = " + juce::String(pc.apiKey.isNotEmpty() ? "yes" : "NO"));
    DBG("  noTemperature     = " + juce::String(pc.noTemperature ? "yes" : "no"));
    DBG("  reasoningEffort   = " + pc.reasoningEffort);
    DBG("  format            = " + juce::String(useCompact ? "compact" : "DSL"));
}

void logMusicAgentResult(const std::string& rawOutput, const std::vector<Instruction>& instructions,
                         const std::string& description, const juce::String& error) {
    DBG("MAGDA MusicAgent raw output (" + juce::String(static_cast<int>(rawOutput.size())) +
        " chars):");
    DBG("---8<---");
    DBG(juce::String(rawOutput));
    DBG("--->8---");
    DBG("MAGDA MusicAgent parsed " + juce::String(static_cast<int>(instructions.size())) +
        " instruction(s)");
    if (!description.empty())
        DBG("MAGDA MusicAgent description: " + juce::String(description));
    if (error.isNotEmpty())
        DBG("MAGDA MusicAgent ERROR: " + error);
}

}  // namespace

MusicAgent::GenerateResult MusicAgent::generate(const std::string& message,
                                                const std::string& projectContext) {
    GenerateResult result;

    if (shouldStop_.load()) {
        result.error = "Cancelled";
        result.hasError = true;
        return result;
    }

    auto agentConfig = Config::getInstance().getAgentLLMConfig(role::MUSIC);
    bool useCompact = (agentConfig.provider == provider::LLAMA_LOCAL);

    auto providerConfig = toLLMProviderConfig(agentConfig, "music");
    logMusicAgentConfig(agentConfig, providerConfig, useCompact);

    if (!useCompact) {
        if (providerConfig.apiKey.isEmpty() && agentConfig.baseUrl.empty()) {
            result.error = "Music agent API key not configured.";
            result.hasError = true;
            DBG("MAGDA MusicAgent ABORT: no API key for provider " +
                juce::String(agentConfig.provider));
            return result;
        }
    }

    auto client = createLLMClient(agentConfig, "music");
    DBG("MAGDA MusicAgent client name: " + client->getName());

    llm::Request request;
    auto basePrompt =
        juce::String::fromUTF8(useCompact ? getCompactSystemPrompt() : getDSLSystemPrompt());
    auto theoryBrief = juce::String::fromUTF8(theory::constraintBrief());
    auto memoryContext = MusicMemory::getInstance().buildContextPrompt();
    request.systemPrompt = basePrompt + theoryBrief + juce::String(memoryContext)
                           + buildEditingContext(projectContext);
    // A full arrangement runs well past the 4096-token default and used to come
    // back truncated mid-line, which the parser then silently discarded -- the
    // visible symptom was "it only ever writes one section".
    request.maxTokens = 16000;
    request.userMessage = juce::String::fromUTF8(message.c_str());
    request.temperature = 0.2f;  // lower temp keeps the strict DSL format stable

    DBG("MAGDA MusicAgent sending request (user message: " + request.userMessage + ")");

    auto response = client->sendRequest(request);

    DBG("MAGDA MusicAgent response: success=" + juce::String(response.success ? "true" : "false") +
        " wall=" + juce::String(response.wallSeconds, 2) + "s");

    if (!response.success) {
        DBG("MAGDA MusicAgent HTTP/provider error: " + response.error);
        result.error = response.error.toStdString();
        result.hasError = true;
        return result;
    }

    auto trimmedText = response.text.trim();
    result.rawOutput = trimmedText.toStdString();
    result.truncated = response.truncated;

    if (useCompact) {
        result.instructions = parser_.parse(trimmedText);
        if (result.instructions.empty() && parser_.getLastError().isNotEmpty()) {
            result.error = "Parse error: " + parser_.getLastError().toStdString();
            result.hasError = true;
        }
    } else {
        result.instructions = parseDSL(trimmedText, result.description);
        if (result.instructions.empty()) {
            result.error = "DSL parse error: no valid note operations found";
            result.hasError = true;
        } else {
            bool hasNotes = false;
            for (const auto& inst : result.instructions) {
                if (inst.opcode == OpCode::Note || inst.opcode == OpCode::Chord ||
                    inst.opcode == OpCode::Arp || inst.opcode == OpCode::Repeat) {
                    hasNotes = true;
                    break;
                }
            }
            if (!hasNotes) {
                result.error = "DSL parse error: track headers found but no note operations";
                result.hasError = true;
                result.instructions.clear();
            }
        }
    }

    logMusicAgentResult(result.rawOutput, result.instructions, result.description,
                        juce::String(result.error));

    return result;
}

MusicAgent::GenerateResult MusicAgent::generateStreaming(const std::string& message,
                                                         TokenCallback onToken,
                                                         const std::string& projectContext) {
    GenerateResult result;

    if (shouldStop_.load()) {
        result.error = "Cancelled";
        result.hasError = true;
        return result;
    }

    auto agentConfig = Config::getInstance().getAgentLLMConfig(role::MUSIC);
    bool useCompact = (agentConfig.provider == provider::LLAMA_LOCAL);

    auto providerConfig = toLLMProviderConfig(agentConfig, "music");
    logMusicAgentConfig(agentConfig, providerConfig, useCompact);

    if (!useCompact) {
        if (providerConfig.apiKey.isEmpty() && agentConfig.baseUrl.empty()) {
            result.error = "Music agent API key not configured.";
            result.hasError = true;
            DBG("MAGDA MusicAgent stream ABORT: no API key for provider " +
                juce::String(agentConfig.provider));
            return result;
        }
    }

    auto client = createLLMClient(agentConfig, "music");
    DBG("MAGDA MusicAgent stream client name: " + client->getName());

    llm::Request request;
    auto basePrompt =
        juce::String::fromUTF8(useCompact ? getCompactSystemPrompt() : getDSLSystemPrompt());
    auto theoryBrief = juce::String::fromUTF8(theory::constraintBrief());
    auto memoryContext = MusicMemory::getInstance().buildContextPrompt();
    request.systemPrompt = basePrompt + theoryBrief + juce::String(memoryContext)
                           + buildEditingContext(projectContext);
    // A full arrangement runs well past the 4096-token default and used to come
    // back truncated mid-line, which the parser then silently discarded -- the
    // visible symptom was "it only ever writes one section".
    request.maxTokens = 16000;
    request.userMessage = juce::String::fromUTF8(message.c_str());
    request.temperature = 0.2f;

    DBG("MAGDA MusicAgent stream sending request (user message: " + request.userMessage + ")");

    auto response = client->sendStreamingRequest(request, [&](const juce::String& token) {
        if (shouldStop_.load())
            return false;
        if (onToken)
            return onToken(token);
        return true;
    });

    DBG("MAGDA MusicAgent stream response: success=" +
        juce::String(response.success ? "true" : "false") +
        " wall=" + juce::String(response.wallSeconds, 2) + "s");

    if (!response.success) {
        DBG("MAGDA MusicAgent stream HTTP/provider error: " + response.error);
        result.error = response.error.toStdString();
        result.hasError = true;
        return result;
    }

    auto trimmedText = response.text.trim();
    result.rawOutput = trimmedText.toStdString();
    result.truncated = response.truncated;

    if (useCompact) {
        result.instructions = parser_.parse(trimmedText);
        if (result.instructions.empty() && parser_.getLastError().isNotEmpty()) {
            result.error = "Parse error: " + parser_.getLastError().toStdString();
            result.hasError = true;
        }
    } else {
        result.instructions = parseDSL(trimmedText, result.description);
        if (result.instructions.empty()) {
            result.error = "DSL parse error: no valid note operations found";
            result.hasError = true;
        } else {
            bool hasNotes = false;
            for (const auto& inst : result.instructions) {
                if (inst.opcode == OpCode::Note || inst.opcode == OpCode::Chord ||
                    inst.opcode == OpCode::Arp || inst.opcode == OpCode::Repeat) {
                    hasNotes = true;
                    break;
                }
            }
            if (!hasNotes) {
                result.error = "DSL parse error: track headers found but no note operations";
                result.hasError = true;
                result.instructions.clear();
            }
        }
    }

    logMusicAgentResult(result.rawOutput, result.instructions, result.description,
                        juce::String(result.error));

    return result;
}

}  // namespace magda
