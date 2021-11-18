; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{07AF81F8-4484-4672-90B4-6262A94E8E1B}
AppName={cm:AppNameBits}
AppVerName={cm:AppNameBits} ver.4.8.3
AppPublisher={cm:Author}
AppPublisherURL=http://katahiromz.web.fc2.com/
AppSupportURL=http://katahiromz.web.fc2.com/
AppUpdatesURL=http://katahiromz.web.fc2.com/
DefaultDirName={pf}\XWordGiver32
DefaultGroupName={cm:AppNameBits}
OutputDir=.
OutputBaseFilename=XWordGiver-x86-4.8.3-setup
Compression=lzma
SolidCompression=yes
UninstallDisplayIcon={app}\XWordGiver32.exe
SetupIconFile=res\Icon_1.ico
LicenseFile=LICENSE.txt
ChangesAssociations=yes

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "ja"; MessagesFile: "compiler:Languages\Japanese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "XWordGiver32.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "ReadMe-ENG.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "ReadMe-JPN.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "TechNote.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "HISTORY.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "DICT\BasicDict-ENG-Freedom.tsv"; DestDir: "{app}\DICT"; Flags: ignoreversion
Source: "DICT\BasicDict-ENG2JPN-Modesty.tsv"; DestDir: "{app}\DICT"; Flags: ignoreversion
Source: "DICT\BasicDict-ENG2JPN-Freedom.tsv"; DestDir: "{app}\DICT"; Flags: ignoreversion
Source: "DICT\BasicDict-JPN-Kana-Freedom.tsv"; DestDir: "{app}\DICT"; Flags: ignoreversion
Source: "DICT\BasicDict-JPN-Kana-Modesty.tsv"; DestDir: "{app}\DICT"; Flags: ignoreversion
Source: "DICT\BasicDict-JPN-Kanji-Freedom.tsv"; DestDir: "{app}\DICT"; Flags: ignoreversion
Source: "DICT\SubDict-JPN-Kotowaza.tsv"; DestDir: "{app}\DICT"; Flags: ignoreversion
Source: "DICT\SubDict-JPN-Kana-4KanjiWords.tsv"; DestDir: "{app}\DICT"; Flags: ignoreversion
Source: "DICT\BasicDict-RUS-Freedom.tsv"; DestDir: "{app}\DICT"; Flags: ignoreversion
Source: "BLOCK\circle.emf"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "BLOCK\club.emf"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "BLOCK\diamond.emf"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "BLOCK\heart.emf"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "BLOCK\ink-save-1.bmp"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "BLOCK\ink-save-2.emf"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "BLOCK\mario-block-1.emf"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "BLOCK\mario-block-2.emf"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "BLOCK\mario-block-3.emf"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "BLOCK\perfect-black.bmp"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "BLOCK\sakura.bmp"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "BLOCK\spade.emf"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "BLOCK\star.bmp"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "BLOCK\star.emf"; DestDir: "{app}\BLOCK"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "XWordGiver.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "pat\data.json"; DestDir: "{app}\pat"; Flags: ignoreversion
Source: "dict_analyze\dict_analyze.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "Policy-ENG.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "Policy-JPN.txt"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{cm:AppNameBits}"; Filename: "{app}\XWordGiver32.exe"
Name: "{group}\ReadMe-ENG.txt"; Filename: "{app}\ReadMe-ENG.txt"
Name: "{group}\ReadMe-JPN.txt"; Filename: "{app}\ReadMe-JPN.txt"
Name: "{group}\TechNote.txt"; Filename: "{app}\TechNote.txt"
Name: "{group}\HISTORY.txt"; Filename: "{app}\HISTORY.txt"
Name: "{group}\{cm:AuthorsHomepage}"; Filename: "http://katahiromz.web.fc2.com/"
Name: "{group}\{cm:UninstallProgram,{cm:AppName}}"; Filename: "{uninstallexe}"
Name: "{group}\DICT"; Filename: "{app}\DICT"
Name: "{group}\BLOCK"; Filename: "{app}\BLOCK"
Name: "{group}\LICENSE.txt"; Filename: "{app}\LICENSE.txt"
Name: "{commondesktop}\{cm:AppNameBits}"; Filename: "{app}\XWordGiver32.exe"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "Software\Katayama Hirofumi MZ\XWord32"; Flags: uninsdeletekey
Root: HKCR; Subkey: ".xwj"; ValueType: string; ValueName: ""; ValueData: "XWordGiver.JsonFile"; Flags: uninsdeletevalue; Tasks: association
Root: HKCR; Subkey: "XWordGiver.JsonFile"; ValueType: string; ValueName: ""; ValueData: "{cm:CrosswordJSONData}"; Flags: uninsdeletekey; Tasks: association
Root: HKCR; Subkey: "XWordGiver.JsonFile\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\XWordGiver32.exe,3"; Tasks: association
Root: HKCR; Subkey: "XWordGiver.JsonFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\XWordGiver32.exe"" ""%1"""; Tasks: association

[Tasks] 
Name: association; Description: "{cm:AssociateXWJFiles}"

[CustomMessages]
AppName=XWordGiver
AppNameBits=XWordGiver (x86)
Author=Katayama Hirofumi MZ
AssociateXWJFiles=Associate *.xwj files
CrosswordJSONData=Crossword JSON data
AuthorsHomepage=Author's homepage
ja.AppName=クロスワード ギバー
ja.AppNameBits=クロスワード ギバー (32ビット)
ja.Author=片山博文MZ
ja.AssociateXWJFiles=*.xwjファイルを関連付ける
ja.CrosswordJSONData=クロスワード JSON データ
ja.AuthorsHomepage=作者のホームページ

[Run]
Filename: "{app}\XWordGiver32.exe"; Description: "{cm:LaunchProgram,{cm:AppName}}"; Flags: nowait postinstall skipifsilent
