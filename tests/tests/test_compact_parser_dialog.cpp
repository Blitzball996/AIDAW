// Regression tests for the compact instruction parser.
//
// Focus: the ASK / SAY channel and the PARAM opcode. Before ASK existed, a
// model that asked a question emitted prose, which the parser dropped and the
// user saw as "nothing happened". These tests pin the parse side of that
// contract; the abort-the-batch half lives in InstructionExecutor::execute.

#include <catch2/catch_test_macros.hpp>

#include <variant>

#include "magda/agents/compact_parser.hpp"

using namespace magda;

namespace {

const Instruction* firstOf(const std::vector<Instruction>& v, OpCode code) {
    for (const auto& i : v)
        if (i.opcode == code)
            return &i;
    return nullptr;
}

}  // namespace

TEST_CASE("ASK keeps the question text verbatim") {
    CompactParser parser;
    auto ir = parser.parse(
        "ASK There is no drum track - add reverb to Percussion instead, or create one?");

    REQUIRE(ir.size() == 1);
    const auto* ask = firstOf(ir, OpCode::Ask);
    REQUIRE(ask != nullptr);
    // Punctuation and spacing must survive: the question is shown to a human.
    REQUIRE(std::get<AskOp>(ask->payload).question
            == "There is no drum track - add reverb to Percussion instead, or create one?");
}

TEST_CASE("SAY is parsed alongside real work") {
    CompactParser parser;
    auto ir = parser.parse("PARAM cutoff=60%\nSAY Softened the tone; ask me for 7ths too.");

    REQUIRE(ir.size() == 2);
    REQUIRE(firstOf(ir, OpCode::Param) != nullptr);
    const auto* say = firstOf(ir, OpCode::Say);
    REQUIRE(say != nullptr);
    REQUIRE(std::get<SayOp>(say->payload).message == "Softened the tone; ask me for 7ths too.");
}

TEST_CASE("A bare ASK with no text is dropped rather than becoming an empty question") {
    CompactParser parser;
    auto ir = parser.parse("ASK   \nTRACK Bass");

    REQUIRE(firstOf(ir, OpCode::Ask) == nullptr);
    REQUIRE(firstOf(ir, OpCode::Track) != nullptr);
}

TEST_CASE("PARAM accepts implicit and explicit track targets") {
    CompactParser parser;

    SECTION("implicit target") {
        auto ir = parser.parse("PARAM cutoff=80% resonance=30%");
        REQUIRE(ir.size() == 1);
        const auto& p = std::get<ParamOp>(ir[0].payload);
        REQUIRE(p.target.isImplicit());
        REQUIRE(p.params.getValue("cutoff", "") == "80%");
        REQUIRE(p.params.getValue("resonance", "") == "30%");
    }

    SECTION("explicit numeric target") {
        auto ir = parser.parse("PARAM 2 attack=40%");
        REQUIRE(ir.size() == 1);
        const auto& p = std::get<ParamOp>(ir[0].payload);
        REQUIRE(p.target.isById());
        REQUIRE(p.target.id == 2);
        REQUIRE(p.params.getValue("attack", "") == "40%");
    }
}

TEST_CASE("PARAM without any key=value is an error, not a silent no-op") {
    CompactParser parser;
    auto ir = parser.parse("PARAM");

    REQUIRE(ir.empty());
    REQUIRE(parser.getLastError().isNotEmpty());
}
