# Microsoft Developer Studio Project File - Name="ModuleAnalysis" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ModuleAnalysis - Win32 Debug with PixLib
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ModuleAnalysis.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ModuleAnalysis.mak" CFG="ModuleAnalysis - Win32 Debug with PixLib"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ModuleAnalysis - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ModuleAnalysis - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "ModuleAnalysis - Win32 Debug with PixLib" (based on "Win32 (x86) Application")
!MESSAGE "ModuleAnalysis - Win32 Release with PixLib" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "tmp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /GR /GX /O1 /I "$(ROOTSYS)\include" /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /D "NO_DEBUG" /D "WIN32" /D "NDEBUG" /D "UNICODE" /D "QT_NO_DEBUG" /D "COMPL" /FD -Zm200 /c
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 delayimp.lib "qt-mt323.lib" "qtmain.lib" "libCore.lib" "libCint.lib" "libHist.lib" "libHistPainter.lib" "libGraf.lib" "libGraf3d.lib" "libGpad.lib" "libTree.lib" "libRint.lib" "libPostscript.lib" "libMatrix.lib" "libPhysics.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" /nologo /subsystem:windows /machine:IX86 /libpath:"$(QTDIR)\lib" /libpath:"$(ROOTSYS)\lib"

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "tmp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MDd /W3 /GR /GX /Z7 /Od /I "$(ROOTSYS)\include" /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /D "COMPL" /FD /GZ -Zm200 /c
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 "qt-mt323.lib" "qtmain.lib" "libCore.lib" "libCint.lib" "libHist.lib" "libHistPainter.lib" "libGraf.lib" "libGraf3d.lib" "libGpad.lib" "libTree.lib" "libRint.lib" "libPostscript.lib" "libMatrix.lib" "libPhysics.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" /nologo /subsystem:windows /debug /machine:IX86 /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"$(ROOTSYS)\lib"

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ModuleAnalysis___Win32_Debug_with_PixLib"
# PROP BASE Intermediate_Dir "ModuleAnalysis___Win32_Debug_with_PixLib"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "tmp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GR /GX /Z7 /Od /I "$(ROOTSYS)\include" /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /D "COMPL" /FD /GZ -Zm200 /c
# ADD CPP /nologo /MDd /W3 /GR /GX /Z7 /Od /I "$(ROOTSYS)\include" /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /I "$(PIX_LIB)\PixConfDBInterface" /I "$(PIX_LIB)\Bits" /I "$(PIX_LIB)\Histo" /I "$(PIX_LIB)" /I "$(VME_INTERFACE)" /I "$(PIX_LIB)/PixController" /I "$(ROD_DAQ)/CommonWithDsp" /I "$(ROD_DAQ)/CommonWithDsp/Pixel" /I "$(ROD_DAQ)/Dsp/Code" /I "$(ROD_DAQ)/RodCrate" /I "$(PIX_ANA)\DataContainer" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /D "WIN32" /D "COMPL" /D "HAVE_PLDB" /D "PIXEL_ROD" /D "TSTAMP" /D "I_AM_LINUX_HOST" /FD /GZ -Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 "qt-mt323.lib" "qtmain.lib" "libCore.lib" "libCint.lib" "libHist.lib" "libGraf.lib" "libGraf3d.lib" "libGpad.lib" "libTree.lib" "libRint.lib" "libPostscript.lib" "libMatrix.lib" "libPhysics.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" /nologo /subsystem:windows /debug /machine:IX86 /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"$(ROOTSYS)\lib"
# ADD LINK32 "PixLib.lib" "BaseException.lib" "PixAnalysis.lib" "qt-mt323.lib" "qtmain.lib" "libCore.lib" "libCint.lib" "libHist.lib" "libHistPainter.lib" "libGraf.lib" "libGraf3d.lib" "libGpad.lib" "libTree.lib" "libRint.lib" "libPostscript.lib" "libMatrix.lib" "libPhysics.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" /nologo /subsystem:windows /debug /machine:IX86 /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"$(ROOTSYS)\lib" /libpath:"$(PIX_LIB)" /libpath:"$(VME_INTERFACE)" /libpath:"$(PIX_ANA)"

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ModuleAnalysis___Win32_Release_with_PixLib"
# PROP BASE Intermediate_Dir "ModuleAnalysis___Win32_Release_with_PixLib"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "tmp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O1 /I "$(ROOTSYS)\include" /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /D "NO_DEBUG" /D "WIN32" /D "NDEBUG" /D "UNICODE" /D "QT_NO_DEBUG" /D "COMPL" /FD -Zm200 /c
# ADD CPP /nologo /MD /W3 /GR /GX /O1 /I "$(ROOTSYS)\include" /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /I "$(PIX_LIB)\PixConfDBInterface" /I "$(PIX_LIB)\Bits" /I "$(PIX_LIB)\Histo" /I "$(PIX_LIB)" /I "$(VME_INTERFACE)" /I "$(PIX_LIB)/PixController" /I "$(ROD_DAQ)/CommonWithDsp" /I "$(ROD_DAQ)/CommonWithDsp/Pixel" /I "$(ROD_DAQ)/Dsp/Code" /I "$(ROD_DAQ)/RodCrate" /I "$(PIX_ANA)\DataContainer" /D "NO_DEBUG" /D "NDEBUG" /D "UNICODE" /D "QT_NO_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /D "WIN32" /D "COMPL" /D "HAVE_PLDB" /D "PIXEL_ROD" /D "TSTAMP" /D "I_AM_LINUX_HOST" /FD -Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 delayimp.lib "qt-mt323.lib" "qtmain.lib" "libCore.lib" "libCint.lib" "libHist.lib" "libGraf.lib" "libGraf3d.lib" "libGpad.lib" "libTree.lib" "libRint.lib" "libPostscript.lib" "libMatrix.lib" "libPhysics.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" /nologo /subsystem:windows /machine:IX86 /libpath:"$(QTDIR)\lib" /libpath:"$(ROOTSYS)\lib"
# ADD LINK32 delayimp.lib "PixLib.lib" "BaseException.lib" "PixAnalysis.lib" "qt-mt323.lib" "qtmain.lib" "libCore.lib" "libCint.lib" "libHist.lib" "libHistPainter.lib" "libGraf.lib" "libGraf3d.lib" "libGpad.lib" "libTree.lib" "libRint.lib" "libPostscript.lib" "libMatrix.lib" "libPhysics.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" /nologo /subsystem:windows /machine:IX86 /libpath:"$(QTDIR)\lib" /libpath:"$(ROOTSYS)\lib" /libpath:"$(PIX_LIB)" /libpath:"$(VME_INTERFACE)" /libpath:"$(PIX_ANA)"

!ENDIF 

# Begin Target

# Name "ModuleAnalysis - Win32 Release"
# Name "ModuleAnalysis - Win32 Debug"
# Name "ModuleAnalysis - Win32 Debug with PixLib"
# Name "ModuleAnalysis - Win32 Release with PixLib"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=BadPixel.cpp
# End Source File
# Begin Source File

SOURCE=.\CDatabase.cpp
# End Source File
# Begin Source File

SOURCE=.\CfgForm.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigViewer.cpp

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=CorrelSel.cpp
# End Source File
# Begin Source File

SOURCE=CutPanel.cpp
# End Source File
# Begin Source File

SOURCE=DataStuff.cpp
# End Source File
# Begin Source File

SOURCE=.\DBtoMAData.cxx

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=FileWin.cpp
# End Source File
# Begin Source File

SOURCE=.\fitPixel.cpp
# End Source File
# Begin Source File

SOURCE=.\fittingRoutines.cpp
# End Source File
# Begin Source File

SOURCE=FitWin.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadStave.cpp
# End Source File
# Begin Source File

SOURCE=LogClasses.cpp
# End Source File
# Begin Source File

SOURCE=LogDisp.cpp
# End Source File
# Begin Source File

SOURCE=LogRoot.cpp
# End Source File
# Begin Source File

SOURCE=.\MAEngine.cpp
# End Source File
# Begin Source File

SOURCE=main.cpp
# End Source File
# Begin Source File

SOURCE=MaskedSel.cpp
# End Source File
# Begin Source File

SOURCE=ModTree.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiLoad.cpp
# End Source File
# Begin Source File

SOURCE=OptClass.cpp
# End Source File
# Begin Source File

SOURCE=OptWin.cpp
# End Source File
# Begin Source File

SOURCE=.\Overviewselect.cpp
# End Source File
# Begin Source File

SOURCE=.\PDBForm.cpp
# End Source File
# Begin Source File

SOURCE=QAReport.cpp

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# ADD CPP /I "$(PIX_ANA)\DataContainer"

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# ADD CPP /I "$(PIX_ANA)\DataContainer"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=QRootApplication.cpp
# End Source File
# Begin Source File

SOURCE=RefWin.cpp
# End Source File
# Begin Source File

SOURCE=.\RegPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\RootDataList.cpp
# End Source File
# Begin Source File

SOURCE=RootStuff.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectionPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\StdListChoice.cpp
# End Source File
# Begin Source File

SOURCE=StdTestWin.cpp
# End Source File
# Begin Source File

SOURCE=TopWin.cpp
# End Source File
# Begin Source File

SOURCE=.\TotRecal.cpp
# End Source File
# Begin Source File

SOURCE=.\Twodsel.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=BadPixel.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing BadPixel.h...
InputPath=BadPixel.h

"moc_BadPixel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc BadPixel.h -o moc_BadPixel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing BadPixel.h...
InputPath=BadPixel.h

