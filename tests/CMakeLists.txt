# Tests
#
# The full unit-test suite lives in tests/tests/ but was never wired into the
# build on this branch (root file was a placeholder). Wiring all ~100 source
# files risks unrelated compile breakage on a divergent branch, so we expose a
# focused verification target that builds only the MIDI-fix regression tests.
# Build with: cmake --build . --target magda_midi_fix_tests
#
# To opt into the full suite instead, set -DMAGDA_BUILD_FULL_TESTS=ON.

option(MAGDA_BUILD_FULL_TESTS "Build the full tests/tests suite" OFF)

if(MAGDA_BUILD_FULL_TESTS)
    add_subdirectory(tests)
endif()

# Focused regression target for the MIDI piano-roll / clip-split fixes.
add_executable(magda_midi_fix_tests
    tests/test_midi_clip_split.cpp
    tests/test_pianoroll_note_position.cpp
    tests/test_llm_response_parse.cpp
    tests/test_compact_parser_dialog.cpp
)

target_link_libraries(magda_midi_fix_tests
    PRIVATE
    magda_daw
    magda_agents
    magda_scripting
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_gui_basics
)

target_include_directories(magda_midi_fix_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}
)

add_test(NAME magda_midi_fix_tests COMMAND magda_midi_fix_tests)
