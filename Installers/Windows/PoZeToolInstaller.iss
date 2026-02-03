[Setup]
AppName=PoZeTool
AppVersion={#AppVersion}
DefaultDirName="{commonpf64}\RV Audio Design"
DisableDirPage=yes
DefaultGroupName=PozeTool
OutputBaseFilename=PozeTool-{#AppVersion}-Windows
InfoBeforeFile="..\readme.rtf"
LicenseFile="..\license.rtf"
SetupIconFile="..\Plot.ico"
Compression=lzma2
WizardStyle=modern

[Types]
Name: "Install_All"; Description: "Install All"
Name: "Install_VST3"; Description: "Install VST3"
Name: "Install_Standalone"; Description: "Install Standalone Plug-in"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "VST3"; Description: "VST3 Plugin"; Types: Install_All Install_VST3
Name: "Standalone"; Description: "Standalone Application"; Types: Install_All Install_Standalone

[Files]
Source: "..\..\Builds\PozeTool_artefacts\Release\VST3\PozeTool.vst3\*"; \
    DestDir: {code:GetVST3Dir}\PozeTool.vst3; \
    Components: VST3; \
    Flags: ignoreversion recursesubdirs createallsubdirs

Source: "..\..\Builds\PozeTool_artefacts\Release\Standalone\PozeTool.exe"; \
    DestDir: {code:GetStandaloneDir}; \
    Components: Standalone; \
    Flags: ignoreversion

[Icons]
Name: "{commondesktop}\PozeTool"; Filename: "{code:GetStandaloneDir}\PozeTool.exe"; IconFilename: "..\Plot.ico"; Components: Standalone
Name: "{group}\PozeTool"; Filename: "{code:GetStandaloneDir}\PozeTool.exe"; IconFilename: "..\Plot.ico"; Components: Standalone

[Code]
var
  StandalonePage: TInputDirWizardPage;
  VST3Page: TInputDirWizardPage;
  AAXPage: TInputDirWizardPage;

function GetStandaloneDir(Value: string): string;
begin
  Result := StandalonePage.Values[0];
end;

function GetVST3Dir(Value: string): string;
begin
  Result := VST3Page.Values[0];
end;

function GetAAXDir(Value: string): string;
begin
  Result := AAXPage.Values[0];
end;

procedure InitializeWizard;
begin
  { Create folder selection pages after the Components page }
  StandalonePage := CreateInputDirPage(wpSelectComponents,
    'Select Standalone Application Folder', '',
    'Select the folder where the Standalone application will be installed:',
    False, '');
  StandalonePage.Add('');
  StandalonePage.Values[0] := ExpandConstant('{commonpf64}\RV Audio Design');

  VST3Page := CreateInputDirPage(StandalonePage.ID,
    'Select VST3 Plugin Folder', '',
    'Select the folder where the VST3 plugin will be installed:',
    False, '');
  VST3Page.Add('');
  VST3Page.Values[0] := ExpandConstant('{commoncf64}\VST3\RV Audio Design');
end;

function ShouldSkipPage(PageID: Integer): Boolean;
begin
  Result := False;

  if PageID = StandalonePage.ID then
    Result := not WizardIsComponentSelected('Standalone')
  else if PageID = VST3Page.ID then
    Result := not WizardIsComponentSelected('VST3')
end;