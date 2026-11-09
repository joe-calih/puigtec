#define StageRoot GetEnv("BLUE_STEEL_STAGE")

[Setup]
AppId={{8E184880-07B3-4DFB-8DC9-AFF98F69FA87}
AppName=Blue Steel EQ
AppVersion=2.0.4
AppPublisher=LOOPS.CO.KE
AppPublisherURL=https://loops.co.ke
AppSupportURL=https://loops.co.ke/contact
AppUpdatesURL=https://github.com/joe-calih/puigtec/releases/latest
DefaultDirName={autopf}\Blue Steel EQ v2
DefaultGroupName=Blue Steel EQ v2
UninstallDisplayName=Blue Steel EQ v2
OutputDir=output
OutputBaseFilename=Blue-Steel-EQ-Setup
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
DisableProgramGroupPage=yes
Uninstallable=yes
CloseApplications=yes
RestartApplications=no

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional shortcuts:"; Flags: unchecked

[Files]
Source: "{#StageRoot}\VST3\Blue Steel EQ v2.vst3\*"; DestDir: "{commoncf}\VST3\Blue Steel EQ v2.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#StageRoot}\Standalone\Blue Steel EQ.exe"; DestDir: "{app}"; DestName: "Blue Steel EQ v2.exe"; Flags: ignoreversion
Source: "{#StageRoot}\Standalone\JUCE-LICENSE.md"; DestDir: "{app}"; DestName: "JUCE-LICENSE.txt"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; DestName: "License.txt"; Flags: ignoreversion
Source: "{#StageRoot}\Standalone\AGPL-3.0.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\docs\V2-RELEASE-NOTES.md"; DestDir: "{app}"; DestName: "Version-2.0.4-Release-Notes.md"; Flags: ignoreversion

[Icons]
Name: "{group}\Blue Steel EQ v2"; Filename: "{app}\Blue Steel EQ v2.exe"
Name: "{autodesktop}\Blue Steel EQ v2"; Filename: "{app}\Blue Steel EQ v2.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\Blue Steel EQ v2.exe"; Description: "Launch Blue Steel EQ v2"; Flags: postinstall nowait skipifsilent
Filename: "https://loops.co.ke"; Description: "Open LOOPS.CO.KE — royalty-free sounds and production tools"; Flags: postinstall shellexec skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf}\VST3\Blue Steel EQ v2.vst3"
