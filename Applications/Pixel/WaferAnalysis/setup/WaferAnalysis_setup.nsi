# Installation script for USBpix FE-I4
#
# To create a setup basing on this script you need NSIS.
# It can be downloaded at http://nsis.sourceforge.net
#
# The Setup copies all files needed to run the WaferAnalysis Applications.
# No further Software Packages are required.
#
# Version History:
# 1.0   5.11.2012   Author: David-Leon Pohl

Name WaferAnalysis

SetCompressor /SOLID lzma

# General Symbol Definitions
!define REGKEY "SOFTWARE\$(^Name)"
!define VERSION 1.1.1
!define COMPANY SiLAB
!define URL http://siliconlab.physik.uni-bonn.de/

# ======================
# Paths to Source Files
# ======================

# Location of the USBpixI4 Repositories (Binaries will be taken from bin subdir)
!define SRC_DIR "C:\USBpixI4svn\host\trunk\"

# Location of qwt libraries (compiled for release)
!define QWT_SRC_DIR "C:\qwt-5.2.1\lib"


!define QT_SRC_DIR "C:\Qt\4.6.3\bin"

# Location of vcredist_x86.exe, just google and download the Visual C++ 2008 Runtime
!define MSVC_SRC_DIR "C:\Users\DavidLP\Downloads"

#Version of root included in this installer
!define INCLUDED_ROOT_VERSION "5.32.04"

# ======================
# End of path section
# ======================

# MUI Symbol Definitions
!define MUI_ICON "${SRC_DIR}\Applications\Pixel\WaferAnalysis\WaferAnalysisIcon.ico"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
!define MUI_STARTMENUPAGE_REGISTRY_KEY ${REGKEY}
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME StartMenuGroup
!define MUI_STARTMENUPAGE_DEFAULTFOLDER WaferAnalysis
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall-colorful.ico"
!define MUI_UNFINISHPAGE_NOAUTOCLOSE
!define MUI_WELCOMEPAGE_TEXT "This setup installs the Wafer/Module analysis program as a stand alone installation.$\n$\npohl@physik.uni-bonn.de"

# Included files
!include Sections.nsh
!include MUI2.nsh
!include x64.nsh
!include FileAssociation.nsh

# Variables
Var StartMenuGroup
Var VCRedistInstalled
Var RootInstalled

# Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuGroup
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
#!insertmacro RegisterExtension
#!insertmacro UnRegisterExtension

# Installer languages
!insertmacro MUI_LANGUAGE English

# Installer attributes
OutFile WaferAnalysisSetup_Release_1_1_1.exe
InstallDir "$PROGRAMFILES\WaferAnalysis"
CRCCheck on
XPStyle on
ShowInstDetails show
VIProductVersion 1.1.1.0
VIAddVersionKey ProductName WaferAnalysis
VIAddVersionKey ProductVersion "${VERSION}"
VIAddVersionKey CompanyName "${COMPANY}"
VIAddVersionKey CompanyWebsite "${URL}"
VIAddVersionKey FileVersion "${VERSION}"
VIAddVersionKey FileDescription ""
VIAddVersionKey LegalCopyright ""
InstallDirRegKey HKLM "${REGKEY}" Path
ShowUninstDetails show


# Installer sections
!macro CREATE_SMGROUP_SHORTCUT NAME PATH
    Push "${NAME}"
    Push "${PATH}"
    Call CreateSMGroupShortcut
!macroend

