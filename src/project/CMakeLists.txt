# AIDAW Project Management Module
# Header-only for now — ProjectManager.cpp needs DAW engine singletons not yet wired.

add_library(aidaw_project INTERFACE)

target_include_directories(aidaw_project
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
