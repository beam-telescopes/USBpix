# Installation script for USBpix FE-I4
#
# To create a setup basing on this script you need NSIS.
# It can be downloaded at http://nsis.sourceforge.net
#
# The Setup copies all files needed to run the USBpix Applications.
# No further Software Packages are required.
#
# Version History:
# 1.0   3.10.2010   Author: Sebastian Schultes
#   - The links in the Start Menu are created for all users
#   - Only the Qt files used by USBpix are copied
#   - Qt ant Qwt are copied for use with the USBpix Applications only
#   - For Installing ROOT and the VC Runtime the respective Installers are started during the setup 
#
# 1.1   4.11.2010   Author: Sebastian Schultes
#   - Changed for USBpix Release 1.5
#   - USBPix -> USBpix
#
# 2.0   28.3.2011   Author: Sebastian Schultes
#   - Changed for USBpixI4 Release 1.2
#   - USBpix-> USBpixI4
# 2.1   20.6.2011   Author: Sebastian Schultes
#   - Changed for USBpixI4 Release 1.3
# - added some comments
# 2.2   5.12.2011   Author: David-Leon Pohl
# - Changed for USBpixI4 Release 2.0
# - using ROOT 5.32.00 now
# - automatically donwload ROOT from cern ftp side to make the installer smaller (55->10 Mb), via NSIS plugin: Inetc
# 2.3   6.01.2012   Author: David-Leon Pohl
# - Changed for USBpixI4 Release 2.0.5
# 2.4   16.04.2012   Author: David-Leon Pohl
# - Changed for USBpixI4 Release 3.0
# - USBpixTest is not part of the installed programs anymore
# 2.5   20.09.2012   Author: David-Leon Pohl
# - Changed for USBpixI4 Release 3.1
# 2.5   ??.??.2012   Author: David-Leon Pohl
# - Changed for USBpixI4 Release 3.2
# 2.6   21.01.2013   Author: David-Leon Pohl
# - Changed for USBpixI4 Release 3.3: correct ROOT installation is not mandatory anymore, WaferAnalysis is included
# 2.7   05.04.2013   Author: David-Leon Pohl
# - Changed for USBpixI4 Release 4.1
# 2.8   18.04.2013   Author: David-Leon Pohl
# - bug fix: dpinst.exe in 64 bit version is used if windows is 64 bit
#
# 3.0   19.05.2014   Author: Joern Grosse-Knetter
# - Changes for USBpixI4 release 5.0
# - adapted to new software package environment and fixed checks for
#   registry entries for ROOT snd VS-redist.
# 3.1   17.06.2014   Author: Joern Grosse-Knetter
# - bug fix: need to add more parts of QT now that we use QT5 and in addition MS .NET framework
#

Name USBpixI4

SetCompressor /SOLID lzma

# General Symbol Definitions
!define REGKEY "SOFTWARE\$(^Name)"
!define VERSION 5.3.0
!define COMPANY ATLAS-Pixel
!define URL http://icwiki.physik.uni-bonn.de/twiki/bin/view/Systems/UsbPix

# ======================
# Paths to Source Files
# ======================

# Location of the USBpixI4 Repositories (Binaries will be taken from bin subdir)
!define SRC_DIR "C:\Users\jgrosse\USBpix\release-5.3"

# Location of qwt libraries (compiled for release)
!define QWT_SRC_DIR "C:\Qwt\6.1.0\lib"

# Location of Root MSI-Installer
!define ROOT_SRC_DIR "C:\Users\jgrosse\USBpix"

# Location of QT dlls
!define QT_SRC_DIR "C:\Qt\5.x\5.2.1\msvc2012\bin"

# Location of vcredist_x86.exe, just google and download the Visual C++ 2012 Runtime
!define MSVC_SRC_DIR "C:\Users\jgrosse\USBpix"

#Version of root included in this installer
!define INCLUDED_ROOT_VERSION "5.34.18"

# ======================
# End of path section
# ======================

