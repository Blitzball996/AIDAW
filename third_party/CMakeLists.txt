# Third-party dependencies for AIDAW
#
# Required submodules:
#   git submodule add https://github.com/Tracktion/tracktion_engine.git third_party/tracktion_engine
#   git submodule add https://github.com/ggerganov/llama.cpp.git third_party/llama.cpp
#
# JUCE: either place in third_party/juce/ OR let Tracktion Engine provide it
#   via its modules/juce submodule (run: git submodule update --init --recursive)

# JUCE — check standalone first, then fall back to Tracktion Engine's bundled copy
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/juce/CMakeLists.txt)
    add_subdirectory(juce)
    message(STATUS "JUCE found (standalone)")
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tracktion_engine/modules/juce/CMakeLists.txt)
    add_subdirectory(tracktion_engine/modules/juce)
    message(STATUS "JUCE found (via Tracktion Engine)")
else()
    message(WARNING "JUCE not found. Either:\n"
        "  1. Place JUCE in third_party/juce/, OR\n"
        "  2. Run: cd third_party/tracktion_engine && git submodule update --init modules/juce")
endif()

# Tracktion Engine — disabled until JUCE/TE version compatibility is resolved
# The current TE develop branch requires JUCE develop (8.x), but we have JUCE 7.0.12.
# TODO: Either pin TE to a compatible release, or switch JUCE to develop branch.
option(AIDAW_ENABLE_TRACKTION "Enable Tracktion Engine" ON)
if(AIDAW_ENABLE_TRACKTION)
    set(TE_ADD_EXAMPLES OFF CACHE BOOL "" FORCE)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tracktion_engine/CMakeLists.txt)
        add_subdirectory(tracktion_engine)
        message(STATUS "Tracktion Engine found")
    else()
        message(WARNING "Tracktion Engine not found.")
    endif()
endif()

# llama.cpp (local LLM)
if(AIDAW_ENABLE_LOCAL_LLM)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/llama.cpp/CMakeLists.txt)
        set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        add_subdirectory(llama.cpp)
        message(STATUS "llama.cpp found")
    else()
        message(WARNING "llama.cpp not found. Run: git submodule add https://github.com/ggerganov/llama.cpp.git third_party/llama.cpp")
    endif()
endif()