"moc_BadPixel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc BadPixel.h -o moc_BadPixel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing BadPixel.h...
InputPath=BadPixel.h

"moc_BadPixel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc BadPixel.h -o moc_BadPixel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing BadPixel.h...
InputPath=BadPixel.h

"moc_BadPixel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc BadPixel.h -o moc_BadPixel.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CDatabase.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing CDatabase.h...
InputPath=.\CDatabase.h

"moc_CDatabase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CDatabase.h -o moc_CDatabase.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing CDatabase.h...
InputPath=.\CDatabase.h

"moc_CDatabase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CDatabase.h -o moc_CDatabase.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing CDatabase.h...
InputPath=.\CDatabase.h

"moc_CDatabase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CDatabase.h -o moc_CDatabase.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing CDatabase.h...
InputPath=.\CDatabase.h

"moc_CDatabase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CDatabase.h -o moc_CDatabase.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CfgForm.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing CfgForm.h...
InputPath=.\CfgForm.h

"moc_CfgForm.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CfgForm.h -o moc_CfgForm.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing CfgForm.h...
InputPath=.\CfgForm.h

"moc_CfgForm.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CfgForm.h -o moc_CfgForm.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing CfgForm.h...
InputPath=.\CfgForm.h

"moc_CfgForm.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CfgForm.h -o moc_CfgForm.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing CfgForm.h...
InputPath=.\CfgForm.h

"moc_CfgForm.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CfgForm.h -o moc_CfgForm.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ConfigViewer.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing ConfigViewer...
InputPath=.\ConfigViewer.h

"moc_ConfigViewer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ConfigViewer.h -o moc_ConfigViewer.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing ConfigViewer...
InputPath=.\ConfigViewer.h

"moc_ConfigViewer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ConfigViewer.h -o moc_ConfigViewer.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=CorrelSel.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__CORRE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing CorrelSel.h...
InputPath=CorrelSel.h

"moc_CorrelSel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CorrelSel.h -o moc_CorrelSel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__CORRE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing CorrelSel.h...
InputPath=CorrelSel.h

"moc_CorrelSel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CorrelSel.h -o moc_CorrelSel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__CORRE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing CorrelSel.h...
InputPath=CorrelSel.h

"moc_CorrelSel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CorrelSel.h -o moc_CorrelSel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__CORRE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing CorrelSel.h...
InputPath=CorrelSel.h

"moc_CorrelSel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CorrelSel.h -o moc_CorrelSel.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=CutPanel.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__CUTPA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing CutPanel.h...
InputPath=CutPanel.h

"moc_CutPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CutPanel.h -o moc_CutPanel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__CUTPA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing CutPanel.h...
InputPath=CutPanel.h

"moc_CutPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CutPanel.h -o moc_CutPanel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__CUTPA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing CutPanel.h...
InputPath=CutPanel.h

"moc_CutPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CutPanel.h -o moc_CutPanel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__CUTPA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing CutPanel.h...
InputPath=CutPanel.h

"moc_CutPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc CutPanel.h -o moc_CutPanel.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=DataStuff.h
# End Source File
# Begin Source File

SOURCE=FileTypes.h
# End Source File
# Begin Source File

SOURCE=FileWin.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__FILEW="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing FileWin.h...
InputPath=FileWin.h

"moc_FileWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc FileWin.h -o moc_FileWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__FILEW="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing FileWin.h...
InputPath=FileWin.h

"moc_FileWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc FileWin.h -o moc_FileWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__FILEW="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing FileWin.h...
InputPath=FileWin.h

"moc_FileWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc FileWin.h -o moc_FileWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__FILEW="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing FileWin.h...
InputPath=FileWin.h

"moc_FileWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc FileWin.h -o moc_FileWin.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=FitWin.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__FITWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing FitWin.h...
InputPath=FitWin.h

"moc_FitWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc FitWin.h -o moc_FitWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__FITWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing FitWin.h...
InputPath=FitWin.h

"moc_FitWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc FitWin.h -o moc_FitWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__FITWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing FitWin.h...
InputPath=FitWin.h

"moc_FitWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc FitWin.h -o moc_FitWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__FITWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing FitWin.h...
InputPath=FitWin.h

"moc_FitWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc FitWin.h -o moc_FitWin.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LinkDef.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__LINKD="LogClasses.h"	"LogRoot.h"	
# Begin Custom Build - Generating logfile root dictionary...
InputPath=.\LinkDef.h

BuildCmds= \
	$(ROOTSYS)\bin\rootcint -f dictLogRoot.cpp -c LogClasses.h LogRoot.h LinkDef.h

"dictLogRoot.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"dictLogRoot.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__LINKD="LogClasses.h"	"LogRoot.h"	
# Begin Custom Build - Generating root dictionary...
InputPath=.\LinkDef.h

BuildCmds= \
	$(ROOTSYS)\bin\rootcint -f dictLogRoot.cpp -c LogClasses.h LogRoot.h LinkDef.h

"dictLogRoot.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"dictLogRoot.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__LINKD="LogClasses.h"	"LogRoot.h"	
# Begin Custom Build - Generating root dictionary...
InputPath=.\LinkDef.h

BuildCmds= \
	$(ROOTSYS)\bin\rootcint -f dictLogRoot.cpp -c LogClasses.h LogRoot.h LinkDef.h

"dictLogRoot.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"dictLogRoot.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__LINKD="LogClasses.h"	"LogRoot.h"	
# Begin Custom Build - Generating logfile root dictionary...
InputPath=.\LinkDef.h

BuildCmds= \
	$(ROOTSYS)\bin\rootcint -f dictLogRoot.cpp -c LogClasses.h LogRoot.h LinkDef.h

"dictLogRoot.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"dictLogRoot.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LoadStave.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing LoadSTave.h...
InputPath=.\LoadStave.h

"moc_LoadStave.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc LoadStave.h -o moc_LoadStave.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing LoadSTave.h...
InputPath=.\LoadStave.h

"moc_LoadStave.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc LoadStave.h -o moc_LoadStave.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing LoadSTave.h...
InputPath=.\LoadStave.h

"moc_LoadStave.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc LoadStave.h -o moc_LoadStave.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing LoadSTave.h...
InputPath=.\LoadStave.h

"moc_LoadStave.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc LoadStave.h -o moc_LoadStave.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LogClasses.h
# End Source File
# Begin Source File

SOURCE=.\LogDisp.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing LogDisp.h...
InputPath=.\LogDisp.h

"moc_LogDisp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc LogDisp.h -o moc_LogDisp.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing LogDisp.h...
InputPath=.\LogDisp.h

"moc_LogDisp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc LogDisp.h -o moc_LogDisp.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing LogDisp.h...
InputPath=.\LogDisp.h

"moc_LogDisp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc LogDisp.h -o moc_LogDisp.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing LogDisp.h...
InputPath=.\LogDisp.h

"moc_LogDisp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc LogDisp.h -o moc_LogDisp.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=LogRoot.h
# End Source File
# Begin Source File

SOURCE=.\MAEngine.h
# End Source File
# Begin Source File

SOURCE=MaskedSel.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing MaskedSel.h...
InputPath=MaskedSel.h

"moc_MaskedSel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc MaskedSel.h -o moc_MaskedSel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing MaskedSel.h...
InputPath=MaskedSel.h

"moc_MaskedSel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc MaskedSel.h -o moc_MaskedSel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing MaskedSel.h...
InputPath=MaskedSel.h

"moc_MaskedSel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc MaskedSel.h -o moc_MaskedSel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing MaskedSel.h...
InputPath=MaskedSel.h

"moc_MaskedSel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc MaskedSel.h -o moc_MaskedSel.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MATPalette.h
# End Source File
# Begin Source File

SOURCE=ModTree.h
# End Source File
# Begin Source File

SOURCE=.\MultiLoad.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing MultiLoad.h...
InputPath=.\MultiLoad.h

"moc_MultiLoad.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc MultiLoad.h -o moc_MultiLoad.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing MultiLoad.h...
InputPath=.\MultiLoad.h

"moc_MultiLoad.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc MultiLoad.h -o moc_MultiLoad.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing MultiLoad.h...
InputPath=.\MultiLoad.h

"moc_MultiLoad.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc MultiLoad.h -o moc_MultiLoad.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing MultiLoad.h...
InputPath=.\MultiLoad.h

"moc_MultiLoad.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc MultiLoad.h -o moc_MultiLoad.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=OptClass.h
# End Source File
# Begin Source File

SOURCE=OptWin.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__OPTWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing OptWin.h...
InputPath=OptWin.h

"moc_OptWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc OptWin.h -o moc_OptWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__OPTWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing OptWin.h...
InputPath=OptWin.h

"moc_OptWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc OptWin.h -o moc_OptWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__OPTWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing OptWin.h...
InputPath=OptWin.h

"moc_OptWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc OptWin.h -o moc_OptWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__OPTWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing OptWin.h...
InputPath=OptWin.h

"moc_OptWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc OptWin.h -o moc_OptWin.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Overviewselect.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing Overviewselect.h...
InputPath=.\Overviewselect.h

"moc_Overviewselect.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc Overviewselect.h -o moc_Overviewselect.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing Overviewselect.h...
InputPath=.\Overviewselect.h

"moc_Overviewselect.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc Overviewselect.h -o moc_Overviewselect.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing Overviewselect.h...
InputPath=.\Overviewselect.h

"moc_Overviewselect.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc Overviewselect.h -o moc_Overviewselect.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing Overviewselect.h...
InputPath=.\Overviewselect.h

