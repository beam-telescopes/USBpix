# Microsoft Developer Studio Project File - Name="LogDisp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=LogDisp - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LogDisp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LogDisp.mak" CFG="LogDisp - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LogDisp - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "LogDisp - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "LogDisp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir ""
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /GX /O1 /I "$(ROOTSYS)\include" /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /D "NO_DEBUG" /D "WIN32" /D "NDEBUG" /D "UNICODE" /D "QT_NO_DEBUG" /FD -Zm200 /c
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 delayimp.lib "libCore.lib" "libCint.lib" "libHist.lib" "libHistPainter.lib" "libGraf.lib" "libGraf3d.lib" "libGpad.lib" "libTree.lib" "libRint.lib" "libPostscript.lib" "libMatrix.lib" "libPhysics.lib" "qt-mt323.lib" "qtmain.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" /nologo /subsystem:windows /machine:IX86 /libpath:"$(QTDIR)\lib" /libpath:"$(ROOTSYS)\lib"

!ELSEIF  "$(CFG)" == "LogDisp - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "$(ROOTSYS)\include" /I "$(QTDIR)\include" /I "$(QTDIR)\mkspecs\win32-msvc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /FD /GZ -Zm200 /c
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 "libCore.lib" "libCint.lib" "libHist.lib" "libHistPainter.lib" "libGraf.lib" "libGraf3d.lib" "libGpad.lib" "libTree.lib" "libRint.lib" "libPostscript.lib" "libMatrix.lib" "libPhysics.lib" "qt-mt323.lib" "qtmain.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" /nologo /subsystem:windows /debug /machine:IX86 /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"$(ROOTSYS)\lib" /libpath:"$(QTDIR)\lib"

!ENDIF 

# Begin Target

# Name "LogDisp - Win32 Release"
# Name "LogDisp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=LogClasses.cpp
# End Source File
# Begin Source File

SOURCE=LogDisp.cpp
# End Source File
# Begin Source File

SOURCE=LogMain.cpp
# End Source File
# Begin Source File

SOURCE=.\ModTree.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=LogClasses.h
# End Source File
# Begin Source File

SOURCE=LogDisp.h

!IF  "$(CFG)" == "LogDisp - Win32 Release"

USERDEP__LOGDI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing LogDisp.h...
InputPath=LogDisp.h

"moc_LogDisp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc LogDisp.h -o moc_LogDisp.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "LogDisp - Win32 Debug"

USERDEP__LOGDI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing LogDisp.h...
InputPath=LogDisp.h

"moc_LogDisp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc LogDisp.h -o moc_LogDisp.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=LogMain.h

!IF  "$(CFG)" == "LogDisp - Win32 Release"

USERDEP__LOGMA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing LogMain.h...
InputPath=LogMain.h

"moc_LogMain.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc LogMain.h -o moc_LogMain.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "LogDisp - Win32 Debug"

USERDEP__LOGMA="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing LogMain.h...
InputPath=LogMain.h

"moc_LogMain.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc LogMain.h -o moc_LogMain.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ModTree.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Forms"

# PROP Default_Filter "ui"
# Begin Source File

SOURCE=LogDispBase.ui

!IF  "$(CFG)" == "LogDisp - Win32 Release"

USERDEP__LOGDIS="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing LogDispBase.ui...
InputPath=LogDispBase.ui

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

!ELSEIF  "$(CFG)" == "LogDisp - Win32 Debug"

USERDEP__LOGDIS="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing LogDispBase.ui...
InputPath=LogDispBase.ui

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

SOURCE=LogMainBase.ui

!IF  "$(CFG)" == "LogDisp - Win32 Release"

USERDEP__LOGMAI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing LogMainBase.ui...
InputPath=LogMainBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic LogMainBase.ui -o LogMainBase.h \
	$(QTDIR)\bin\uic LogMainBase.ui -i LogMainBase.h -o LogMainBase.cpp \
	$(QTDIR)\bin\moc LogMainBase.h -o moc_LogMainBase.cpp \
	

"LogMainBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LogMainBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LogMainBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "LogDisp - Win32 Debug"

USERDEP__LOGMAI="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing LogMainBase.ui...
InputPath=LogMainBase.ui

BuildCmds= \
	$(QTDIR)\bin\uic LogMainBase.ui -o LogMainBase.h \
	$(QTDIR)\bin\uic LogMainBase.ui -i LogMainBase.h -o LogMainBase.cpp \
	$(QTDIR)\bin\moc LogMainBase.h -o moc_LogMainBase.cpp \
	

"LogMainBase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"LogMainBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_LogMainBase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Generated"

# PROP Default_Filter ""
# Begin Source File

SOURCE=LogDispBase.cpp
# End Source File
# Begin Source File

SOURCE=LogDispBase.h
# End Source File
# Begin Source File

SOURCE=LogMainBase.cpp
# End Source File
# Begin Source File

SOURCE=LogMainBase.h
# End Source File
# Begin Source File

SOURCE=moc_LogDisp.cpp
# End Source File
# Begin Source File

SOURCE=moc_LogDispBase.cpp
# End Source File
# Begin Source File

SOURCE=moc_LogMain.cpp
# End Source File
# Begin Source File

SOURCE=moc_LogMainBase.cpp
# End Source File
# End Group
# End Target
# End Project