# MUI Symbol Definitions
!define MUI_ICON "${SRC_DIR}\Applications\Pixel\STcontrol\images\GA_Si.ico"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
!define MUI_STARTMENUPAGE_REGISTRY_KEY ${REGKEY}
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME StartMenuGroup
!define MUI_STARTMENUPAGE_DEFAULTFOLDER USBpixI4
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall-colorful.ico"
!define MUI_UNFINISHPAGE_NOAUTOCLOSE
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of USBpix for FE-I4.$\r$\n$\r$\n!!! Please note that a new SiLab USB driver will be installed to use this release. Make sure that you uninstall the old driver BEFORE running this setup. See ReadMe !!!"

# Included files
!include Sections.nsh
!include MUI2.nsh
!include x64.nsh
!include FileAssociation.nsh

# Variables
Var StartMenuGroup
Var VCRedistInstalled
Var MSnetInstalled
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

# Installer languages
!insertmacro MUI_LANGUAGE English

# Installer attributes
OutFile usbpixi4_5_3_0_setup_with_ROOT.exe
InstallDir "C:\USBpixI4"
CRCCheck on
XPStyle on
ShowInstDetails show
VIProductVersion 5.3.0.0
VIAddVersionKey ProductName USBpixI4
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
       MessageBox MB_YESNO "The ROOT Version installed on your System might not work with USBpix!$\r$\nTo install ROOT ${INCLUDED_ROOT_VERSION} your installed ROOT has to be removed. The installer will download the needed version automatially$\r$\n$\r$\nDo you want to change your ROOT version?" /SD IDYES IDYES uninstall_root IDNO exit
    ${EndIf}
    
    # Install ROOT
    Goto install_root
    
    uninstall_root:
    ExecWait 'MsiExec.exe /qr /x{F570A3D8-BC0D-408E-BBE3-57E6DEEE5AAA}'
    DetailPrint "Removed previously installed ROOT"
    Goto install_root
    
    install_root:
        File "${ROOT_SRC_DIR}\root_v${INCLUDED_ROOT_VERSION}.win32.vc11.msi"
    DetailPrint "Installing ${INCLUDED_ROOT_VERSION}"
    ExecWait 'msiexec /i "$INSTDIR\root_v${INCLUDED_ROOT_VERSION}.win32.vc11.msi"'
    DetailPrint "Root ${INCLUDED_ROOT_VERSION} installed"
    Delete /REBOOTOK "$INSTDIR\root_v${INCLUDED_ROOT_VERSION}.win32.vc11.msi"
    Goto exit
    
    exit:
    SectionEnd
    
    Section "Visual C++ 2012 Runtime" SEC0004
        Call CheckVCRedist
        ${If} $VCRedistInstalled == 1
        DetailPrint "VC Runtime is already installed."
        ${Else}
           SetOutPath $INSTDIR
           SetOverwrite on
           File "${MSVC_SRC_DIR}\vcredist_x86.exe"

           #run runtime installation tool
           DetailPrint "Installing Microsoft Visual C++ 2012 Update4 Redistributable Package (x86)"
           ExecWait '"$INSTDIR\vcredist_x86.exe"'
           DetailPrint "ready"
           Delete /REBOOTOK "$INSTDIR\vcredist_x86.exe"
       ${EndIf}       
    SectionEnd
    Section "Microsoft .NET Framework 4" SEC0007
        Call CheckMSNET
        ${If} $MSnetInstalled == 1
        DetailPrint "Microsoft .NET Framework 4 is already installed."
        ${Else}
           SetOutPath $INSTDIR
           SetOverwrite on
           File "${MSVC_SRC_DIR}\dotNetFx40_Full_x86_x64.exe"

           #run runtime installation tool
           DetailPrint "Installing Microsoft .NET Framework 4"
           ExecWait '"$INSTDIR\dotNetFx40_Full_x86_x64.exe"'
           DetailPrint "ready"
           Delete /REBOOTOK "$INSTDIR\dotNetFx40_Full_x86_x64.exe"
        ${EndIf}       
    SectionEnd

    Section Qt SEC0001
        SetOutPath $INSTDIR
        SetOverwrite on
        SetOutPath $INSTDIR\bin
        File "${QT_SRC_DIR}\Qt5Core.dll"
        File "${QT_SRC_DIR}\Qt5Widgets.dll"
        File "${QT_SRC_DIR}\Qt5Gui.dll"
        File "${QT_SRC_DIR}\Qt5Concurrent.dll"
        File "${QT_SRC_DIR}\Qt5Network.dll"
        File "${QT_SRC_DIR}\Qt5OpenGL.dll"
        File "${QT_SRC_DIR}\Qt5Svg.dll"
        File "${QT_SRC_DIR}\Qt5PrintSupport.dll"
        File "${QT_SRC_DIR}\icudt51.dll"
        File "${QT_SRC_DIR}\icuin51.dll"
        File "${QT_SRC_DIR}\icuuc51.dll"
        File "${QT_SRC_DIR}\libEGL.dll"
        File "${QT_SRC_DIR}\libGLESv2.dll"
        SetOutPath $INSTDIR\bin\platforms
        File "${QT_SRC_DIR}\..\plugins\platforms\*.*"
        WriteRegStr HKLM "${REGKEY}\Components" Complete 1
    SectionEnd

    Section "Qwt 6.1.0" SEC0002   
        SetOutPath $INSTDIR
        SetOverwrite on
        SetOutPath $INSTDIR\bin
        File "${QWT_SRC_DIR}\qwt.dll"
        WriteRegStr HKLM "${REGKEY}\Components" Complete 1
    SectionEnd
    
