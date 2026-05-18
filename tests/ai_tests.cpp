#include "ai/MusicInstruction.hpp"
#include "ai/MusicAgent.hpp"
#include "ai/RouterAgent.hpp"
#include "ai/PromptParser.hpp"
#include "ai/llm/LlmClient.hpp"
#include <cassert>
#include <iostream>
#include <string>

using namespace aidaw;

// --- Mock LLM Client for testing ---

class MockLlmClient : public LlmClient {
public:
    std::string responseContent;
    bool available = true;
    bool shouldSucceed = true;
    std::string errorMsg = "mock error";

    std::string getName() const override { return "MockLLM"; }
    bool isAvailable() const override { return available; }

    LlmResponse send(const LlmRequest& /*request*/) override {
        LlmResponse resp;
        resp.success = shouldSucceed;
        resp.content = responseContent;
        if (!shouldSucceed) resp.error = errorMsg;
        return resp;
    }

    LlmResponse sendStreaming(const LlmRequest& /*request*/, TokenCallback onToken) override {
        LlmResponse resp;
        resp.success = shouldSucceed;
        resp.content = responseContent;
        if (shouldSucceed && onToken) onToken(responseContent);
        if (!shouldSucceed) resp.error = errorMsg;
        return resp;
    }
};

// --- PromptParser Tests ---

void testPromptParser() {
    PromptParser parser;

    auto instructions = parser.parseDirectCommands("tempo 120");
    assert(instructions.size() == 1);
    assert(instructions[0].type == InstructionType::SetTempo);
    assert(instructions[0].params[0] == "120");

    auto keyInstr = parser.parseDirectCommands("key C major");
    assert(keyInstr.size() == 1);
    assert(keyInstr[0].type == InstructionType::SetKey);

    std::cout << "testPromptParser passed\n";
}

void testPromptParserBpmAlias() {
    PromptParser parser;

    // "bpm" should also be recognized as a tempo command
    auto instructions = parser.parseDirectCommands("bpm 90");
    assert(instructions.size() == 1);
    assert(instructions[0].type == InstructionType::SetTempo);
    assert(instructions[0].params[0] == "90");

    std::cout << "testPromptParserBpmAlias passed\n";
}

void testPromptParserKeyParams() {
    PromptParser parser;

    auto instructions = parser.parseDirectCommands("key D minor");
    assert(instructions.size() == 1);
    assert(instructions[0].type == InstructionType::SetKey);
    assert(instructions[0].params.size() == 2);
    assert(instructions[0].params[0] == "D");
    assert(instructions[0].params[1] == "minor");

    std::cout << "testPromptParserKeyParams passed\n";
}

void testPromptParserUnknownCommand() {
    PromptParser parser;

    // Unknown commands should return empty
    auto instructions = parser.parseDirectCommands("foobar 42");
    assert(instructions.empty());

    auto instructions2 = parser.parseDirectCommands("play");
    assert(instructions2.empty());

    auto instructions3 = parser.parseDirectCommands("");
    assert(instructions3.empty());

    std::cout << "testPromptParserUnknownCommand passed\n";
}

void testBuildMusicPromptNoContext() {
    PromptParser parser;

    std::string result = parser.buildMusicPrompt("make a jazz chord progression");
    assert(result == "make a jazz chord progression");

    std::cout << "testBuildMusicPromptNoContext passed\n";
}

void testBuildMusicPromptWithContext() {
    PromptParser parser;

    std::string result = parser.buildMusicPrompt("add bass", "tempo=120, key=C major");
    assert(result.find("tempo=120, key=C major") != std::string::npos);
    assert(result.find("add bass") != std::string::npos);
    // Context should appear before user input
    assert(result.find("tempo=120") < result.find("add bass"));

    std::cout << "testBuildMusicPromptWithContext passed\n";
}

void testBuildMusicPromptEmptyContext() {
    PromptParser parser;

    // Empty context string should behave like no context
    std::string result = parser.buildMusicPrompt("compose melody", "");
    assert(result == "compose melody");

    std::cout << "testBuildMusicPromptEmptyContext passed\n";
}

// --- MusicInstruction Tests ---

