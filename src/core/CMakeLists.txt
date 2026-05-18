add_library(aidaw_core STATIC
    Engine.cpp
    TrackManager.cpp
    ClipManager.cpp
    MixerEngine.cpp
    PluginHost.cpp
    TransportControl.cpp
    UndoManager.cpp
    AutomationManager.cpp
    SelectionManager.cpp
    Config.cpp
    ClipCommands.cpp
    TrackCommands.cpp
    MidiNoteCommands.cpp
)

target_include_directories(aidaw_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(aidaw_core PUBLIC
    juce::juce_audio_utils
)

if(TARGET tracktion::tracktion_engine)
    target_link_libraries(aidaw_core PUBLIC tracktion::tracktion_engine)
    target_compile_definitions(aidaw_core PUBLIC AIDAW_HAS_TRACKTION=1)
endif()
