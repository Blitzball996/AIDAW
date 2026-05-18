# AIDAW Music Theory Module
# Standalone music theory: chord detection, scale detection, chord suggestions.

add_library(aidaw_music STATIC
    ChordEngine.cpp
    ChordSuggestionEngine.cpp
)

target_include_directories(aidaw_music
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(aidaw_music
    PUBLIC
        juce::juce_core
)

# Header-only components (no .cpp needed):
#   ChordEnums.hpp
#   ChordTypes.hpp
#   ChordCache.hpp
#   LRUCache.hpp
#   Scales.hpp
#   ScaleDetector.hpp
#   KeyModeHistogram.hpp
