@echo off
echo === AIDAW SoundFont Downloader ===
echo.
echo This script downloads free SoundFont files for use with AIDAW.
echo Files will be placed in the soundfonts/ directory next to Blitz.exe.
echo.

set SF_DIR=%~dp0soundfonts
if not exist "%SF_DIR%" mkdir "%SF_DIR%"

echo [1/3] Downloading FluidR3_GM.sf2 (141MB - General MIDI)...
echo       Source: https://member.keymusician.com/Member/FluidR3_GM/FluidR3_GM.sf2
curl -L -# "https://member.keymusician.com/Member/FluidR3_GM/FluidR3_GM.sf2" -o "%SF_DIR%\FluidR3_GM.sf2"
if errorlevel 1 (
    echo       FAILED - trying alternate source...
    curl -L -# "https://archive.org/download/fluidr3-gm-gs/FluidR3_GM.sf2" -o "%SF_DIR%\FluidR3_GM.sf2"
)

echo.
echo [2/3] Downloading Yamaha DX7 ROM 1A (18MB - FM Synth)...
echo       Source: https://github.com/Caskexe/DX
echo       NOTE: This requires Git LFS. If download fails, clone the repo manually:
echo         git lfs clone https://github.com/Caskexe/DX.git
echo         copy "DX\DX7\Factory ROM\Yamaha DX7 ROM 1A.sf2" "%SF_DIR%\"
echo.

echo [3/3] Downloading GeneralUser GS (31MB - already included)...
if exist "%SF_DIR%\GeneralUser_GS.sf2" (
    echo       Already exists, skipping.
) else (
    curl -L -# "https://github.com/mrbumpy409/GeneralUser-GS/raw/main/GeneralUser%%20GS%%20v1.471.sf2" -o "%SF_DIR%\GeneralUser_GS.sf2"
)

echo.
echo === Download complete! ===
echo.
echo Available SoundFonts in %SF_DIR%:
dir /b "%SF_DIR%\*.sf2" "%SF_DIR%\*.sf3" 2>nul
echo.
echo To add more SoundFonts, simply copy .sf2 files to:
echo   %SF_DIR%
echo.
echo They will appear in the SoundFont Player's file selector.
pause