SectionGroup "External Libraries" SECGRP0000
    Section "ROOT ${INCLUDED_ROOT_VERSION}" SEC0003
    SetOutPath $INSTDIR
    SetOverwrite on
        
        Call CheckRootInstalled
        ${If} $RootInstalled == 0
           DetailPrint "Installed ROOT is the same as the included. Skipping ROOT installation."
           Goto exit
        ${EndIf}
    
    ${If} $RootInstalled == 2
       # ROOT is already installed, call uninstaller
       MessageBox MB_YESNO "The ROOT Version installed on your System is not tested with WaferAnalysis.$\r$\n Most likely your version will not work. $\r$\nTo install the tested ROOT ${INCLUDED_ROOT_VERSION} your installed ROOT has to be removed. The installer will download the needed version automatially$\r$\n$\r$\nDo you want to change your ROOT version?" /SD IDYES IDYES uninstall_root IDNO exit
    ${EndIf}
    
    # Install ROOT
    Goto install_root
    
    uninstall_root:
        #ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{F570A3D8-BC0D-408E-BBE3-57E6DEEE5AAA}" "UninstallString"
        #ExecWait '$R0'
        ExecWait 'MsiExec.exe /qr /x{F570A3D8-BC0D-408E-BBE3-57E6DEEE5AAA}'
        DetailPrint "Removed previously installed ROOT"
        Goto install_root
    
    install_root:
        InitPluginsDir
        inetc::get "ftp://root.cern.ch/root/root_v${INCLUDED_ROOT_VERSION}.win32.vc90.msi" "$PLUGINSDIR\root_v${INCLUDED_ROOT_VERSION}.win32.vc90.msi"
        Pop $0
        StrCmp $0 "OK" dlok
        MessageBox MB_OK|MB_ICONEXCLAMATION "Cannot find ftp://root.cern.ch/root/root_v${INCLUDED_ROOT_VERSION}.win32.vc90.msi Please check internet connection!" /SD IDOK
        Abort
    dlok:
        DetailPrint "Installing ${INCLUDED_ROOT_VERSION}"
        ExecWait 'msiexec /i "$PLUGINSDIR\root_v${INCLUDED_ROOT_VERSION}.win32.vc90.msi"'
        DetailPrint "Root ${INCLUDED_ROOT_VERSION} installed"
        #Delete /REBOOTOK "$INSTDIR\root_v5.32.00.win32.vc90.msi"
        Goto exit
    
    exit:
    SectionEnd
    
    Section "Visual C++ 2008 Runtime" SEC0004
        Call CheckVCRedist
        ${If} $VCRedistInstalled == 1
        DetailPrint "VC Runtime is already installed."
        ${Else}
           SetOutPath $INSTDIR
           SetOverwrite on
           File "${MSVC_SRC_DIR}\vcredist_x86.exe"

           #run runtime installation tool
           DetailPrint "Installing Microsoft Visual C++ 2008 SP1 Redistributable Package (x86)"
           ExecWait '"$INSTDIR\vcredist_x86.exe"'
           DetailPrint "ready"
           Delete /REBOOTOK "$INSTDIR\vcredist_x86.exe"
       ${EndIf}       
    SectionEnd
    Section Qt SEC0001
        SetOutPath $INSTDIR
        SetOverwrite on
        SetOutPath $INSTDIR
        File "${QT_SRC_DIR}\Qt3Support4.dll"
        File "${QT_SRC_DIR}\QtCore4.dll"
        File "${QT_SRC_DIR}\QtGui4.dll"
        File "${QT_SRC_DIR}\QtNetwork4.dll"
        File "${QT_SRC_DIR}\QtSql4.dll"
        File "${QT_SRC_DIR}\QtXml4.dll"
        WriteRegStr HKLM "${REGKEY}\Components" Complete 1
    SectionEnd

    Section "Qwt 5.2.1" SEC0002   
        SetOutPath $INSTDIR
        SetOverwrite on
        SetOutPath $INSTDIR
        File "${QWT_SRC_DIR}\qwt5.dll"
        WriteRegStr HKLM "${REGKEY}\Components" Complete 1
    SectionEnd
    
SectionGroupEnd

Section Applications SEC0000
    SetShellVarContext all
    
    SetOutPath $INSTDIR
    SetOverwrite on
    #File "${SRC_DIR}bin\DataViewer.bat"
    File "${SRC_DIR}bin\DBeditor.exe"
    #File "${SRC_DIR}bin\SiUSBman.exe"      exluded due to compiling/linking issues
    #File "${SRC_DIR}bin\STcontrol.exe"
    #File "${SRC_DIR}bin\USBPixI4Test.exe"
    File "${SRC_DIR}bin\WaferViewer.bat"
    File "${SRC_DIR}bin\ModuleAnalysis.exe"
    File "${SRC_DIR}bin\WaferAnalysis.exe"
    
    #libraries
    File "${SRC_DIR}bin\SiLibUSB.dll"
    File "${SRC_DIR}bin\usbpixdll.dll"
    File "${SRC_DIR}bin\nclient.dll"
    
    #configuration files
    SetOutPath $INSTDIR
    File "${SRC_DIR}config\CutConfWafer.txt"
    File "${SRC_DIR}config\CutConfModule.txt"
    File "${SRC_DIR}Applications\Pixel\WaferAnalysis\ReadMeForCutConf.txt"
    File "${SRC_DIR}config\SettingsWafer.txt"
    File "${SRC_DIR}config\SettingsModule.txt"
    File "${SRC_DIR}Applications\Pixel\WaferAnalysis\ReadMeForSettings.txt"
    
    #register file type
     ${registerExtension} "$INSTDIR\WaferAnalysis.exe" ".waprj" "WaferAnalysis Project"
    
    #!insertmacro CREATE_SMGROUP_SHORTCUT DataViewer $INSTDIR\DataViewer.bat
    #!insertmacro CREATE_SMGROUP_SHORTCUT STControl $INSTDIR\bin\STcontrol.exe
    #!insertmacro CREATE_SMGROUP_SHORTCUT ModuleAnalysis $INSTDIR\ModuleAnalysis.exe
    #!insertmacro CREATE_SMGROUP_SHORTCUT DBEditor $INSTDIR\DBeditor.exe
    #!insertmacro CREATE_SMGROUP_SHORTCUT USBPixI4Test $INSTDIR\bin\USBPixI4Test.exe
    #!insertmacro CREATE_SMGROUP_SHORTCUT WaferViewer $INSTDIR\WaferViewer.bat
    #!insertmacro CREATE_SMGROUP_SHORTCUT SiUSBManager $INSTDIR\bin\SiUSBman.exe         exluded due to compiling/linking issues
     !insertmacro CREATE_SMGROUP_SHORTCUT WaferAnalysis $INSTDIR\WaferAnalysis.exe
    WriteRegStr HKLM "${REGKEY}\Components" Complete 1
    
    # Remove .mafrc Files from User
    FindFirst $0 $1 "$DOCUMENTS\..\..\*.*"
    loop:
    StrCmp $1 "" done
    IfFileExists "$DOCUMENTS\..\..\$1\.mafrc" DeleteFile NoFile
   DeleteFile:
    DetailPrint "Remove $DOCUMENTS\..\..\$1\.mafrc"
    Delete "$DOCUMENTS\..\..\$1\.mafrc"
   NoFile:
    FindNext $0 $1
    Goto loop
    done:
    FindClose $0
