; Blitz DAW - Inno Setup Script
; Builds a Windows installer for Blitz

[Setup]
AppName=Blitz
AppVersion=0.1.0
AppPublisher=Blitz
AppPublisherURL=https://github.com/Blitzball996/AIDAW
DefaultDirName={autopf}\Blitz
DefaultGroupName=Blitz
OutputBaseFilename=Blitz-Setup
SetupIconFile=assets\app_icon.ico
Compression=lzma2
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible

[Files]
Source: "cmake-build-release\magda\daw\magda_daw_app_artefacts\Release\Blitz.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "cmake-build-release\magda\daw\magda_daw_app_artefacts\Release\magda_plugin_scanner.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "cmake-build-release\magda\daw\magda_daw_app_artefacts\Release\soundfonts\*"; DestDir: "{app}\soundfonts"; Flags: ignoreversion recursesubdirs
Source: "cmake-build-release\magda\daw\magda_daw_app_artefacts\Release\faustlibraries\*"; DestDir: "{app}\faustlibraries"; Flags: ignoreversion recursesubdirs
Source: "cmake-build-release\magda\daw\magda_daw_app_artefacts\Release\lang\*"; DestDir: "{app}\lang"; Flags: ignoreversion recursesubdirs
Source: "cmake-build-release\magda\daw\magda_daw_app_artefacts\Release\controllers\*"; DestDir: "{app}\controllers"; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{group}\Blitz"; Filename: "{app}\Blitz.exe"
Name: "{commondesktop}\Blitz"; Filename: "{app}\Blitz.exe"

[Run]
Filename: "{app}\Blitz.exe"; Description: "Launch Blitz"; Flags: nowait postinstall skipifsilent
