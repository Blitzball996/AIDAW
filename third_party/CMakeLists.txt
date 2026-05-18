# Third-party dependencies for AIDAW
# Clone these as git submodules:
#   git submodule add https://github.com/juce-framework/JUCE.git third_party/juce
#   git submodule add https://github.com/Tracktion/tracktion_engine.git third_party/tracktion_engine
#   git submodule add https://github.com/ggerganov/llama.cpp.git third_party/llama.cpp

# JUCE
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/juce/CMakeLists.txt)
    add_subdirectory(juce)
    message(STATUS "JUCE found")
else()
    message(WARNING "JUCE not found. Run: git submodule add https://github.com/juce-framework/JUCE.git third_party/juce")
endif()

# Tracktion Engine
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tracktion_engine/CMakeLists.txt)
    add_subdirectory(tracktion_engine)
    message(STATUS "Tracktion Engine found")
else()
    message(WARNING "Tracktion Engine not found. Run: git submodule add https://github.com/Tracktion/tracktion_engine.git third_party/tracktion_engine")
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