SectionGroupEnd

Section Applications SEC0000
    SetShellVarContext all
    
    SetOutPath $INSTDIR\bin
    SetOverwrite on
    File "${SRC_DIR}\bin\DataViewer.exe"
    File "${SRC_DIR}\bin\DBeditor.exe"
    File "${SRC_DIR}\bin\SiUSBman.exe"
    File "${SRC_DIR}\bin\STcontrol.exe"
    #File "${SRC_DIR}\bin\USBPixI4Test.exe"       excluded: needs upgrade first
    File "${SRC_DIR}\bin\WaferAnalysis.exe"
    File "${SRC_DIR}\bin\WaferViewer.bat"
    
    #libraries
    File "${SRC_DIR}\bin\PixGPIB.dll"
    File "${SRC_DIR}\bin\PixRS232.dll"
    File "${SRC_DIR}\bin\SiLibUSB.dll"
    File "${SRC_DIR}\bin\Suess.dll"
    File "${SRC_DIR}\bin\usbpixi3dll.dll"
    File "${SRC_DIR}\bin\usbpixi4dll.dll"
    File "${SRC_DIR}\bin\gpac.dll"
    #File "${SRC_DIR}\bin\nclient.dll"
    
    #configuration files
    SetOutPath $INSTDIR\config
    File "${SRC_DIR}\config\usbpix_FW_15.bix"
    File "${SRC_DIR}\config\usbpix_FW_15.hex"
    File "${SRC_DIR}\config\usbpixi4.bit"
    
    !insertmacro CREATE_SMGROUP_SHORTCUT DataViewer $INSTDIR\bin\DataViewer.exe
    !insertmacro CREATE_SMGROUP_SHORTCUT DBEditor $INSTDIR\bin\DBeditor.exe
    !insertmacro CREATE_SMGROUP_SHORTCUT SiUSBManager $INSTDIR\bin\SiUSBman.exe
    !insertmacro CREATE_SMGROUP_SHORTCUT STControl $INSTDIR\bin\STcontrol.exe
    #!insertmacro CREATE_SMGROUP_SHORTCUT USBPixI4Test $INSTDIR\bin\USBPixI4Test.exe     excluded: needs upgrade first
    !insertmacro CREATE_SMGROUP_SHORTCUT WaferViewer $INSTDIR\bin\WaferViewer.bat
    !insertmacro CREATE_SMGROUP_SHORTCUT WaferAnalysis $INSTDIR\bin\WaferAnalysis.exe
    WriteRegStr HKLM "${REGKEY}\Components" Complete 1
    
    #register file type
    ${registerExtension} "$INSTDIR\WaferAnalysis.exe" ".waprj" "WaferAnalysis Project"
    
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

