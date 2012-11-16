!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"

!define VERSION 1.0
!define VERSIONMAJOR 1
!define VERSIONMINOR 0

Name "Revelade Revolution"
Caption "Revelade Revolution ${VERSION} Setup"
OutFile "reveladerevolution_${VERSION}_setup.exe"
ShowInstDetails hide
RequestExecutionLevel admin
InstallDir "$PROGRAMFILES\ReveladeRevolution"
InstallDirRegKey HKLM "Software\ReveladeRevolution" "InstallLocation"

; Comment the following line and uncomment the next to use a generic installer icon
!define MUI_ICON "rr.ico"
;!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico"

!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"

VIAddVersionKey "ProductName" "Revelade Revolution"
VIAddVersionKey "CompanyName" "The Intercooler Games"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIProductVersion "${VERSION}.0.0"

;!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
;!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
 
!insertmacro MUI_LANGUAGE "English"

Section "!Core Files" SecCore

  SetDetailsPrint textonly
  DetailPrint "Installing Revelade Revolution ${VERSION}..."
  SetDetailsPrint listonly
  
  SectionIn RO
  
  SetOutPath $INSTDIR
  SetOverwrite on
  
  File /r ".\ReveladeRevolution\*"
    
SectionEnd

Section "Start Menu Shortcuts" SecSMShortcuts

  SetDetailsPrint textonly
  DetailPrint "Installing Start Menu Shortcuts..."
  SetDetailsPrint listonly
  
  SectionIn 1
  SetOutPath $INSTDIR
  
  CreateDirectory "$SMPROGRAMS\Revelade Revolution\"
  CreateShortCut "$SMPROGRAMS\Revelade Revolution\Revelade Revolution.lnk" "$INSTDIR\bin\RR_GAME.exe" '"-q$$HOME\My Games\Revelade Revolution" -r %*'
  CreateShortCut "$SMPROGRAMS\Revelade Revolution\Readme.lnk" "$INSTDIR\Readme.txt"
  CreateShortCut "$SMPROGRAMS\Revelade Revolution\Uninstall.lnk" "$INSTDIR\uninst-rr.exe"

SectionEnd

Section "Desktop Shortcut" SecDTShortcut

  SetDetailsPrint textonly
  DetailPrint "Installing Desktop Shortcut..."
  SetDetailsPrint listonly

  SectionIn 1
  SetOutPath $INSTDIR

  CreateShortCut "$DESKTOP\Revelade Revolution.lnk" "$INSTDIR\bin\RR_GAME.exe" '"-q$$HOME\My Games\Revelade Revolution" -r %*'

SectionEnd

Function CheckVCRedist
	ReadRegDword $R0 HKLM "SOFTWARE\Wow6432Node\Microsoft\DevDiv\VC\Servicing\10.0\RED\1033" "SPName"
	${If} $R0 == "RTM"
		return
	${Else}
		ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\DevDiv\VC\Servicing\10.0\RED\1033" "SPName"
		${If} $R0 == "RTM"
			return
		${Else}
			StrCpy $R0 "-1"
		${EndIf}
	${EndIf}
FunctionEnd

Section -post

  WriteUninstaller $INSTDIR\uninst-rr.exe
  
  SetDetailsPrint textonly
  DetailPrint "Adding registry keys..."
  SetDetailsPrint listonly
  SetDetailsPrint both
  
  WriteRegStr HKLM "Software\ReveladeRevolution" "InstallLocation" $INSTDIR
  WriteRegStr HKLM "Software\ReveladeRevolution" "Version" ${VERSION}
  WriteRegDWORD HKLM "Software\ReveladeRevolution" "MajorVersion" ${VERSIONMAJOR}
  WriteRegDWORD HKLM "Software\ReveladeRevolution" "MinorVersion" ${VERSIONMINOR}
  
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReveladeRevolution" "UninstallString" "$INSTDIR\uninst-rr.exe"
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReveladeRevolution" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReveladeRevolution" "DisplayName" "Revelade Revolution"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReveladeRevolution" "DisplayIcon" "$INSTDIR\bin\RR_GAME.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReveladeRevolution" "DisplayVersion" "${VERSION}"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReveladeRevolution" "URLInfoAbout" "http://theintercoolergames.com/"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReveladeRevolution" "HelpLink" "http://theintercoolergames.boards.net/"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReveladeRevolution" "NoModify" "1"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReveladeRevolution" "NoRepair" "1"

SectionEnd

Section Uninstall

  SetDetailsPrint textonly
  DetailPrint "Uninstalling Revelade Revolution ${VERSION}..."
  SetDetailsPrint listonly

  Delete "$SMPROGRAMS\Revelade Revolution\*"
  RMDir /r "$SMPROGRAMS\Revelade Revolution"
  Delete "$DESKTOP\Revelade Revolution.lnk"
  RMDir /r $INSTDIR
  
  DeleteRegKey HKLM "Software\ReveladeRevolution"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ReveladeRevolution"

  SetDetailsPrint both

SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "The core files required to play Revelade Revolution"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSMShortcuts} "Add shortcuts to Start menu"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDTShortcut} "Add a shortcut to your desktop"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

!verbose 3