"moc_Overviewselect.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc Overviewselect.h -o moc_Overviewselect.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\paletteLinkDef.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Creating MATPalette dictionary...
InputPath=.\paletteLinkDef.h

BuildCmds= \
	$(ROOTSYS)\bin\rootcint -f dictPalette.cpp -c MATPalette.h paletteLinkDef.h

"dictPalette.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"dictPalette.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Creating MATPalette dictionary...
InputPath=.\paletteLinkDef.h

BuildCmds= \
	$(ROOTSYS)\bin\rootcint -f dictPalette.cpp -c MATPalette.h paletteLinkDef.h

"dictPalette.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"dictPalette.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Creating MATPalette dictionary...
InputPath=.\paletteLinkDef.h

BuildCmds= \
	$(ROOTSYS)\bin\rootcint -f dictPalette.cpp -c MATPalette.h paletteLinkDef.h

"dictPalette.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"dictPalette.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Creating MATPalette dictionary...
InputPath=.\paletteLinkDef.h

BuildCmds= \
	$(ROOTSYS)\bin\rootcint -f dictPalette.cpp -c MATPalette.h paletteLinkDef.h

"dictPalette.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"dictPalette.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PDBForm.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing PDBForm.h...
InputPath=.\PDBForm.h

"moc_PDBForm.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc PDBForm.h -o moc_PDBForm.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing PDBForm.h...
InputPath=.\PDBForm.h

"moc_PDBForm.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc PDBForm.h -o moc_PDBForm.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing PDBForm.h...
InputPath=.\PDBForm.h

"moc_PDBForm.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc PDBForm.h -o moc_PDBForm.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing PDBForm.h...
InputPath=.\PDBForm.h

"moc_PDBForm.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc PDBForm.h -o moc_PDBForm.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\QAReport.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing QAReport.h...
InputPath=.\QAReport.h

"moc_QAReport.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc QAReport.h -o moc_QAReport.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing QAReport.h...
InputPath=.\QAReport.h

"moc_QAReport.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc QAReport.h -o moc_QAReport.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing QAReport.h...
InputPath=.\QAReport.h

"moc_QAReport.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc QAReport.h -o moc_QAReport.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing QAReport.h...
InputPath=.\QAReport.h

"moc_QAReport.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc QAReport.h -o moc_QAReport.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=QRootApplication.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__QROOT="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing QRootApplication.h...
InputPath=QRootApplication.h

"moc_QRootApplication.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc QRootApplication.h -o moc_QRootApplication.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__QROOT="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing QRootApplication.h...
InputPath=QRootApplication.h

"moc_QRootApplication.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc QRootApplication.h -o moc_QRootApplication.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__QROOT="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing QRootApplication.h...
InputPath=QRootApplication.h

"moc_QRootApplication.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc QRootApplication.h -o moc_QRootApplication.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__QROOT="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing QRootApplication.h...
InputPath=QRootApplication.h

"moc_QRootApplication.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc QRootApplication.h -o moc_QRootApplication.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=RefWin.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__REFWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing RefWin.h...
InputPath=RefWin.h

"moc_RefWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc RefWin.h -o moc_RefWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__REFWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing RefWin.h...
InputPath=RefWin.h

"moc_RefWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc RefWin.h -o moc_RefWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__REFWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing RefWin.h...
InputPath=RefWin.h

"moc_RefWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc RefWin.h -o moc_RefWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__REFWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing RefWin.h...
InputPath=RefWin.h

"moc_RefWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc RefWin.h -o moc_RefWin.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RegPanel.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing RegPanel.h...
InputPath=.\RegPanel.h

"moc_RegPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc RegPanel.h -o moc_RegPanel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing RegPanel.h...
InputPath=.\RegPanel.h

"moc_RegPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc RegPanel.h -o moc_RegPanel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing RegPanel.h...
InputPath=.\RegPanel.h

"moc_RegPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc RegPanel.h -o moc_RegPanel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing RegPanel.h...
InputPath=.\RegPanel.h

"moc_RegPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc RegPanel.h -o moc_RegPanel.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RootDataList.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing RootDataList.h...
InputPath=.\RootDataList.h

"moc_RootDataList.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc RootDataList.h -o moc_RootDataList.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing RootDataList.h...
InputPath=.\RootDataList.h

"moc_RootDataList.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc RootDataList.h -o moc_RootDataList.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing RootDataList.h...
InputPath=.\RootDataList.h

"moc_RootDataList.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc RootDataList.h -o moc_RootDataList.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing RootDataList.h...
InputPath=.\RootDataList.h

"moc_RootDataList.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc RootDataList.h -o moc_RootDataList.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=RootStuff.h
# End Source File
# Begin Source File

SOURCE=.\SelectionPanel.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing SelectionPanel.h
InputPath=.\SelectionPanel.h

"moc_SelectionPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc SelectionPanel.h -o moc_SelectionPanel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing SelectionPanel.h
InputPath=.\SelectionPanel.h

"moc_SelectionPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc SelectionPanel.h -o moc_SelectionPanel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing SelectionPanel.h
InputPath=.\SelectionPanel.h

"moc_SelectionPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc SelectionPanel.h -o moc_SelectionPanel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing SelectionPanel.h
InputPath=.\SelectionPanel.h

"moc_SelectionPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc SelectionPanel.h -o moc_SelectionPanel.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StdListChoice.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing StdListChoice...
InputPath=.\StdListChoice.h

"moc_StdListChoice.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc StdListChoice.h -o moc_StdListChoice.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing StdListChoice...
InputPath=.\StdListChoice.h

"moc_StdListChoice.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc StdListChoice.h -o moc_StdListChoice.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing StdListChoice.h ...
InputPath=.\StdListChoice.h

"moc_StdListChoice.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc StdListChoice.h -o moc_StdListChoice.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing StdListChoice...
InputPath=.\StdListChoice.h

"moc_StdListChoice.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc StdListChoice.h -o moc_StdListChoice.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=StdTestWin.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__STDTE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing StdTestWin.h...
InputPath=StdTestWin.h

"moc_StdTestWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc StdTestWin.h -o moc_StdTestWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__STDTE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing StdTestWin.h...
InputPath=StdTestWin.h

"moc_StdTestWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc StdTestWin.h -o moc_StdTestWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__STDTE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing StdTestWin.h...
InputPath=StdTestWin.h

"moc_StdTestWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc StdTestWin.h -o moc_StdTestWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__STDTE="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing StdTestWin.h...
InputPath=StdTestWin.h

"moc_StdTestWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc StdTestWin.h -o moc_StdTestWin.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=TopWin.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__TOPWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing TopWin.h...
InputPath=TopWin.h

"moc_TopWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc TopWin.h -o moc_TopWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__TOPWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing TopWin.h...
InputPath=TopWin.h

"moc_TopWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc TopWin.h -o moc_TopWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__TOPWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing TopWin.h...
InputPath=TopWin.h

"moc_TopWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc TopWin.h -o moc_TopWin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__TOPWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing TopWin.h...
InputPath=TopWin.h

"moc_TopWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc TopWin.h -o moc_TopWin.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TotRecal.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing TotRecal.h...
InputPath=.\TotRecal.h

"moc_TotRecal.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc TotRecal.h -o moc_TotRecal.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing TotRecal.h...
InputPath=.\TotRecal.h

"moc_TotRecal.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc TotRecal.h -o moc_TotRecal.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing TotRecal.h...
InputPath=.\TotRecal.h

"moc_TotRecal.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc TotRecal.h -o moc_TotRecal.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing TotRecal.h...
InputPath=.\TotRecal.h

"moc_TotRecal.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc TotRecal.h -o moc_TotRecal.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Twodsel.h

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Moc'ing Twodsel.h...
InputPath=.\Twodsel.h

"moc_Twodsel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc Twodsel.h -o moc_Twodsel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Moc'ing Twodsel.h...
InputPath=.\Twodsel.h

"moc_Twodsel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc Twodsel.h -o moc_Twodsel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Moc'ing Twodsel.h...
InputPath=.\Twodsel.h

"moc_Twodsel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc Twodsel.h -o moc_Twodsel.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Moc'ing Twodsel.h...
InputPath=.\Twodsel.h

"moc_Twodsel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc Twodsel.h -o moc_Twodsel.cpp

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Forms"

# PROP Default_Filter "ui"
# Begin Source File

SOURCE=BadPixelBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing BadPixelBase.ui ...
InputPath=BadPixelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic BadPixelBase.ui -o BadPixelBase.h \
	$(QTDIR)\bin\uic BadPixelBase.ui -i BadPixelBase.h -o BadPixelBase.cpp \
	$(QTDIR)\bin\moc BadPixelBase.h -o moc_BadPixelBase.cpp \
	

"BadPixelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"BadPixelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_BadPixelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing BadPixelBase.ui ...
InputPath=BadPixelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic BadPixelBase.ui -o BadPixelBase.h \
	$(QTDIR)\bin\uic BadPixelBase.ui -i BadPixelBase.h -o BadPixelBase.cpp \
	$(QTDIR)\bin\moc BadPixelBase.h -o moc_BadPixelBase.cpp \
	

"BadPixelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"BadPixelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_BadPixelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing BadPixelBase.ui ...
InputPath=BadPixelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic BadPixelBase.ui -o BadPixelBase.h \
	$(QTDIR)\bin\uic BadPixelBase.ui -i BadPixelBase.h -o BadPixelBase.cpp \
	$(QTDIR)\bin\moc BadPixelBase.h -o moc_BadPixelBase.cpp \
	

"BadPixelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"BadPixelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_BadPixelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing BadPixelBase.ui ...
InputPath=BadPixelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic BadPixelBase.ui -o BadPixelBase.h \
	$(QTDIR)\bin\uic BadPixelBase.ui -i BadPixelBase.h -o BadPixelBase.cpp \
	$(QTDIR)\bin\moc BadPixelBase.h -o moc_BadPixelBase.cpp \
	

"BadPixelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"BadPixelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_BadPixelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CfgFormBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing CfgFormBase.ui
InputPath=.\CfgFormBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic CfgFormBase.ui -o CfgFormBase.h \
	$(QTDIR)\bin\uic CfgFormBase.ui -i CfgFormBase.h -o CfgFormBase.cpp \
	$(QTDIR)\bin\moc CfgFormBase.h -o moc_CfgFormBase.cpp \
	

"CfgFormBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CfgFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_CfgFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing CfgFormBase.ui...
InputPath=.\CfgFormBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic CfgFormBase.ui -o CfgFormBase.h \
	$(QTDIR)\bin\uic CfgFormBase.ui -i CfgFormBase.h -o CfgFormBase.cpp \
	$(QTDIR)\bin\moc CfgFormBase.h -o moc_CfgFormBase.cpp \
	

"CfgFormBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CfgFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_CfgFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing CfgFormBase.ui...
InputPath=.\CfgFormBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic CfgFormBase.ui -o CfgFormBase.h \
	$(QTDIR)\bin\uic CfgFormBase.ui -i CfgFormBase.h -o CfgFormBase.cpp \
	$(QTDIR)\bin\moc CfgFormBase.h -o moc_CfgFormBase.cpp \
	

"CfgFormBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CfgFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_CfgFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing CfgFormBase.ui
InputPath=.\CfgFormBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic CfgFormBase.ui -o CfgFormBase.h \
	$(QTDIR)\bin\uic CfgFormBase.ui -i CfgFormBase.h -o CfgFormBase.cpp \
	$(QTDIR)\bin\moc CfgFormBase.h -o moc_CfgFormBase.cpp \
	

"CfgFormBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CfgFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_CfgFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=CorrelSelBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__CORREL="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing CorrelSelBase.ui...
InputPath=CorrelSelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic CorrelSelBase.ui -o CorrelSelBase.h \
	$(QTDIR)\bin\uic CorrelSelBase.ui -i CorrelSelBase.h -o CorrelSelBase.cpp \
	$(QTDIR)\bin\moc CorrelSelBase.h -o moc_CorrelSelBase.cpp \
	

"CorrelSelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CorrelSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_CorrelSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__CORREL="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing CorrelSelBase.ui...
InputPath=CorrelSelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic CorrelSelBase.ui -o CorrelSelBase.h \
	$(QTDIR)\bin\uic CorrelSelBase.ui -i CorrelSelBase.h -o CorrelSelBase.cpp \
	$(QTDIR)\bin\moc CorrelSelBase.h -o moc_CorrelSelBase.cpp \
	

"CorrelSelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CorrelSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_CorrelSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__CORREL="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing CorrelSelBase.ui...
InputPath=CorrelSelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic CorrelSelBase.ui -o CorrelSelBase.h \
	$(QTDIR)\bin\uic CorrelSelBase.ui -i CorrelSelBase.h -o CorrelSelBase.cpp \
	$(QTDIR)\bin\moc CorrelSelBase.h -o moc_CorrelSelBase.cpp \
	

"CorrelSelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CorrelSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_CorrelSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__CORREL="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing CorrelSelBase.ui...
InputPath=CorrelSelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic CorrelSelBase.ui -o CorrelSelBase.h \
	$(QTDIR)\bin\uic CorrelSelBase.ui -i CorrelSelBase.h -o CorrelSelBase.cpp \
	$(QTDIR)\bin\moc CorrelSelBase.h -o moc_CorrelSelBase.cpp \
	

"CorrelSelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CorrelSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_CorrelSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=CutPanelBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__CUTPAN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing CutPanelBase.ui...
InputPath=CutPanelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic CutPanelBase.ui -o CutPanelBase.h \
	$(QTDIR)\bin\uic CutPanelBase.ui -i CutPanelBase.h -o CutPanelBase.cpp \
	$(QTDIR)\bin\moc CutPanelBase.h -o moc_CutPanelBase.cpp \
	

"CutPanelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CutPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_CutPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__CUTPAN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing CutPanelBase.ui...
InputPath=CutPanelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic CutPanelBase.ui -o CutPanelBase.h \
	$(QTDIR)\bin\uic CutPanelBase.ui -i CutPanelBase.h -o CutPanelBase.cpp \
	$(QTDIR)\bin\moc CutPanelBase.h -o moc_CutPanelBase.cpp \
	

"CutPanelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CutPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_CutPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__CUTPAN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing CutPanelBase.ui...
InputPath=CutPanelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic CutPanelBase.ui -o CutPanelBase.h \
	$(QTDIR)\bin\uic CutPanelBase.ui -i CutPanelBase.h -o CutPanelBase.cpp \
	$(QTDIR)\bin\moc CutPanelBase.h -o moc_CutPanelBase.cpp \
	

"CutPanelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CutPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_CutPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__CUTPAN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing CutPanelBase.ui...
InputPath=CutPanelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic CutPanelBase.ui -o CutPanelBase.h \
	$(QTDIR)\bin\uic CutPanelBase.ui -i CutPanelBase.h -o CutPanelBase.cpp \
	$(QTDIR)\bin\moc CutPanelBase.h -o moc_CutPanelBase.cpp \
	

"CutPanelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"CutPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_CutPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=FileWinBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__FILEWI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing FileWinBase.ui...
InputPath=FileWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic FileWinBase.ui -o FileWinBase.h \
	$(QTDIR)\bin\uic FileWinBase.ui -i FileWinBase.h -o FileWinBase.cpp \
	$(QTDIR)\bin\moc FileWinBase.h -o moc_FileWinBase.cpp \
	

"FileWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FileWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_FileWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__FILEWI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing FileWinBase.ui...
InputPath=FileWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic FileWinBase.ui -o FileWinBase.h \
	$(QTDIR)\bin\uic FileWinBase.ui -i FileWinBase.h -o FileWinBase.cpp \
	$(QTDIR)\bin\moc FileWinBase.h -o moc_FileWinBase.cpp \
	

"FileWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FileWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_FileWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__FILEWI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing FileWinBase.ui...
InputPath=FileWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic FileWinBase.ui -o FileWinBase.h \
	$(QTDIR)\bin\uic FileWinBase.ui -i FileWinBase.h -o FileWinBase.cpp \
	$(QTDIR)\bin\moc FileWinBase.h -o moc_FileWinBase.cpp \
	

"FileWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FileWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_FileWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__FILEWI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing FileWinBase.ui...
InputPath=FileWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic FileWinBase.ui -o FileWinBase.h \
	$(QTDIR)\bin\uic FileWinBase.ui -i FileWinBase.h -o FileWinBase.cpp \
	$(QTDIR)\bin\moc FileWinBase.h -o moc_FileWinBase.cpp \
	

"FileWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FileWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_FileWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=FitWinBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__FITWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing FitWinBase.ui...
InputPath=FitWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic FitWinBase.ui -o FitWinBase.h \
	$(QTDIR)\bin\uic FitWinBase.ui -i FitWinBase.h -o FitWinBase.cpp \
	$(QTDIR)\bin\moc FitWinBase.h -o moc_FitWinBase.cpp \
	

"FitWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FitWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_FitWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__FITWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing FitWinBase.ui...
InputPath=FitWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic FitWinBase.ui -o FitWinBase.h \
	$(QTDIR)\bin\uic FitWinBase.ui -i FitWinBase.h -o FitWinBase.cpp \
	$(QTDIR)\bin\moc FitWinBase.h -o moc_FitWinBase.cpp \
	

"FitWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FitWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_FitWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__FITWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing FitWinBase.ui...
InputPath=FitWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic FitWinBase.ui -o FitWinBase.h \
	$(QTDIR)\bin\uic FitWinBase.ui -i FitWinBase.h -o FitWinBase.cpp \
	$(QTDIR)\bin\moc FitWinBase.h -o moc_FitWinBase.cpp \
	

"FitWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FitWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_FitWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__FITWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing FitWinBase.ui...
InputPath=FitWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic FitWinBase.ui -o FitWinBase.h \
	$(QTDIR)\bin\uic FitWinBase.ui -i FitWinBase.h -o FitWinBase.cpp \
	$(QTDIR)\bin\moc FitWinBase.h -o moc_FitWinBase.cpp \
	

"FitWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"FitWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_FitWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LoadStaveBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing LoadPanedlBase.ui...
InputPath=.\LoadStaveBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic LoadStaveBase.ui -o LoadStaveBase.h \
	$(QTDIR)\bin\uic LoadStaveBase.ui -i LoadStaveBase.h -o LoadStaveBase.cpp \
	$(QTDIR)\bin\moc LoadStaveBase.h -o moc_LoadStaveBase.cpp \
	

"LoadStaveBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LoadStaveBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LoadStaveBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing LoadPanedlBase.ui...
InputPath=.\LoadStaveBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic LoadStaveBase.ui -o LoadStaveBase.h \
	$(QTDIR)\bin\uic LoadStaveBase.ui -i LoadStaveBase.h -o LoadStaveBase.cpp \
	$(QTDIR)\bin\moc LoadStaveBase.h -o moc_LoadStaveBase.cpp \
	

"LoadStaveBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LoadStaveBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LoadStaveBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing LoadPanedlBase.ui...
InputPath=.\LoadStaveBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic LoadStaveBase.ui -o LoadStaveBase.h \
	$(QTDIR)\bin\uic LoadStaveBase.ui -i LoadStaveBase.h -o LoadStaveBase.cpp \
	$(QTDIR)\bin\moc LoadStaveBase.h -o moc_LoadStaveBase.cpp \
	

"LoadStaveBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LoadStaveBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LoadStaveBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing LoadPanedlBase.ui...
InputPath=.\LoadStaveBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic LoadStaveBase.ui -o LoadStaveBase.h \
	$(QTDIR)\bin\uic LoadStaveBase.ui -i LoadStaveBase.h -o LoadStaveBase.cpp \
	$(QTDIR)\bin\moc LoadStaveBase.h -o moc_LoadStaveBase.cpp \
	

"LoadStaveBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LoadStaveBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LoadStaveBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LogDispBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing LogDispBase.ui...
InputPath=.\LogDispBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic LogDispBase.ui -o LogDispBase.h \
	$(QTDIR)\bin\uic LogDispBase.ui -i LogDispBase.h -o LogDispBase.cpp \
	$(QTDIR)\bin\moc LogDispBase.h -o moc_LogDispBase.cpp \
	

"LogDispBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LogDispBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LogDispBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing LogDispBase.ui...
InputPath=.\LogDispBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic LogDispBase.ui -o LogDispBase.h \
	$(QTDIR)\bin\uic LogDispBase.ui -i LogDispBase.h -o LogDispBase.cpp \
	$(QTDIR)\bin\moc LogDispBase.h -o moc_LogDispBase.cpp \
	

"LogDispBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LogDispBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LogDispBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing LogDispBase.ui...
InputPath=.\LogDispBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic LogDispBase.ui -o LogDispBase.h \
	$(QTDIR)\bin\uic LogDispBase.ui -i LogDispBase.h -o LogDispBase.cpp \
	$(QTDIR)\bin\moc LogDispBase.h -o moc_LogDispBase.cpp \
	

"LogDispBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LogDispBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LogDispBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing LogDispBase.ui...
InputPath=.\LogDispBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic LogDispBase.ui -o LogDispBase.h \
	$(QTDIR)\bin\uic LogDispBase.ui -i LogDispBase.h -o LogDispBase.cpp \
	$(QTDIR)\bin\moc LogDispBase.h -o moc_LogDispBase.cpp \
	

"LogDispBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LogDispBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LogDispBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LoginPanel.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing LoginPanel.ui...
InputPath=.\LoginPanel.ui

BuildCmds= \
	$(QTDIR)\bin\uic LoginPanel.ui -o LoginPanel.h \
	$(QTDIR)\bin\uic LoginPanel.ui -i LoginPanel.h -o LoginPanel.cpp \
	$(QTDIR)\bin\moc LoginPanel.h -o moc_LoginPanel.cpp \
	

"LoginPanel.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LoginPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LoginPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing LoginPanel.ui...
InputPath=.\LoginPanel.ui

BuildCmds= \
	$(QTDIR)\bin\uic LoginPanel.ui -o LoginPanel.h \
	$(QTDIR)\bin\uic LoginPanel.ui -i LoginPanel.h -o LoginPanel.cpp \
	$(QTDIR)\bin\moc LoginPanel.h -o moc_LoginPanel.cpp \
	

"LoginPanel.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LoginPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LoginPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing LoginPanel.ui...
InputPath=.\LoginPanel.ui

BuildCmds= \
	$(QTDIR)\bin\uic LoginPanel.ui -o LoginPanel.h \
	$(QTDIR)\bin\uic LoginPanel.ui -i LoginPanel.h -o LoginPanel.cpp \
	$(QTDIR)\bin\moc LoginPanel.h -o moc_LoginPanel.cpp \
	

"LoginPanel.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LoginPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LoginPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing LoginPanel.ui...
InputPath=.\LoginPanel.ui

BuildCmds= \
	$(QTDIR)\bin\uic LoginPanel.ui -o LoginPanel.h \
	$(QTDIR)\bin\uic LoginPanel.ui -i LoginPanel.h -o LoginPanel.cpp \
	$(QTDIR)\bin\moc LoginPanel.h -o moc_LoginPanel.cpp \
	

"LoginPanel.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LoginPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LoginPanel.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=MaskedSelBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__MASKE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing MaskedSelBase.ui...
InputPath=MaskedSelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic MaskedSelBase.ui -o MaskedSelBase.h \
	$(QTDIR)\bin\uic MaskedSelBase.ui -i MaskedSelBase.h -o MaskedSelBase.cpp \
	$(QTDIR)\bin\moc MaskedSelBase.h -o moc_MaskedSelBase.cpp \
	

"MaskedSelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MaskedSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_MaskedSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__MASKE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing MaskedSelBase.ui...
InputPath=MaskedSelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic MaskedSelBase.ui -o MaskedSelBase.h \
	$(QTDIR)\bin\uic MaskedSelBase.ui -i MaskedSelBase.h -o MaskedSelBase.cpp \
	$(QTDIR)\bin\moc MaskedSelBase.h -o moc_MaskedSelBase.cpp \
	

"MaskedSelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MaskedSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_MaskedSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__MASKE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing MaskedSelBase.ui...
InputPath=MaskedSelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic MaskedSelBase.ui -o MaskedSelBase.h \
	$(QTDIR)\bin\uic MaskedSelBase.ui -i MaskedSelBase.h -o MaskedSelBase.cpp \
	$(QTDIR)\bin\moc MaskedSelBase.h -o moc_MaskedSelBase.cpp \
	

"MaskedSelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MaskedSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_MaskedSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__MASKE="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing MaskedSelBase.ui...
InputPath=MaskedSelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic MaskedSelBase.ui -o MaskedSelBase.h \
	$(QTDIR)\bin\uic MaskedSelBase.ui -i MaskedSelBase.h -o MaskedSelBase.cpp \
	$(QTDIR)\bin\moc MaskedSelBase.h -o moc_MaskedSelBase.cpp \
	

"MaskedSelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MaskedSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_MaskedSelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=ModWin.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__MODWI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ModWin.ui...
InputPath=ModWin.ui

BuildCmds= \
	$(QTDIR)\bin\uic ModWin.ui -o ModWin.h \
	$(QTDIR)\bin\uic ModWin.ui -i ModWin.h -o ModWin.cpp \
	$(QTDIR)\bin\moc ModWin.h -o moc_ModWin.cpp \
	

"ModWin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ModWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_ModWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__MODWI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ModWin.ui...
InputPath=ModWin.ui

BuildCmds= \
	$(QTDIR)\bin\uic ModWin.ui -o ModWin.h \
	$(QTDIR)\bin\uic ModWin.ui -i ModWin.h -o ModWin.cpp \
	$(QTDIR)\bin\moc ModWin.h -o moc_ModWin.cpp \
	

"ModWin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ModWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_ModWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__MODWI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ModWin.ui...
InputPath=ModWin.ui

BuildCmds= \
	$(QTDIR)\bin\uic ModWin.ui -o ModWin.h \
	$(QTDIR)\bin\uic ModWin.ui -i ModWin.h -o ModWin.cpp \
	$(QTDIR)\bin\moc ModWin.h -o moc_ModWin.cpp \
	

"ModWin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ModWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_ModWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__MODWI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing ModWin.ui...
InputPath=ModWin.ui

BuildCmds= \
	$(QTDIR)\bin\uic ModWin.ui -o ModWin.h \
	$(QTDIR)\bin\uic ModWin.ui -i ModWin.h -o ModWin.cpp \
	$(QTDIR)\bin\moc ModWin.h -o moc_ModWin.cpp \
	

"ModWin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ModWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_ModWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MultiLoadBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing MultiLoadBase.ui...
InputPath=.\MultiLoadBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic MultiLoadBase.ui -o MultiLoadBase.h \
	$(QTDIR)\bin\uic MultiLoadBase.ui -i MultiLoadBase.h -o MultiLoadBase.cpp \
	$(QTDIR)\bin\moc MultiLoadBase.h -o moc_MultiLoadBase.cpp \
	

"MultiLoadBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MultiLoadBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_MultiLoadBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing MultiLoadBase.ui...
InputPath=.\MultiLoadBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic MultiLoadBase.ui -o MultiLoadBase.h \
	$(QTDIR)\bin\uic MultiLoadBase.ui -i MultiLoadBase.h -o MultiLoadBase.cpp \
	$(QTDIR)\bin\moc MultiLoadBase.h -o moc_MultiLoadBase.cpp \
	

"MultiLoadBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MultiLoadBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_MultiLoadBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing MultiLoadBase.ui...
InputPath=.\MultiLoadBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic MultiLoadBase.ui -o MultiLoadBase.h \
	$(QTDIR)\bin\uic MultiLoadBase.ui -i MultiLoadBase.h -o MultiLoadBase.cpp \
	$(QTDIR)\bin\moc MultiLoadBase.h -o moc_MultiLoadBase.cpp \
	

"MultiLoadBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MultiLoadBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_MultiLoadBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing MultiLoadBase.ui...
InputPath=.\MultiLoadBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic MultiLoadBase.ui -o MultiLoadBase.h \
	$(QTDIR)\bin\uic MultiLoadBase.ui -i MultiLoadBase.h -o MultiLoadBase.cpp \
	$(QTDIR)\bin\moc MultiLoadBase.h -o moc_MultiLoadBase.cpp \
	

"MultiLoadBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MultiLoadBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_MultiLoadBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=OptWinBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__OPTWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing OptWinBase.ui...
InputPath=OptWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic OptWinBase.ui -o OptWinBase.h \
	$(QTDIR)\bin\uic OptWinBase.ui -i OptWinBase.h -o OptWinBase.cpp \
	$(QTDIR)\bin\moc OptWinBase.h -o moc_OptWinBase.cpp \
	

"OptWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"OptWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_OptWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__OPTWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing OptWinBase.ui...
InputPath=OptWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic OptWinBase.ui -o OptWinBase.h \
	$(QTDIR)\bin\uic OptWinBase.ui -i OptWinBase.h -o OptWinBase.cpp \
	$(QTDIR)\bin\moc OptWinBase.h -o moc_OptWinBase.cpp \
	

"OptWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"OptWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_OptWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__OPTWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing OptWinBase.ui...
InputPath=OptWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic OptWinBase.ui -o OptWinBase.h \
	$(QTDIR)\bin\uic OptWinBase.ui -i OptWinBase.h -o OptWinBase.cpp \
	$(QTDIR)\bin\moc OptWinBase.h -o moc_OptWinBase.cpp \
	

"OptWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"OptWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_OptWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__OPTWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing OptWinBase.ui...
InputPath=OptWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic OptWinBase.ui -o OptWinBase.h \
	$(QTDIR)\bin\uic OptWinBase.ui -i OptWinBase.h -o OptWinBase.cpp \
	$(QTDIR)\bin\moc OptWinBase.h -o moc_OptWinBase.cpp \
	

"OptWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"OptWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_OptWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Overviewselectbase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing Overviewselectbase.ui...
InputPath=.\Overviewselectbase.ui

BuildCmds= \
	$(QTDIR)\bin\uic Overviewselectbase.ui -o Overviewselectbase.h \
	$(QTDIR)\bin\uic Overviewselectbase.ui -i Overviewselectbase.h -o Overviewselectbase.cpp \
	$(QTDIR)\bin\moc Overviewselectbase.h -o moc_Overviewselectbase.cpp \
	

"Overviewselectbase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Overviewselectbase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_Overviewselectbase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing Overviewselectbase.ui...
InputPath=.\Overviewselectbase.ui

BuildCmds= \
	$(QTDIR)\bin\uic Overviewselectbase.ui -o Overviewselectbase.h \
	$(QTDIR)\bin\uic Overviewselectbase.ui -i Overviewselectbase.h -o Overviewselectbase.cpp \
	$(QTDIR)\bin\moc Overviewselectbase.h -o moc_Overviewselectbase.cpp \
	

"Overviewselectbase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Overviewselectbase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_Overviewselectbase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing Overviewselectbase.ui...
InputPath=.\Overviewselectbase.ui

BuildCmds= \
	$(QTDIR)\bin\uic Overviewselectbase.ui -o Overviewselectbase.h \
	$(QTDIR)\bin\uic Overviewselectbase.ui -i Overviewselectbase.h -o Overviewselectbase.cpp \
	$(QTDIR)\bin\moc Overviewselectbase.h -o moc_Overviewselectbase.cpp \
	

"Overviewselectbase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Overviewselectbase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_Overviewselectbase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing Overviewselectbase.ui...
InputPath=.\Overviewselectbase.ui

BuildCmds= \
	$(QTDIR)\bin\uic Overviewselectbase.ui -o Overviewselectbase.h \
	$(QTDIR)\bin\uic Overviewselectbase.ui -i Overviewselectbase.h -o Overviewselectbase.cpp \
	$(QTDIR)\bin\moc Overviewselectbase.h -o moc_Overviewselectbase.cpp \
	

"Overviewselectbase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Overviewselectbase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_Overviewselectbase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PDBFormBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing PDBFormBase.ui...
InputPath=.\PDBFormBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic PDBFormBase.ui -o PDBFormBase.h \
	$(QTDIR)\bin\uic PDBFormBase.ui -i PDBFormBase.h -o PDBFormBase.cpp \
	$(QTDIR)\bin\moc PDBFormBase.h -o moc_PDBFormBase.cpp \
	

"PDBFormBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"PDBFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_PDBFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing PDBFormBase.ui...
InputPath=.\PDBFormBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic PDBFormBase.ui -o PDBFormBase.h \
	$(QTDIR)\bin\uic PDBFormBase.ui -i PDBFormBase.h -o PDBFormBase.cpp \
	$(QTDIR)\bin\moc PDBFormBase.h -o moc_PDBFormBase.cpp \
	

"PDBFormBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"PDBFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_PDBFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing PDBFormBase.ui...
InputPath=.\PDBFormBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic PDBFormBase.ui -o PDBFormBase.h \
	$(QTDIR)\bin\uic PDBFormBase.ui -i PDBFormBase.h -o PDBFormBase.cpp \
	$(QTDIR)\bin\moc PDBFormBase.h -o moc_PDBFormBase.cpp \
	

"PDBFormBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"PDBFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_PDBFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing PDBFormBase.ui...
InputPath=.\PDBFormBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic PDBFormBase.ui -o PDBFormBase.h \
	$(QTDIR)\bin\uic PDBFormBase.ui -i PDBFormBase.h -o PDBFormBase.cpp \
	$(QTDIR)\bin\moc PDBFormBase.h -o moc_PDBFormBase.cpp \
	

"PDBFormBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"PDBFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_PDBFormBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\QAReportBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing QAReportBase.ui...
InputPath=.\QAReportBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic QAReportBase.ui -o QAReportBase.h \
	$(QTDIR)\bin\uic QAReportBase.ui -i QAReportBase.h -o QAReportBase.cpp \
	$(QTDIR)\bin\moc QAReportBase.h -o moc_QAReportBase.cpp \
	

"QAReportBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"QAReportBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_QAReportBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing QAReportBase.ui...
InputPath=.\QAReportBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic QAReportBase.ui -o QAReportBase.h \
	$(QTDIR)\bin\uic QAReportBase.ui -i QAReportBase.h -o QAReportBase.cpp \
	$(QTDIR)\bin\moc QAReportBase.h -o moc_QAReportBase.cpp \
	

"QAReportBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"QAReportBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_QAReportBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing QAReportBase.ui...
InputPath=.\QAReportBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic QAReportBase.ui -o QAReportBase.h \
	$(QTDIR)\bin\uic QAReportBase.ui -i QAReportBase.h -o QAReportBase.cpp \
	$(QTDIR)\bin\moc QAReportBase.h -o moc_QAReportBase.cpp \
	

"QAReportBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"QAReportBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_QAReportBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing QAReportBase.ui...
InputPath=.\QAReportBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic QAReportBase.ui -o QAReportBase.h \
	$(QTDIR)\bin\uic QAReportBase.ui -i QAReportBase.h -o QAReportBase.cpp \
	$(QTDIR)\bin\moc QAReportBase.h -o moc_QAReportBase.cpp \
	

"QAReportBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"QAReportBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_QAReportBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=RefWinBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__REFWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing RefWinBase.ui...
InputPath=RefWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic RefWinBase.ui -o RefWinBase.h \
	$(QTDIR)\bin\uic RefWinBase.ui -i RefWinBase.h -o RefWinBase.cpp \
	$(QTDIR)\bin\moc RefWinBase.h -o moc_RefWinBase.cpp \
	

"RefWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RefWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RefWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__REFWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing RefWinBase.ui...
InputPath=RefWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic RefWinBase.ui -o RefWinBase.h \
	$(QTDIR)\bin\uic RefWinBase.ui -i RefWinBase.h -o RefWinBase.cpp \
	$(QTDIR)\bin\moc RefWinBase.h -o moc_RefWinBase.cpp \
	

"RefWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RefWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RefWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__REFWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing RefWinBase.ui...
InputPath=RefWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic RefWinBase.ui -o RefWinBase.h \
	$(QTDIR)\bin\uic RefWinBase.ui -i RefWinBase.h -o RefWinBase.cpp \
	$(QTDIR)\bin\moc RefWinBase.h -o moc_RefWinBase.cpp \
	

"RefWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RefWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RefWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__REFWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing RefWinBase.ui...
InputPath=RefWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic RefWinBase.ui -o RefWinBase.h \
	$(QTDIR)\bin\uic RefWinBase.ui -i RefWinBase.h -o RefWinBase.cpp \
	$(QTDIR)\bin\moc RefWinBase.h -o moc_RefWinBase.cpp \
	

"RefWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RefWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RefWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RegPanelBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing RegPanelBase.ui...
InputPath=.\RegPanelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic RegPanelBase.ui -o RegPanelBase.h \
	$(QTDIR)\bin\uic RegPanelBase.ui -i RegPanelBase.h -o RegPanelBase.cpp \
	$(QTDIR)\bin\moc RegPanelBase.h -o moc_RegPanelBase.cpp \
	

"RegPanelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RegPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RegPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing RegPanelBase.ui...
InputPath=.\RegPanelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic RegPanelBase.ui -o RegPanelBase.h \
	$(QTDIR)\bin\uic RegPanelBase.ui -i RegPanelBase.h -o RegPanelBase.cpp \
	$(QTDIR)\bin\moc RegPanelBase.h -o moc_RegPanelBase.cpp \
	

"RegPanelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RegPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RegPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing RegPanelBase.ui...
InputPath=.\RegPanelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic RegPanelBase.ui -o RegPanelBase.h \
	$(QTDIR)\bin\uic RegPanelBase.ui -i RegPanelBase.h -o RegPanelBase.cpp \
	$(QTDIR)\bin\moc RegPanelBase.h -o moc_RegPanelBase.cpp \
	

"RegPanelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RegPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RegPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing RegPanelBase.ui...
InputPath=.\RegPanelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic RegPanelBase.ui -o RegPanelBase.h \
	$(QTDIR)\bin\uic RegPanelBase.ui -i RegPanelBase.h -o RegPanelBase.cpp \
	$(QTDIR)\bin\moc RegPanelBase.h -o moc_RegPanelBase.cpp \
	

"RegPanelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RegPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RegPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=RenameWin.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__RENAM="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing RenameWin.ui...
InputPath=RenameWin.ui

BuildCmds= \
	$(QTDIR)\bin\uic RenameWin.ui -o RenameWin.h \
	$(QTDIR)\bin\uic RenameWin.ui -i RenameWin.h -o RenameWin.cpp \
	$(QTDIR)\bin\moc RenameWin.h -o moc_RenameWin.cpp \
	

"RenameWin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RenameWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RenameWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__RENAM="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing RenameWin.ui...
InputPath=RenameWin.ui

BuildCmds= \
	$(QTDIR)\bin\uic RenameWin.ui -o RenameWin.h \
	$(QTDIR)\bin\uic RenameWin.ui -i RenameWin.h -o RenameWin.cpp \
	$(QTDIR)\bin\moc RenameWin.h -o moc_RenameWin.cpp \
	

"RenameWin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RenameWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RenameWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__RENAM="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing RenameWin.ui...
InputPath=RenameWin.ui

BuildCmds= \
	$(QTDIR)\bin\uic RenameWin.ui -o RenameWin.h \
	$(QTDIR)\bin\uic RenameWin.ui -i RenameWin.h -o RenameWin.cpp \
	$(QTDIR)\bin\moc RenameWin.h -o moc_RenameWin.cpp \
	

"RenameWin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RenameWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RenameWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__RENAM="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing RenameWin.ui...
InputPath=RenameWin.ui

BuildCmds= \
	$(QTDIR)\bin\uic RenameWin.ui -o RenameWin.h \
	$(QTDIR)\bin\uic RenameWin.ui -i RenameWin.h -o RenameWin.cpp \
	$(QTDIR)\bin\moc RenameWin.h -o moc_RenameWin.cpp \
	

"RenameWin.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RenameWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RenameWin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RootDataListBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing RootDataListBase.ui...
InputPath=.\RootDataListBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic RootDataListBase.ui -o RootDataListBase.h \
	$(QTDIR)\bin\uic RootDataListBase.ui -i RootDataListBase.h -o RootDataListBase.cpp \
	$(QTDIR)\bin\moc RootDataListBase.h -o moc_RootDataListBase.cpp \
	

"RootDataListBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RootDataListBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RootDataListBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing RootDataListBase.ui...
InputPath=.\RootDataListBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic RootDataListBase.ui -o RootDataListBase.h \
	$(QTDIR)\bin\uic RootDataListBase.ui -i RootDataListBase.h -o RootDataListBase.cpp \
	$(QTDIR)\bin\moc RootDataListBase.h -o moc_RootDataListBase.cpp \
	

"RootDataListBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RootDataListBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RootDataListBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing RootDataListBase.ui...
InputPath=.\RootDataListBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic RootDataListBase.ui -o RootDataListBase.h \
	$(QTDIR)\bin\uic RootDataListBase.ui -i RootDataListBase.h -o RootDataListBase.cpp \
	$(QTDIR)\bin\moc RootDataListBase.h -o moc_RootDataListBase.cpp \
	

"RootDataListBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RootDataListBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RootDataListBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing RootDataListBase.ui...
InputPath=.\RootDataListBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic RootDataListBase.ui -o RootDataListBase.h \
	$(QTDIR)\bin\uic RootDataListBase.ui -i RootDataListBase.h -o RootDataListBase.cpp \
	$(QTDIR)\bin\moc RootDataListBase.h -o moc_RootDataListBase.cpp \
	

"RootDataListBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RootDataListBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_RootDataListBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SelectionPanelBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing SelectionPanelBase.ui
InputPath=.\SelectionPanelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic SelectionPanelBase.ui -o SelectionPanelBase.h \
	$(QTDIR)\bin\uic SelectionPanelBase.ui -i SelectionPanelBase.h -o SelectionPanelBase.cpp \
	$(QTDIR)\bin\moc SelectionPanelBase.h -o moc_SelectionPanelBase.cpp \
	

"SelectionPanelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SelectionPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_SelectionPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing SelectionPanelBase.ui
InputPath=.\SelectionPanelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic SelectionPanelBase.ui -o SelectionPanelBase.h \
	$(QTDIR)\bin\uic SelectionPanelBase.ui -i SelectionPanelBase.h -o SelectionPanelBase.cpp \
	$(QTDIR)\bin\moc SelectionPanelBase.h -o moc_SelectionPanelBase.cpp \
	

"SelectionPanelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SelectionPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_SelectionPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing SelectionPanelBase.ui
InputPath=.\SelectionPanelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic SelectionPanelBase.ui -o SelectionPanelBase.h \
	$(QTDIR)\bin\uic SelectionPanelBase.ui -i SelectionPanelBase.h -o SelectionPanelBase.cpp \
	$(QTDIR)\bin\moc SelectionPanelBase.h -o moc_SelectionPanelBase.cpp \
	

"SelectionPanelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SelectionPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_SelectionPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing SelectionPanelBase.ui
InputPath=.\SelectionPanelBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic SelectionPanelBase.ui -o SelectionPanelBase.h \
	$(QTDIR)\bin\uic SelectionPanelBase.ui -i SelectionPanelBase.h -o SelectionPanelBase.cpp \
	$(QTDIR)\bin\moc SelectionPanelBase.h -o moc_SelectionPanelBase.cpp \
	

"SelectionPanelBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"SelectionPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_SelectionPanelBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=StdTestWinBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__STDTES="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing StdTestWinBase.ui...
InputPath=StdTestWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic StdTestWinBase.ui -o StdTestWinBase.h \
	$(QTDIR)\bin\uic StdTestWinBase.ui -i StdTestWinBase.h -o StdTestWinBase.cpp \
	$(QTDIR)\bin\moc StdTestWinBase.h -o moc_StdTestWinBase.cpp \
	

"StdTestWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StdTestWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_StdTestWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__STDTES="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing StdTestWinBase.ui...
InputPath=StdTestWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic StdTestWinBase.ui -o StdTestWinBase.h \
	$(QTDIR)\bin\uic StdTestWinBase.ui -i StdTestWinBase.h -o StdTestWinBase.cpp \
	$(QTDIR)\bin\moc StdTestWinBase.h -o moc_StdTestWinBase.cpp \
	

"StdTestWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StdTestWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_StdTestWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__STDTES="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing StdTestWinBase.ui...
InputPath=StdTestWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic StdTestWinBase.ui -o StdTestWinBase.h \
	$(QTDIR)\bin\uic StdTestWinBase.ui -i StdTestWinBase.h -o StdTestWinBase.cpp \
	$(QTDIR)\bin\moc StdTestWinBase.h -o moc_StdTestWinBase.cpp \
	

"StdTestWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StdTestWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_StdTestWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__STDTES="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing StdTestWinBase.ui...
InputPath=StdTestWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic StdTestWinBase.ui -o StdTestWinBase.h \
	$(QTDIR)\bin\uic StdTestWinBase.ui -i StdTestWinBase.h -o StdTestWinBase.cpp \
	$(QTDIR)\bin\moc StdTestWinBase.h -o moc_StdTestWinBase.cpp \
	

"StdTestWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"StdTestWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_StdTestWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=TopWinBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

USERDEP__TOPWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing TopWinBase.ui...
InputPath=TopWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic TopWinBase.ui -o TopWinBase.h \
	$(QTDIR)\bin\uic TopWinBase.ui -i TopWinBase.h -o TopWinBase.cpp \
	$(QTDIR)\bin\moc TopWinBase.h -o moc_TopWinBase.cpp \
	

"TopWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TopWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_TopWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

USERDEP__TOPWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing TopWinBase.ui...
InputPath=TopWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic TopWinBase.ui -o TopWinBase.h \
	$(QTDIR)\bin\uic TopWinBase.ui -i TopWinBase.h -o TopWinBase.cpp \
	$(QTDIR)\bin\moc TopWinBase.h -o moc_TopWinBase.cpp \
	

"TopWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TopWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_TopWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

USERDEP__TOPWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing TopWinBase.ui...
InputPath=TopWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic TopWinBase.ui -o TopWinBase.h \
	$(QTDIR)\bin\uic TopWinBase.ui -i TopWinBase.h -o TopWinBase.cpp \
	$(QTDIR)\bin\moc TopWinBase.h -o moc_TopWinBase.cpp \
	

"TopWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TopWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_TopWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

USERDEP__TOPWIN="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing TopWinBase.ui...
InputPath=TopWinBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic TopWinBase.ui -o TopWinBase.h \
	$(QTDIR)\bin\uic TopWinBase.ui -i TopWinBase.h -o TopWinBase.cpp \
	$(QTDIR)\bin\moc TopWinBase.h -o moc_TopWinBase.cpp \
	

"TopWinBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TopWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_TopWinBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TotRecalBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing TotRecalBase.ui...
InputPath=.\TotRecalBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic TotRecalBase.ui -o TotRecalBase.h \
	$(QTDIR)\bin\uic TotRecalBase.ui -i TotRecalBase.h -o TotRecalBase.cpp \
	$(QTDIR)\bin\moc TotRecalBase.h -o moc_TotRecalBase.cpp \
	

"TotRecalBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TotRecalBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_TotRecalBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing TotRecalBase.ui...
InputPath=.\TotRecalBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic TotRecalBase.ui -o TotRecalBase.h \
	$(QTDIR)\bin\uic TotRecalBase.ui -i TotRecalBase.h -o TotRecalBase.cpp \
	$(QTDIR)\bin\moc TotRecalBase.h -o moc_TotRecalBase.cpp \
	

"TotRecalBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TotRecalBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_TotRecalBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing TotRecalBase.ui...
InputPath=.\TotRecalBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic TotRecalBase.ui -o TotRecalBase.h \
	$(QTDIR)\bin\uic TotRecalBase.ui -i TotRecalBase.h -o TotRecalBase.cpp \
	$(QTDIR)\bin\moc TotRecalBase.h -o moc_TotRecalBase.cpp \
	

"TotRecalBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TotRecalBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_TotRecalBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing TotRecalBase.ui...
InputPath=.\TotRecalBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic TotRecalBase.ui -o TotRecalBase.h \
	$(QTDIR)\bin\uic TotRecalBase.ui -i TotRecalBase.h -o TotRecalBase.cpp \
	$(QTDIR)\bin\moc TotRecalBase.h -o moc_TotRecalBase.cpp \
	

"TotRecalBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TotRecalBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_TotRecalBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TwodselBase.ui

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# Begin Custom Build - Uic'ing TwodselBase.ui...
InputPath=.\TwodselBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic TwodselBase.ui -o TwodselBase.h \
	$(QTDIR)\bin\uic TwodselBase.ui -i TwodselBase.h -o TwodselBase.cpp \
	$(QTDIR)\bin\moc TwodselBase.h -o moc_TwodselBase.cpp \
	

"TwodselBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TwodselBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_TwodselBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# Begin Custom Build - Uic'ing TwodselBase.ui...
InputPath=.\TwodselBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic TwodselBase.ui -o TwodselBase.h \
	$(QTDIR)\bin\uic TwodselBase.ui -i TwodselBase.h -o TwodselBase.cpp \
	$(QTDIR)\bin\moc TwodselBase.h -o moc_TwodselBase.cpp \
	

"TwodselBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TwodselBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_TwodselBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

# Begin Custom Build - Uic'ing TwodselBase.ui...
InputPath=.\TwodselBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic TwodselBase.ui -o TwodselBase.h \
	$(QTDIR)\bin\uic TwodselBase.ui -i TwodselBase.h -o TwodselBase.cpp \
	$(QTDIR)\bin\moc TwodselBase.h -o moc_TwodselBase.cpp \
	

"TwodselBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TwodselBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_TwodselBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

# Begin Custom Build - Uic'ing TwodselBase.ui...
InputPath=.\TwodselBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic TwodselBase.ui -o TwodselBase.h \
	$(QTDIR)\bin\uic TwodselBase.ui -i TwodselBase.h -o TwodselBase.cpp \
	$(QTDIR)\bin\moc TwodselBase.h -o moc_TwodselBase.cpp \
	

"TwodselBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TwodselBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_TwodselBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Generated"

# PROP Default_Filter ""
# Begin Source File

SOURCE=BadPixelBase.cpp
# End Source File
# Begin Source File

SOURCE=BadPixelBase.h
# End Source File
# Begin Source File

SOURCE=.\CfgFormBase.cpp
# End Source File
# Begin Source File

SOURCE=CorrelSelBase.cpp
# End Source File
# Begin Source File

SOURCE=CorrelSelBase.h
# End Source File
# Begin Source File

SOURCE=CutPanelBase.cpp
# End Source File
# Begin Source File

SOURCE=dictLogRoot.cpp
# End Source File
# Begin Source File

SOURCE=.\dictPalette.cpp
# End Source File
# Begin Source File

SOURCE=FileWinBase.cpp
# End Source File
# Begin Source File

SOURCE=FileWinBase.h
# End Source File
# Begin Source File

SOURCE=FitWinBase.cpp
# End Source File
# Begin Source File

SOURCE=FitWinBase.h
# End Source File
# Begin Source File

SOURCE=.\LoadStaveBase.cpp
# End Source File
# Begin Source File

SOURCE=LogDispBase.cpp
# End Source File
# Begin Source File

SOURCE=.\LoginPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\LoginPanel.h
# End Source File
# Begin Source File

SOURCE=MaskedSelBase.cpp
# End Source File
# Begin Source File

SOURCE=moc_BadPixel.cpp
# End Source File
# Begin Source File

SOURCE=moc_BadPixelBase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_CDatabase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_CfgForm.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_CfgFormBase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_ConfigViewer.cpp

!IF  "$(CFG)" == "ModuleAnalysis - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Debug with PixLib"

!ELSEIF  "$(CFG)" == "ModuleAnalysis - Win32 Release with PixLib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=moc_CorrelSel.cpp
# End Source File
# Begin Source File

SOURCE=moc_CorrelSelBase.cpp
# End Source File
# Begin Source File

SOURCE=moc_CutPanel.cpp
# End Source File
# Begin Source File

SOURCE=moc_CutPanelBase.cpp
# End Source File
# Begin Source File

SOURCE=moc_FileWin.cpp
# End Source File
# Begin Source File

SOURCE=moc_FileWinBase.cpp
# End Source File
# Begin Source File

SOURCE=moc_FitWin.cpp
# End Source File
# Begin Source File

SOURCE=moc_FitWinBase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_LoadStave.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_LoadStaveBase.cpp
# End Source File
# Begin Source File

SOURCE=moc_LogDisp.cpp
# End Source File
# Begin Source File

SOURCE=moc_LogDispBase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_LoginPanel.cpp
# End Source File
# Begin Source File

SOURCE=moc_MaskedSel.cpp
# End Source File
# Begin Source File

SOURCE=moc_MaskedSelBase.cpp
# End Source File
# Begin Source File

SOURCE=moc_ModWin.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_MultiLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_MultiLoadBase.cpp
# End Source File
# Begin Source File

SOURCE=moc_OptWin.cpp
# End Source File
# Begin Source File

SOURCE=moc_OptWinBase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_Overviewselect.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_Overviewselectbase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_PDBForm.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_PDBFormBase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_QAReport.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_QAReportBase.cpp
# End Source File
# Begin Source File

SOURCE=moc_QRootApplication.cpp
# End Source File
# Begin Source File

SOURCE=moc_RefWin.cpp
# End Source File
# Begin Source File

SOURCE=moc_RefWinBase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_RegPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_RegPanelBase.cpp
# End Source File
# Begin Source File

SOURCE=moc_RenameWin.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_RootDataList.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_RootDataListBase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_SelectionPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_SelectionPanelBase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_StdListChoice.cpp
# End Source File
# Begin Source File

SOURCE=moc_StdTestWin.cpp
# End Source File
# Begin Source File

SOURCE=moc_StdTestWinBase.cpp
# End Source File
# Begin Source File

SOURCE=moc_TopWin.cpp
# End Source File
# Begin Source File

SOURCE=moc_TopWinBase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_TotRecal.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_TotRecalBase.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_Twodsel.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_TwodselBase.cpp
# End Source File
# Begin Source File

SOURCE=ModWin.cpp
# End Source File
# Begin Source File

SOURCE=ModWin.h
# End Source File
# Begin Source File

SOURCE=.\MultiLoadBase.cpp
# End Source File
# Begin Source File

SOURCE=OptWinBase.cpp
# End Source File
# Begin Source File

SOURCE=OptWinBase.h
# End Source File
# Begin Source File

SOURCE=.\Overviewselectbase.cpp
# End Source File
# Begin Source File

SOURCE=.\Overviewselectbase.h
# End Source File
# Begin Source File

SOURCE=.\PDBFormBase.cpp
# End Source File
# Begin Source File

SOURCE=.\PDBFormBase.h
# End Source File
# Begin Source File

SOURCE=.\QAReportBase.cpp
# End Source File
# Begin Source File

SOURCE=RefWinBase.cpp
# End Source File
# Begin Source File

SOURCE=RefWinBase.h
# End Source File
# Begin Source File

SOURCE=.\RegPanelBase.cpp
# End Source File
# Begin Source File

SOURCE=RenameWin.cpp
# End Source File
# Begin Source File

SOURCE=RenameWin.h
# End Source File
# Begin Source File

SOURCE=.\RootDataListBase.cpp
# End Source File
# Begin Source File

SOURCE=.\RootDataListBase.h
# End Source File
# Begin Source File

SOURCE=.\SelectionPanelBase.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectionPanelBase.h
# End Source File
# Begin Source File

SOURCE=StdTestWinBase.cpp
# End Source File
# Begin Source File

SOURCE=StdTestWinBase.h
# End Source File
# Begin Source File

SOURCE=TopWinBase.cpp
# End Source File
# Begin Source File

SOURCE=TopWinBase.h
# End Source File
# Begin Source File

SOURCE=.\TotRecalBase.cpp
# End Source File
# Begin Source File

SOURCE=.\TotRecalBase.h
# End Source File
# Begin Source File

SOURCE=.\TwodselBase.cpp
# End Source File
# End Group
# End Target
# End Project
