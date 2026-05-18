set(AUDIO_SOURCES
    AudioBridge.cpp
    AudioThumbnailManager.cpp
    DeviceMeteringManager.cpp
    MidiBridge.cpp
    TrackController.cpp
    WarpMarkerManager.cpp
    WaveformPeakCache.cpp
)

set(AUDIO_HEADERS
    AudioBridge.hpp
    AudioThumbnailManager.hpp
    DeviceMeteringManager.hpp
    MidiBridge.hpp
    TrackController.hpp
    TransportStateManager.hpp
    WarpMarkerManager.hpp
    WaveformPeakCache.hpp
    midi/MidiInputRouter.hpp
    midi/MidiLearnSession.hpp
    midi/QwertyMidiKeyboard.hpp
)

# Prepend current source directory to all source files
list(TRANSFORM AUDIO_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
list(TRANSFORM AUDIO_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

# Create a library target for the audio module
add_library(aidaw_audio STATIC ${AUDIO_SOURCES} ${AUDIO_HEADERS})

target_include_directories(aidaw_audio
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Link JUCE modules
target_link_libraries(aidaw_audio
    PUBLIC
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_utils
        juce::juce_gui_basics
        juce::juce_events
)

# Conditionally link Tracktion Engine
if(TARGET tracktion_engine)
    target_link_libraries(aidaw_audio PUBLIC tracktion_engine)
    target_compile_definitions(aidaw_audio PUBLIC AIDAW_HAS_TRACKTION=1)
endif()
