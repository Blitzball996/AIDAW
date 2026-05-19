add_library(aidaw_ui STATIC
    MainWindow.cpp
    pro/ProMainView.cpp
    ai/AiMainView.cpp
    ai/ChatPanel.cpp
    panels/TransportPanel.cpp
    views/ArrangementView.cpp
    views/MixerView.cpp
    views/SessionView.cpp
    themes/DarkTheme.cpp
)

target_include_directories(aidaw_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(aidaw_ui PUBLIC
    juce::juce_gui_extra
    aidaw_modes
    aidaw_ai
)
