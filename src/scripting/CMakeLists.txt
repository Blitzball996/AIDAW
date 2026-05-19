# Scripting - Embedded Lua runtime for AIDAW
#
# This module owns the embedded Lua VM and the AidawApi bindings plus the
# MIDI controller bridge. Because the bindings expose DAW types directly,
# aidaw_scripting will eventually link aidaw_daw — splitting the API types
# into a thin lib so this can be DAW-independent is tracked as future cleanup.

# ---------------------------------------------------------------------------
# Lua 5.4 static library
#
# NOTE: The Lua source must be fetched externally (e.g. via FetchContent in
# the root CMakeLists.txt). The variable `lua_SOURCE_DIR` should point to
# the Lua source tree. If using a system-installed Lua, adjust the
# target_link_libraries below accordingly.
# ---------------------------------------------------------------------------

set(LUA_SRC_DIR "${lua_SOURCE_DIR}/src")

set(LUA_CORE_SOURCES
    ${LUA_SRC_DIR}/lapi.c
    ${LUA_SRC_DIR}/lauxlib.c
    ${LUA_SRC_DIR}/lbaselib.c
    ${LUA_SRC_DIR}/lcode.c
    ${LUA_SRC_DIR}/lcorolib.c
    ${LUA_SRC_DIR}/lctype.c
    ${LUA_SRC_DIR}/ldblib.c
    ${LUA_SRC_DIR}/ldebug.c
    ${LUA_SRC_DIR}/ldo.c
    ${LUA_SRC_DIR}/ldump.c
    ${LUA_SRC_DIR}/lfunc.c
    ${LUA_SRC_DIR}/lgc.c
    ${LUA_SRC_DIR}/linit.c
    ${LUA_SRC_DIR}/liolib.c
    ${LUA_SRC_DIR}/llex.c
    ${LUA_SRC_DIR}/lmathlib.c
    ${LUA_SRC_DIR}/lmem.c
    ${LUA_SRC_DIR}/loadlib.c
    ${LUA_SRC_DIR}/lobject.c
    ${LUA_SRC_DIR}/lopcodes.c
    ${LUA_SRC_DIR}/loslib.c
    ${LUA_SRC_DIR}/lparser.c
    ${LUA_SRC_DIR}/lstate.c
    ${LUA_SRC_DIR}/lstring.c
    ${LUA_SRC_DIR}/lstrlib.c
    ${LUA_SRC_DIR}/ltable.c
    ${LUA_SRC_DIR}/ltablib.c
    ${LUA_SRC_DIR}/ltm.c
    ${LUA_SRC_DIR}/lundump.c
    ${LUA_SRC_DIR}/lutf8lib.c
    ${LUA_SRC_DIR}/lvm.c
    ${LUA_SRC_DIR}/lzio.c
)

add_library(lua_static STATIC ${LUA_CORE_SOURCES})

target_include_directories(lua_static
    PUBLIC
    ${LUA_SRC_DIR}
)

# Lua compiles as C; tell it which platform features are available.
if(APPLE)
    target_compile_definitions(lua_static PRIVATE LUA_USE_MACOSX)
elseif(UNIX)
    target_compile_definitions(lua_static PRIVATE LUA_USE_LINUX)
endif()

# Suppress warnings from Lua's source (third-party C code).
if(MSVC)
    target_compile_options(lua_static PRIVATE /w)
else()
    target_compile_options(lua_static PRIVATE -w)
endif()

set_target_properties(lua_static PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# ---------------------------------------------------------------------------
# aidaw_scripting — thin C++ wrapper exposing LuaRuntime + LuaSandbox.
# ---------------------------------------------------------------------------

set(AIDAW_SCRIPTING_SOURCES
    LuaRuntime.hpp
    LuaRuntime.cpp
    LuaSandbox.hpp
    LuaSandbox.cpp
    AidawApiLuaBindings.hpp
    AidawApiLuaBindings.cpp
    LuaController.hpp
    LuaController.cpp
    LuaScriptStore.hpp
    LuaScriptStore.cpp
)

add_library(aidaw_scripting STATIC ${AIDAW_SCRIPTING_SOURCES})

target_link_libraries(aidaw_scripting
    PUBLIC
    lua_static
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_devices
    # TODO: Link aidaw_daw (or equivalent) once the DAW API layer is ported.
    # The AidawApi bindings currently use stubs.
)

target_include_directories(aidaw_scripting
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

set_target_properties(aidaw_scripting PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

if(MSVC)
    target_compile_options(aidaw_scripting PRIVATE /W4)
else()
    target_compile_options(aidaw_scripting PRIVATE -Wall -Wextra -Wpedantic)
endif()
