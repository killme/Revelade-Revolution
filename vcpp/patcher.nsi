!include "MUI2.nsh"
!include "LogicLib.nsh"

!define VERSION 1.0
!define VERSIONMAJOR 1
!define VERSIONMINOR 0
!define PATCHVERSION 1

Name "Revelade Revolution Patch"
Caption "Revelade Revolution ${VERSION} Patch ${PATCHVERSION} Setup"
OutFile "reveladerevolution_${VERSION}_patch_${PATCHVERSION}_setup.exe"
ShowInstDetails hide
RequestExecutionLevel admin
InstallDir "$PROGRAMFILES\ReveladeRevolution"
InstallDirRegKey HKLM "Software\ReveladeRevolution" "InstallLocation"

; Comment the following line and uncomment the next to use a generic installer icon
!define MUI_ICON "rr.ico"
;!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico"

!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"

VIAddVersionKey "ProductName" "Revelade Revolution Patch"
VIAddVersionKey "CompanyName" "The Intercooler Games"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIProductVersion "${VERSION}.0.0"

;!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
;!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
;!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!define MUI_CUSTOMFUNCTION_GUIINIT GUIInit

Function GUIInit
  
  ReadRegDWORD $R0 HKLM "Software\ReveladeRevolution" "MajorVersion"
  ReadRegDWORD $R1 HKLM "Software\ReveladeRevolution" "MinorVersion"

  ;ReadRegStr $R0 HKLM "Software\ReveladeRevolution" "Version"
  ${If} $R0 > ${VERSIONMAJOR}
    MessageBox MB_YESNO|MB_ICONQUESTION "Setup has detected that you do not have Revelade Revolution ${VERSION} installed.$\r$\nContinue anyway?" IDYES InstallAnyway
    Abort
  ${EndIf}
  ${If} $R0 == ${VERSIONMAJOR}
	  ${If} $R1 > ${VERSIONMINOR}
		MessageBox MB_YESNO|MB_ICONQUESTION "Setup has detected that you do not have Revelade Revolution $R0.$R1 installed.$\r$\nContinue anyway?" IDYES InstallAnyway
		Abort
	  ${EndIf}
  ${EndIf}
  
  ReadRegDWORD $R0 HKLM "Software\ReveladeRevolution" "Patch"
  ${If} $R0 > ${PATCHVERSION}
    MessageBox MB_YESNO|MB_ICONQUESTION "Setup has detected that you have a newer patch ($R0) installed.$\r$\nContinue anyway?" IDYES InstallAnyway
    Abort
  ${EndIf}
  
  InstallAnyway:

FunctionEnd
 
!insertmacro MUI_LANGUAGE "English"

Section "!Core Files" SecCore

  SetDetailsPrint textonly
  DetailPrint "Installing Revelade Revolution ${VERSION} Patch ${PATCHVERSION}..."
  SetDetailsPrint listonly
  
  SectionIn RO
  
  SetOutPath $INSTDIR
  SetOverwrite on
  
  File /r ".\ReveladeRevolution\*"
    
SectionEnd

Section -post

  SetDetailsPrint both
  WriteRegDWORD HKLM "Software\ReveladeRevolution" "Patch" ${PATCHVERSION}

SectionEnd

!verbose 3
