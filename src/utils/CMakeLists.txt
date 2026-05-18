add_library(aidaw_utils STATIC
    Logger.cpp
)

target_include_directories(aidaw_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(aidaw_utils PUBLIC juce::juce_core)