SectionEnd

Section -post SEC0006
    SetShellVarContext all
    WriteRegStr HKLM "${REGKEY}" Path $INSTDIR
    SetOutPath $INSTDIR
    WriteUninstaller $INSTDIR\uninstall.exe
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    SetOutPath $SMPROGRAMS\$StartMenuGroup
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\Uninstall $(^Name).lnk" $INSTDIR\uninstall.exe
    !insertmacro MUI_STARTMENU_WRITE_END
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayName "$(^Name)"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayVersion "${VERSION}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" Publisher "${COMPANY}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" URLInfoAbout "${URL}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayIcon $INSTDIR\uninstall.exe
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" UninstallString $INSTDIR\uninstall.exe
    WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" NoModify 1
    WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" NoRepair 1
SectionEnd

# Macro for selecting uninstaller sections
!macro SELECT_UNSECTION SECTION_NAME UNSECTION_ID
    Push $R0
    ReadRegStr $R0 HKLM "${REGKEY}\Components" "${SECTION_NAME}"
    StrCmp $R0 1 0 next${UNSECTION_ID}
    !insertmacro SelectSection "${UNSECTION_ID}"
    GoTo done${UNSECTION_ID}
next${UNSECTION_ID}:
    !insertmacro UnselectSection "${UNSECTION_ID}"
done${UNSECTION_ID}:
    Pop $R0
!macroend

# Uninstaller sections
!macro DELETE_SMGROUP_SHORTCUT NAME
    Push "${NAME}"
    Call un.DeleteSMGroupShortcut
!macroend

Section /o -un.Driver UNSEC0001
    Delete /REBOOTOK $INSTDIR\driver\*.*
    RmDir /REBOOTOK $INSTDIR\driver
    DeleteRegValue HKLM "${REGKEY}\Components" Driver
SectionEnd

Section /o -un.Applications UNSEC0000
    SetShellVarContext all
    
    #!insertmacro DELETE_SMGROUP_SHORTCUT USBPixI4Test
    !insertmacro DELETE_SMGROUP_SHORTCUT DBEditor
    !insertmacro DELETE_SMGROUP_SHORTCUT ModuleAnalysis
    #!insertmacro DELETE_SMGROUP_SHORTCUT STControl
    #!insertmacro DELETE_SMGROUP_SHORTCUT SiUSBManager       exluded due to compiling/linking issues
    !insertmacro DELETE_SMGROUP_SHORTCUT WaferViewer
    !insertmacro DELETE_SMGROUP_SHORTCUT DataViewer
    !insertmacro DELETE_SMGROUP_SHORTCUT WaferAnalysis
    
    #unregister file type
     ${unregisterExtension} ".waprj" "WaferAnalysis Project"
    
    RmDir /r /REBOOTOK $INSTDIR
    
    MessageBox MB_YESNO "Do you want to keep the config files in the directory ($INSTDIR)?$\r$\n$\r$\nThis might be usefull if you saved your own configurations to this directory." /SD IDYES IDYES deletemain
        Delete /REBOOTOK $INSTDIR\config\*.*
        RmDir /REBOOTOK $INSTDIR\config
deletemain:    
    DeleteRegValue HKLM "${REGKEY}\Components" Applications
SectionEnd