void testMusicInstruction() {
    MusicInstruction instr;
    instr.type = InstructionType::AddChord;
    instr.params = {"C", "maj"};
    instr.bar = 1;
    instr.beat = 1;
    instr.duration = 4.0f;

    assert(instr.params.size() == 2);
    assert(instr.bar == 1);
    assert(instr.duration == 4.0f);

    std::cout << "testMusicInstruction passed\n";
}

void testMusicInstructionDefaults() {
    MusicInstruction instr;
    instr.type = InstructionType::AddNote;

    // Verify defaults
    assert(instr.bar == 0);
    assert(instr.beat == 0);
    assert(instr.duration == 1.0f);
    assert(instr.velocity == 100);
    assert(instr.params.empty());
    assert(instr.target.empty());

    std::cout << "testMusicInstructionDefaults passed\n";
}

// --- RouterAgent Tests ---

void testRouterClassifyCommand() {
    MockLlmClient mock;
    mock.responseContent = "COMMAND";
    RouterAgent router(&mock);

    auto result = router.classify("stop playback");
    assert(!result.hasError);
    assert(result.intent == RouterAgent::Intent::Command);
    assert(result.intentString == "COMMAND");

    std::cout << "testRouterClassifyCommand passed\n";
}

void testRouterClassifyMusic() {
    MockLlmClient mock;
    mock.responseContent = "MUSIC";
    RouterAgent router(&mock);

    auto result = router.classify("compose a jazz ballad");
    assert(!result.hasError);
    assert(result.intent == RouterAgent::Intent::Music);
    assert(result.intentString == "MUSIC");

    std::cout << "testRouterClassifyMusic passed\n";
}

void testRouterClassifyBoth() {
    MockLlmClient mock;
    mock.responseContent = "BOTH";
    RouterAgent router(&mock);

    auto result = router.classify("set tempo to 120 and add a drum beat");
    assert(!result.hasError);
    assert(result.intent == RouterAgent::Intent::Both);
    assert(result.intentString == "BOTH");

    std::cout << "testRouterClassifyBoth passed\n";
}

void testRouterClassifyDefaultsToMusic() {
    MockLlmClient mock;
    // Unrecognized response should default to Music
    mock.responseContent = "something unexpected";
    RouterAgent router(&mock);

    auto result = router.classify("hello");
    assert(!result.hasError);
    assert(result.intent == RouterAgent::Intent::Music);

    std::cout << "testRouterClassifyDefaultsToMusic passed\n";
}

void testRouterClassifyWithWhitespace() {
    MockLlmClient mock;
    // Response with extra whitespace/newlines
    mock.responseContent = "  COMMAND  \n";
    RouterAgent router(&mock);

    auto result = router.classify("mute track 1");
    assert(!result.hasError);
    assert(result.intent == RouterAgent::Intent::Command);

    std::cout << "testRouterClassifyWithWhitespace passed\n";
}

void testRouterClassifyLlmUnavailable() {
    MockLlmClient mock;
    mock.available = false;
    RouterAgent router(&mock);

    auto result = router.classify("anything");
    assert(result.hasError);
    assert(!result.error.empty());

    std::cout << "testRouterClassifyLlmUnavailable passed\n";
}

void testRouterClassifyLlmError() {
    MockLlmClient mock;
    mock.shouldSucceed = false;
    mock.errorMsg = "network timeout";
    RouterAgent router(&mock);

    auto result = router.classify("play");
    assert(result.hasError);
    assert(result.error == "network timeout");

    std::cout << "testRouterClassifyLlmError passed\n";
}

void testRouterClassifyNullClient() {
    RouterAgent router(nullptr);

    auto result = router.classify("test");
    assert(result.hasError);

    std::cout << "testRouterClassifyNullClient passed\n";
}

void testRouterClassifyMeasuresTime() {
    MockLlmClient mock;
    mock.responseContent = "MUSIC";
    RouterAgent router(&mock);

    auto result = router.classify("compose something");
    assert(!result.hasError);
    assert(result.wallSeconds >= 0.0);

    std::cout << "testRouterClassifyMeasuresTime passed\n";
}

// --- MusicAgent Tests ---

