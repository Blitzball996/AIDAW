---
name: building
description: Build, test, and run AIDAW. Use when compiling, running tests, or launching the app.
---

# Building AIDAW

## Prerequisites

- CMake 3.24+
- Ninja
- MSVC 2022 (Windows) / GCC 12+ / Clang 15+
- Git submodules cloned in third_party/

## Build Commands

| Command | Purpose |
|---------|---------|
| `build.bat` | Full release build (Windows) |
| `cmake -B build -G Ninja` | Configure |
| `cmake --build build` | Build |
| `cmake --build build --target aidaw_tests` | Build tests only |
| `ctest --test-dir build` | Run tests |

## Build Directories

- Default: `build/`
- App binary: `build/src/AIDAW_artefacts/`
- Test binary: `build/tests/aidaw_tests`

## Third-Party Dependencies

Must be cloned as git submodules before building:

```bash
git submodule add https://github.com/juce-framework/JUCE.git third_party/juce
git submodule add https://github.com/Tracktion/tracktion_engine.git third_party/tracktion_engine
git submodule add https://github.com/ggerganov/llama.cpp.git third_party/llama.cpp
```

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `AIDAW_ENABLE_LOCAL_LLM` | ON | Build with llama.cpp |
| `AIDAW_ENABLE_CLOUD_LLM` | ON | Build with cloud API support |
| `AIDAW_BUILD_TESTS` | ON | Build test suite |

## When to Build

- **Logic changes**: always build to verify compilation
- **Cosmetic changes** (UI sizes, colors): don't build automatically
- **New files added**: always reconfigure CMake first
