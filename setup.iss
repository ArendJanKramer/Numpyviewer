; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!
#define MyAppName    "Numpy viewer"
#define MyAppExeName "numpyviewer.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{8F6DBECF-4E0C-4244-8BFD-6BEA29D30915}
AppName={#MyAppName}
AppVersion=1.6
;AppVerName=Numpy viewer 1.0
AppPublisher=Arend Jan Kramer
AppPublisherURL=https://github.com/ArendJanKramer/Numpyviewer
AppSupportURL=https://github.com/ArendJanKramer/Numpyviewer
AppUpdatesURL=https://github.com/ArendJanKramer/Numpyviewer
ChangesAssociations=yes
DefaultDirName={pf}\Numpy viewer
DisableProgramGroupPage=yes
OutputBaseFilename=setup
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Users\Arend Jan\numpyviewer\deploy windows\numpyviewer.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Arend Jan\numpyviewer\deploy windows\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\Users\Arend Jan\numpyviewer\deploy windows\vcredist_x64.exe"; DestDir: {tmp}; Flags: deleteafterinstall
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Registry]
Root: HKCR; Subkey: ".npy"; ValueType: string; ValueName: ""; ValueData: "{#MyAppName}"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".npz"; ValueType: string; ValueName: ""; ValueData: "{#MyAppName}"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "{#MyAppName}"; ValueType: string; ValueName: ""; ValueData: "Program {#MyAppName}"; Flags: uninsdeletekey
Root: HKCR; Subkey: "{#MyAppName}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKCR; Subkey: "{#MyAppName}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

[Icons]
Name: "{commonprograms}\Numpy viewer"; Filename: "{app}\numpyviewer.exe"
Name: "{commondesktop}\Numpy viewer"; Filename: "{app}\numpyviewer.exe"; Tasks: desktopicon

[Run]
Filename: {tmp}\vcredist_x64.exe; \
    Parameters: "/install /quiet /norestart "; \
    StatusMsg: "Installing VC++ 2015 Redistributables..."
Filename: "{app}\numpyviewer.exe"; Description: "{cm:LaunchProgram,Numpy viewer}"; Flags: nowait postinstall skipifsilent

