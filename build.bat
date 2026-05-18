@echo off
echo === AIDAW Build Script ===
echo.

if not exist build mkdir build
cd build

echo [1/2] Configuring CMake...
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo CMake configuration failed.
    echo Make sure third_party dependencies are cloned:
    echo   git submodule add https://github.com/juce-framework/JUCE.git third_party/juce
    echo   git submodule add https://github.com/Tracktion/tracktion_engine.git third_party/tracktion_engine
    echo   git submodule add https://github.com/ggerganov/llama.cpp.git third_party/llama.cpp
    exit /b 1
)

echo [2/2] Building...
cmake --build . --config Release
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo.
echo === Build complete! ===