void testMusicAgentParseChord() {
    MockLlmClient mock;
    mock.responseContent = "CHORD C maj 1 1 4\nCHORD F min 2 1 2";
    MusicAgent agent(&mock);

    auto result = agent.generate("add chords");
    assert(!result.hasError);
    assert(result.instructions.size() == 2);

    assert(result.instructions[0].type == InstructionType::AddChord);
    assert(result.instructions[0].params[0] == "C");
    assert(result.instructions[0].params[1] == "maj");
    assert(result.instructions[0].bar == 1);
    assert(result.instructions[0].beat == 1);
    assert(result.instructions[0].duration == 4.0f);

    assert(result.instructions[1].type == InstructionType::AddChord);
    assert(result.instructions[1].params[0] == "F");
    assert(result.instructions[1].params[1] == "min");
    assert(result.instructions[1].bar == 2);
    assert(result.instructions[1].beat == 1);
    assert(result.instructions[1].duration == 2.0f);

    std::cout << "testMusicAgentParseChord passed\n";
}

void testMusicAgentParseNote() {
    MockLlmClient mock;
    mock.responseContent = "NOTE C4 1 1 2 80";
    MusicAgent agent(&mock);

    auto result = agent.generate("add a note");
    assert(!result.hasError);
    assert(result.instructions.size() == 1);
    assert(result.instructions[0].type == InstructionType::AddNote);
    assert(result.instructions[0].params[0] == "C4");
    assert(result.instructions[0].bar == 1);
    assert(result.instructions[0].beat == 1);
    assert(result.instructions[0].duration == 2.0f);
    assert(result.instructions[0].velocity == 80);

    std::cout << "testMusicAgentParseNote passed\n";
}

void testMusicAgentParseTempo() {
    MockLlmClient mock;
    mock.responseContent = "TEMPO 140";
    MusicAgent agent(&mock);

    auto result = agent.generate("set tempo");
    assert(!result.hasError);
    assert(result.instructions.size() == 1);
    assert(result.instructions[0].type == InstructionType::SetTempo);
    assert(result.instructions[0].params[0] == "140");

    std::cout << "testMusicAgentParseTempo passed\n";
}

void testMusicAgentParseKey() {
    MockLlmClient mock;
    mock.responseContent = "KEY G minor";
    MusicAgent agent(&mock);

    auto result = agent.generate("set key");
    assert(!result.hasError);
    assert(result.instructions.size() == 1);
    assert(result.instructions[0].type == InstructionType::SetKey);
    assert(result.instructions[0].params[0] == "G");
    assert(result.instructions[0].params[1] == "minor");

    std::cout << "testMusicAgentParseKey passed\n";
}

void testMusicAgentParseTrack() {
    MockLlmClient mock;
    mock.responseContent = "TRACK drums DrumKit";
    MusicAgent agent(&mock);

    auto result = agent.generate("create drum track");
    assert(!result.hasError);
    assert(result.instructions.size() == 1);
    assert(result.instructions[0].type == InstructionType::CreateTrack);
    assert(result.instructions[0].target == "drums");
    assert(result.instructions[0].params[0] == "DrumKit");

    std::cout << "testMusicAgentParseTrack passed\n";
}

void testMusicAgentParseMultipleLines() {
    MockLlmClient mock;
    mock.responseContent = "TEMPO 120\nKEY C major\nTRACK piano Piano\nCHORD C maj 1 1 4";
    MusicAgent agent(&mock);

    auto result = agent.generate("compose something");
    assert(!result.hasError);
    assert(result.instructions.size() == 4);
    assert(result.instructions[0].type == InstructionType::SetTempo);
    assert(result.instructions[1].type == InstructionType::SetKey);
    assert(result.instructions[2].type == InstructionType::CreateTrack);
    assert(result.instructions[3].type == InstructionType::AddChord);

    std::cout << "testMusicAgentParseMultipleLines passed\n";
}

void testMusicAgentParseMalformedSkipsUnknown() {
    MockLlmClient mock;
    // Unknown commands and garbage lines should be skipped
    mock.responseContent = "GARBAGE line here\nTEMPO 100\nINVALID stuff\nKEY A major";
    MusicAgent agent(&mock);

    auto result = agent.generate("test");
    assert(!result.hasError);
    assert(result.instructions.size() == 2);
    assert(result.instructions[0].type == InstructionType::SetTempo);
    assert(result.instructions[1].type == InstructionType::SetKey);

    std::cout << "testMusicAgentParseMalformedSkipsUnknown passed\n";
}

