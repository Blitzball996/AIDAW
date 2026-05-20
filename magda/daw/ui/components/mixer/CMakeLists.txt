target_sources(magda_daw_app PRIVATE
    MasterChannelStrip.cpp
    InputTypeSelector.cpp
    RoutingSelector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VCAStrip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SendKnob.cpp
    MasterChannelStrip.hpp
    InputTypeSelector.hpp
    RoutingSelector.hpp
    MidiNoteStrip.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VCAStrip.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SendKnob.hpp
)
