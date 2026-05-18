cmake_minimum_required(VERSION 3.24)

project(AIDAW VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W3 /FS /Zc:char8_t-)
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

# JUCE options
option(JUCE_BUILD_EXTRAS "Build JUCE extras" OFF)
option(JUCE_BUILD_EXAMPLES "Build JUCE examples" OFF)

# Options
option(AIDAW_ENABLE_LOCAL_LLM "Enable local llama.cpp backend" ON)
option(AIDAW_ENABLE_CLOUD_LLM "Enable cloud API backend" ON)
option(AIDAW_BUILD_TESTS "Build tests" ON)

# Third-party dependencies
add_subdirectory(third_party)

# Main source
add_subdirectory(src)

# Tests
if(AIDAW_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
