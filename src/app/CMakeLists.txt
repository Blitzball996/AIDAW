add_library(aidaw_app STATIC
    AiDaw.cpp
    AppConfig.cpp
)

target_include_directories(aidaw_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(aidaw_app PUBLIC
    juce::juce_gui_extra
    juce::juce_audio_utils
    aidaw_modes
    aidaw_ui
    aidaw_core
)