Section -un.post UNSEC0002
    SetShellVarContext all
    
    DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\Uninstall $(^Name).lnk"
    Delete /REBOOTOK $INSTDIR\uninstall.exe
    DeleteRegValue HKLM "${REGKEY}" StartMenuGroup
    DeleteRegValue HKLM "${REGKEY}" Path
    DeleteRegKey /IfEmpty HKLM "${REGKEY}\Components"
    DeleteRegKey /IfEmpty HKLM "${REGKEY}"
    RmDir /REBOOTOK $SMPROGRAMS\$StartMenuGroup
    RmDir $INSTDIR
    Push $R0
    StrCpy $R0 $StartMenuGroup 1
    StrCmp $R0 ">" no_smgroup
no_smgroup:
    Pop $R0
SectionEnd

# Installer functions
Function .onInit
    InitPluginsDir
    #!insertmacro MULTIUSER_INIT
FunctionEnd

Function CreateSMGroupShortcut
    Exch $R0 ;PATH
    Exch
    Exch $R1 ;NAME
    Push $R2
    StrCpy $R2 $StartMenuGroup 1
    StrCmp $R2 ">" no_smgroup
    SetOutPath $SMPROGRAMS\$StartMenuGroup #used to eventually create the StartMenuGroup
    SetOutPath $INSTDIR
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\$R1.lnk" $R0
no_smgroup:
    Pop $R2
    Pop $R1
    Pop $R0
FunctionEnd

# Uninstaller functions
Function un.onInit
    !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuGroup
    #!insertmacro MULTIUSER_UNINIT
    !insertmacro SELECT_UNSECTION Complete ${UNSEC0000}
    !insertmacro SELECT_UNSECTION Driver ${UNSEC0001}
FunctionEnd

Function un.DeleteSMGroupShortcut
    Exch $R1 ;NAME
    Push $R2
    StrCpy $R2 $StartMenuGroup 1
    StrCmp $R2 ">" no_smgroup
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\$R1.lnk"
no_smgroup:
    Pop $R2
    Pop $R1
FunctionEnd

;-------------------------------
; Test if Visual Studio Redistributables 2008 SP1 installed
; Returns -1 if there is no VC redistributables intstalled
Function CheckVCRedist
   StrCpy $VCRedistInstalled 1
   
   ClearErrors
   # SP 1
   ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{9A25302D-30C0-39D9-BD6F-21E6EC160475}" "Version"
   IfErrors 0 VSRedistInstalled
   
   # SP 1 + ATL Security Update
   ClearErrors
   ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{1F1C2DFC-2D24-3E06-BCB8-725134ADF989}" "Version"
   IfErrors 0 VSRedistInstalled
   
   # Runtime not found
   StrCpy $VCRedistInstalled 0

VSRedistInstalled:
FunctionEnd

;-------------------------------
; Test if the included ROOT is newer or not
; Returns 1 if there is no ROOT installed
; Returns 2 if the installed ROOT differs from the included
; Returns 0 if the installed ROOT is the same as the included
Function CheckRootInstalled
   StrCpy $RootInstalled 0
   ClearErrors
   ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{F570A3D8-BC0D-408E-BBE3-57E6DEEE5AAA}" "DisplayVersion"
   IfErrors NoRoot RootInstalled
   
NoRoot:   
   # Root not found
   StrCpy $RootInstalled 1
   DetailPrint "No ROOT found"
   Goto Exit
   
RootInstalled:
   ${If} "$R0" == "${INCLUDED_ROOT_VERSION}"
    StrCpy $RootInstalled 0
   ${Else}
    StrCpy $RootInstalled 2
   ${EndIf}
   DetailPrint "Found ROOT $R0"
   Goto Exit
   
Exit:
FunctionEnd

# Section Descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SEC0000} "Install WaferAnalysis and ModuleAnalysis, DBEditor, DataViewer, WaferViewer."
!insertmacro MUI_DESCRIPTION_TEXT ${SECGRP0000} "Install all needed external libraries. If this option is selected no further Software is required to run the WaferAnalysis Applications."
!insertmacro MUI_DESCRIPTION_TEXT ${SEC0001} "Install Qt 4.6.3 Libraries for use with the WaferAnalysis application. If you deselect this option you will need to install the Qt-Framework (VS 2008) from qt.nokia.com"
!insertmacro MUI_DESCRIPTION_TEXT ${SEC0002} "Install the qwt5.dll for use with the WaferAnalysis Application. If you deselect this option you need to download Qwt and compile it."
!insertmacro MUI_DESCRIPTION_TEXT ${SEC0003} "Install Root ${INCLUDED_ROOT_VERSION} for VC++9. Deselect this option if you want to install ROOT manually. WaferAnalysis needs ROOT."
!insertmacro MUI_DESCRIPTION_TEXT ${SEC0004} "Install Microsoft Visual C++ 2008 SP1 Redistributable Package (x86). This package is needed if you have not installed Visual Studio 2008."
!insertmacro MUI_FUNCTION_DESCRIPTION_END