void testMusicAgentParseEmptyOutput() {
    MockLlmClient mock;
    mock.responseContent = "";
    MusicAgent agent(&mock);

    auto result = agent.generate("test");
    assert(!result.hasError);
    assert(result.instructions.empty());

    std::cout << "testMusicAgentParseEmptyOutput passed\n";
}

void testMusicAgentParseBlankLines() {
    MockLlmClient mock;
    mock.responseContent = "\n\n\nTEMPO 90\n\n\n";
    MusicAgent agent(&mock);

    auto result = agent.generate("test");
    assert(!result.hasError);
    assert(result.instructions.size() == 1);
    assert(result.instructions[0].params[0] == "90");

    std::cout << "testMusicAgentParseBlankLines passed\n";
}

void testMusicAgentLlmUnavailable() {
    MockLlmClient mock;
    mock.available = false;
    MusicAgent agent(&mock);

    auto result = agent.generate("anything");
    assert(result.hasError);
    assert(!result.error.empty());

    std::cout << "testMusicAgentLlmUnavailable passed\n";
}

void testMusicAgentLlmFailure() {
    MockLlmClient mock;
    mock.shouldSucceed = false;
    mock.errorMsg = "rate limited";
    MusicAgent agent(&mock);

    auto result = agent.generate("compose");
    assert(result.hasError);
    assert(result.error == "rate limited");

    std::cout << "testMusicAgentLlmFailure passed\n";
}

void testMusicAgentNullClient() {
    MusicAgent agent(nullptr);

    auto result = agent.generate("test");
    assert(result.hasError);

    std::cout << "testMusicAgentNullClient passed\n";
}

void testMusicAgentRawOutputPreserved() {
    MockLlmClient mock;
    mock.responseContent = "TEMPO 120\nKEY C major";
    MusicAgent agent(&mock);

    auto result = agent.generate("test");
    assert(!result.hasError);
    assert(result.rawOutput == "TEMPO 120\nKEY C major");

    std::cout << "testMusicAgentRawOutputPreserved passed\n";
}

void testMusicAgentProjectContext() {
    MockLlmClient mock;
    mock.responseContent = "TEMPO 100";
    MusicAgent agent(&mock);

    agent.setProjectContext("tempo=120, key=C");
    auto result = agent.generate("change tempo");
    assert(!result.hasError);
    assert(result.instructions.size() == 1);

    std::cout << "testMusicAgentProjectContext passed\n";
}

// --- Main ---

int main() {
    // PromptParser tests
    testPromptParser();
    testPromptParserBpmAlias();
    testPromptParserKeyParams();
    testPromptParserUnknownCommand();
    testBuildMusicPromptNoContext();
    testBuildMusicPromptWithContext();
    testBuildMusicPromptEmptyContext();

    // MusicInstruction tests
    testMusicInstruction();
    testMusicInstructionDefaults();

    // RouterAgent tests
    testRouterClassifyCommand();
    testRouterClassifyMusic();
    testRouterClassifyBoth();
    testRouterClassifyDefaultsToMusic();
    testRouterClassifyWithWhitespace();
    testRouterClassifyLlmUnavailable();
    testRouterClassifyLlmError();
    testRouterClassifyNullClient();
    testRouterClassifyMeasuresTime();

    // MusicAgent tests
    testMusicAgentParseChord();
    testMusicAgentParseNote();
    testMusicAgentParseTempo();
    testMusicAgentParseKey();
    testMusicAgentParseTrack();
    testMusicAgentParseMultipleLines();
    testMusicAgentParseMalformedSkipsUnknown();
    testMusicAgentParseEmptyOutput();
    testMusicAgentParseBlankLines();
    testMusicAgentLlmUnavailable();
    testMusicAgentLlmFailure();
    testMusicAgentNullClient();
    testMusicAgentRawOutputPreserved();
    testMusicAgentProjectContext();

    std::cout << "All AI tests passed!\n";
    return 0;
}
