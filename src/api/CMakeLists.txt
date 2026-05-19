# AIDAW API Layer — pure virtual interfaces (header-only)
#
# These are abstract API surfaces consumed by the agent layer, scripting,
# and CLI. No .cpp files — all pure virtual or inline.

add_library(aidaw_api INTERFACE)

target_include_directories(aidaw_api INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Dependencies: core types (TypeIds, AutomationInfo, etc.) and project types
target_link_libraries(aidaw_api INTERFACE
    aidaw_core
    aidaw_project
    juce::juce_core
    juce::juce_audio_basics
)
