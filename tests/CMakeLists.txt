if(AIDAW_BUILD_TESTS)
    add_executable(aidaw_tests
        core_tests.cpp
        ai_tests.cpp
        mode_tests.cpp
    )

    target_link_libraries(aidaw_tests PRIVATE
        aidaw_core
        aidaw_ai
        aidaw_modes
        aidaw_utils
        juce::juce_core
    )

    add_test(NAME AidawTests COMMAND aidaw_tests)
endif()