Section "Multi IO Board 32/64-bit Driver" SEC0005
    SetOutPath $INSTDIR\driver
    SetOverwrite on
    File "${SRC_DIR}\driver\win\*.*"
    SetOutPath $INSTDIR\driver\amd64
    SetOverwrite on
    File "${SRC_DIR}\driver\win\amd64\*.*"
    SetOutPath $INSTDIR\driver\x86
    SetOverwrite on
    File "${SRC_DIR}\driver\win\x86\*.*"
    WriteRegStr HKLM "${REGKEY}\Components" Driver 1
  
    #run driver installation tool
    DetailPrint "Installing 32/64-bit Device Driver"
    IfFileExists $WINDIR\SYSWOW64\*.* Is64bit Is32bit   #check if 64/32 bit installation of dpinst should be called
    Is32bit:
        ExecWait '"$INSTDIR\driver\InstallDriver.exe"' $0
        goto exit
    Is64bit:
        ExecWait '"$INSTDIR\driver\InstallDriver64.exe"' $0
    exit:
    DetailPrint "Device Driver installed with return value $0"
SectionEnd

Section -post SEC0006
    SetShellVarContext all
    WriteRegStr HKLM "${REGKEY}" Path $INSTDIR
    SetOutPath $INSTDIR
    WriteUninstaller $INSTDIR\usbpixI4_uninstall.exe
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    SetOutPath $SMPROGRAMS\$StartMenuGroup
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\Uninstall $(^Name).lnk" $INSTDIR\usbpixI4_uninstall.exe
    !insertmacro MUI_STARTMENU_WRITE_END
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayName "$(^Name)"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayVersion "${VERSION}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" Publisher "${COMPANY}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" URLInfoAbout "${URL}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayIcon $INSTDIR\usbpixI4_uninstall.exe
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" UninstallString $INSTDIR\usbpixI4_uninstall.exe
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
    
    !insertmacro DELETE_SMGROUP_SHORTCUT DataViewer
    !insertmacro DELETE_SMGROUP_SHORTCUT DBEditor
    !insertmacro DELETE_SMGROUP_SHORTCUT STControl
    !insertmacro DELETE_SMGROUP_SHORTCUT SiUSBManager 
    #!insertmacro DELETE_SMGROUP_SHORTCUT USBPixI4Test
    !insertmacro DELETE_SMGROUP_SHORTCUT WaferViewer
    !insertmacro DELETE_SMGROUP_SHORTCUT WaferAnalysis
    
    #unregister file type
    ${unregisterExtension} ".waprj" "WaferAnalysis Project"
    
    RmDir /r /REBOOTOK $INSTDIR\bin
    
    MessageBox MB_YESNO "Do you want to keep the files in the config directory ($INSTDIR\config)?$\r$\n$\r$\nThis might be usefull if you saved your own configurations to this directory." /SD IDYES IDYES deletemain
        Delete /REBOOTOK $INSTDIR\config\*.*
        RmDir /REBOOTOK $INSTDIR\config
deletemain:    
    DeleteRegValue HKLM "${REGKEY}\Components" Applications
SectionEnd

