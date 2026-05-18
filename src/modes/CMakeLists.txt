add_library(aidaw_modes STATIC
    ModeController.cpp
    ProMode.cpp
    AiMode.cpp
)

target_include_directories(aidaw_modes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(aidaw_modes PUBLIC juce::juce_core)
