# Source modules
add_subdirectory(app)
add_subdirectory(core)
add_subdirectory(api)
add_subdirectory(audio)
add_subdirectory(music)
add_subdirectory(project)
add_subdirectory(modes)
add_subdirectory(ai)
add_subdirectory(ui)
add_subdirectory(utils)

# Main executable
juce_add_gui_app(AIDAW
    PRODUCT_NAME "AIDAW"
    VERSION "${PROJECT_VERSION}"
    COMPANY_NAME "AIDAW"
)

target_sources(AIDAW PRIVATE main.cpp)

target_link_libraries(AIDAW PRIVATE
    aidaw_app
    aidaw_core
    aidaw_api
    aidaw_audio
    aidaw_music
    aidaw_project
    aidaw_modes
    aidaw_ai
    aidaw_ui
    aidaw_utils
    juce::juce_audio_utils
    juce::juce_gui_extra
)

target_compile_definitions(AIDAW PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:AIDAW,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:AIDAW,JUCE_VERSION>"
)