Section -un.post UNSEC0002
    SetShellVarContext all
    
    DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\Uninstall $(^Name).lnk"
    Delete /REBOOTOK $INSTDIR\usbpixI4_uninstall.exe
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
    SetOutPath $INSTDIR\bin
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
; Test if Visual Studio Redistributables 2012 Update4 installed
; Returns 0 if there is no VC redistributables intstalled
Function CheckVCRedist
   StrCpy $VCRedistInstalled 1
   
   ClearErrors
   IfFileExists $WINDIR\SYSWOW64\*.* Is64bit Is32bit   #check if 64/32 bit system to fix registry reading
   Is32bit:
     SetRegView 32
     ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{BD95A8CD-1D9F-35AD-981A-3E7925026EBB}" "DisplayVersion"
     DetailPrint "Installed VC Runtime is $R0"
     IfErrors 0 VSRedistInstalled
   Is64bit:
     SetRegView 64
     ReadRegStr $R0 HKLM "SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{BD95A8CD-1D9F-35AD-981A-3E7925026EBB}" "DisplayVersion"
     DetailPrint "Installed VC Runtime is $R0"
     IfErrors 0 VSRedistInstalled
   # Runtime not found
   StrCpy $VCRedistInstalled 0

VSRedistInstalled:
FunctionEnd

;-------------------------------
; Test if MS .NET framwork 4 is installed
; Returns 0 if there is no MS .NET is installed
Function CheckMSNET
   StrCpy $MSnetInstalled 1
   
   ClearErrors
   IfFileExists $WINDIR\SYSWOW64\*.* Is64bit Is32bit   #check if 64/32 bit system to fix registry reading
   Is32bit:
     SetRegView 32
     goto no64bit
   Is64bit:
     SetRegView 64
   no64bit:
   # check registry entry
   ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Client\1033" "Version"
   DetailPrint "Installed MS .NET version is $R0"
   IfErrors 0 MSnetInstalled
   
   # MS NET not found
   StrCpy $MSnetInstalled 0

MSnetInstalled:
FunctionEnd

;-------------------------------
; Test if the included ROOT is newer or not
; Returns 1 if there is no ROOT installed
; Returns 2 if the installed ROOT differs from the included
; Returns 0 if the installed ROOT is the same as the included
Function CheckRootInstalled
   StrCpy $RootInstalled 0
   
   ClearErrors
   IfFileExists $WINDIR\SYSWOW64\*.* Is64bit Is32bit   #check if 64/32 bit system to fix registry reading
   Is32bit:
     SetRegView 32
     ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{F570A3D8-BC0D-408E-BBE3-57E6DEEE5AAA}" "DisplayVersion"
     IfErrors NoRoot RootInstalled
   Is64bit:
     SetRegView 64
     ReadRegStr $R0 HKLM "SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{F570A3D8-BC0D-408E-BBE3-57E6DEEE5AAA}" "DisplayVersion"
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
!insertmacro MUI_DESCRIPTION_TEXT ${SEC0000} "Install STControl, DBEditor, DataViewer, WaferViewer and WaferAnalysis"
!insertmacro MUI_DESCRIPTION_TEXT ${SEC0005} "Install the 32/64 bit Driver for the S3-Multi-IO-Board"
!insertmacro MUI_DESCRIPTION_TEXT ${SECGRP0000} "Install all needed external libraries. If this option is selected no further Software is required to run the USBpix Applications."
!insertmacro MUI_DESCRIPTION_TEXT ${SEC0001} "Install QT libraries for use with the USBpix Applications. If you deselect this option you will need to install the Qt-Framework (VS 2012)"
!insertmacro MUI_DESCRIPTION_TEXT ${SEC0002} "Install the qwt.dll for use with the USBpix Applications. If you deselect this option you need to download Qwt and compile it."
!insertmacro MUI_DESCRIPTION_TEXT ${SEC0003} "Install Root ${INCLUDED_ROOT_VERSION} for VC++11. Deselect this option if you want to install ROOT manually. USBpix needs ROOT ${INCLUDED_ROOT_VERSION} built with VS 2012."
!insertmacro MUI_DESCRIPTION_TEXT ${SEC0004} "Install Microsoft Visual C++ 2012 Redistributable Package (x86). This package is needed if you have not installed Visual Studio 2012."
!insertmacro MUI_DESCRIPTION_TEXT ${SEC0007} "Install Microsoft .NET Framework 4 (x86). This package is needed if you have not installed Visual Studio 2012."
!insertmacro MUI_FUNCTION_DESCRIPTION_END
