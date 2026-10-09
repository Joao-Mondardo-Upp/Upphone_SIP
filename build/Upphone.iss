; Instalador do Upphone SIP (Inno Setup 6)
; Normalmente chamado pelo build.ps1:
;   ISCC.exe /DAppVersion=1.0.0 /DPlatform=x64 /DSourceDir=..\dist\1.0.0\x64 Upphone.iss

#ifndef AppVersion
  #define AppVersion "1.0.0"
#endif
#ifndef Platform
  #define Platform "x64"
#endif
#ifndef SourceDir
  #define SourceDir "..\dist\" + AppVersion + "\" + Platform
#endif

#define AppName "Upphone SIP"
#define AppExe "Upphone.exe"

[Setup]
; AppId fixo: nao altere, e ele que permite atualizar uma instalacao existente
AppId={{6F2B8C1E-4D7A-4E93-9B5C-2A1F0E8D7C34}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher=Upp Tecnologia
AppPublisherURL=https://upphone.com.br/
AppSupportURL=https://upphone.com.br/
AppUpdatesURL=https://upphone.com.br/
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
UninstallDisplayIcon={app}\{#AppExe}
UninstallDisplayName={#AppName}
SetupIconFile=..\res\microsip.ico
OutputBaseFilename=Upphone-SIP-{#AppVersion}-{#Platform}-setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
UsedUserAreasWarning=no
CloseApplications=force
RestartApplications=no
VersionInfoVersion={#AppVersion}
VersionInfoCompany=Upp Tecnologia
VersionInfoProductName={#AppName}
#if Platform == "x64"
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
#endif

[Languages]
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "{#SourceDir}\{#AppExe}"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExe}"; Tasks: desktopicon

[Registry]
; O app procura esta chave: se apontar para a pasta do .exe, ele roda em modo "instalado"
; e guarda configuracoes em %APPDATA%\Upphone SIP\ (em vez de ao lado do .exe).
Root: HKLM; Subkey: "Software\{#AppName}"; ValueType: string; ValueName: ""; ValueData: "{app}"; Flags: uninsdeletekey
; O app se registra para iniciar com o Windows; remove essa entrada ao desinstalar
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: none; ValueName: "{#AppName}"; Flags: uninsdeletevalue dontcreatekey

[Run]
Filename: "{app}\{#AppExe}"; Description: "{cm:LaunchProgram,{#AppName}}"; Flags: nowait postinstall skipifsilent

[UninstallRun]
Filename: "{sys}\taskkill.exe"; Parameters: "/F /IM {#AppExe}"; Flags: runhidden; RunOnceId: "KillUpphone"
