if(AIDAW_BUILD_TESTS)
    # Core tests
    add_executable(aidaw_core_tests core_tests.cpp)
    target_link_libraries(aidaw_core_tests PRIVATE aidaw_core aidaw_utils juce::juce_core)
    add_test(NAME CoreTests COMMAND aidaw_core_tests)

    # AI tests
    add_executable(aidaw_ai_tests ai_tests.cpp)
    target_link_libraries(aidaw_ai_tests PRIVATE aidaw_ai aidaw_utils juce::juce_core)
    add_test(NAME AiTests COMMAND aidaw_ai_tests)

    # Mode tests
    add_executable(aidaw_mode_tests mode_tests.cpp)
    target_link_libraries(aidaw_mode_tests PRIVATE aidaw_modes juce::juce_core)
    add_test(NAME ModeTests COMMAND aidaw_mode_tests)
endif()
