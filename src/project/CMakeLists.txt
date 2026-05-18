# AIDAW Project Management Module
# Project lifecycle, serialization, and metadata.
#
# NOTE: ProjectManager.cpp and ProjectSerializer depend on DAW-engine singletons
# (TrackManager, ClipManager, AutomationManager). These are guarded behind
# AIDAW_HAS_DAW_ENGINE. When building without the full DAW engine, only the
# ProjectInfo struct and SerializationHelpers are usable standalone.

add_library(aidaw_project STATIC
    ProjectManager.cpp
)

target_include_directories(aidaw_project
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(aidaw_project
    PUBLIC
        juce::juce_core
        juce::juce_events
        juce::juce_gui_basics
)

# Optionally link DAW engine modules when available
if(TARGET aidaw_core)
    target_link_libraries(aidaw_project PRIVATE aidaw_core)
    target_compile_definitions(aidaw_project PRIVATE AIDAW_HAS_DAW_ENGINE=1)
endif()
