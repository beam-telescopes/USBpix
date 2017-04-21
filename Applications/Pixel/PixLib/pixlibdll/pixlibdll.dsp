# Microsoft Developer Studio Project File - Name="pixlibdll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=pixlibdll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pixlibdll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pixlibdll.mak" CFG="pixlibdll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pixlibdll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pixlibdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pixlibdll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PIXLIBDLL_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PIXLIBDLL_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "pixlibdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PIXLIBDLL_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "$(PIX_LIB)" /I "$(VME_INTERFACE)" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PIXLIBDLL_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libCint.lib libHist.lib libGraf.lib libGraf3d.lib libGpad.lib libTree.lib libRint.lib libPostscript.lib libMatrix.lib libPhysics.lib libGui.lib /nologo /dll /debug /machine:I386 /out:"$(PIX_LIB)/PixConfDBInterface/libRoot.so" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "pixlibdll - Win32 Release"
# Name "pixlibdll - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\VmeInterface\BaseException.cxx
# End Source File
# Begin Source File

SOURCE=..\Bits\Bits.cxx
# End Source File
# Begin Source File

SOURCE=..\PixConfDBInterface\PixConfDBInterface.cxx
# End Source File
# Begin Source File

SOURCE=.\pixlibdll.cpp
# End Source File
# Begin Source File

SOURCE=..\PixConfDBInterface\RootDB.cxx
# End Source File
# Begin Source File

SOURCE=..\PixConfDBInterface\RootDBDict.cxx
# End Source File
# Begin Source File

SOURCE=..\PixConfDBInterface\TurboDaqDB.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\VmeInterface\BaseException.h
# End Source File
# Begin Source File

SOURCE=..\Bits\Bits.h
# End Source File
# Begin Source File

SOURCE=..\PixConfDBInterface\PixConfDBInterface.h
# End Source File
# Begin Source File

SOURCE=.\pixlibdll.h
# End Source File
# Begin Source File

SOURCE=..\PixConfDBInterface\RootDB.h
# End Source File
# Begin Source File

SOURCE=..\PixConfDBInterface\RootDBDict.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\PixConfDBInterface\TurboDaqDB.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "LinDef"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\PixConfDBInterface\RootDBLinkDef.h

!IF  "$(CFG)" == "pixlibdll - Win32 Release"

!ELSEIF  "$(CFG)" == "pixlibdll - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputPath=..\PixConfDBInterface\RootDBLinkDef.h

BuildCmds= \
	"%ROOTSYS%\bin\rootcint" -f $(PIX_LIB)\PixConfDBInterface\RootDBDict.cxx -c -I$ROOTSYS\include -I$(PIX_LIB)\PixConfDBInterface -I$(VME_INTERFACE) RootDB.h RootDBLinkDef.h

"RootDBDict.cxx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"RootDBDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "libraries"

# PROP Default_Filter ""
# Begin Source File

SOURCE="C:\Program Files\root\lib\libCint.lib"
# End Source File
# Begin Source File

SOURCE="C:\Program Files\root\lib\libGpad.lib"
# End Source File
# Begin Source File

SOURCE="C:\Program Files\root\lib\libGraf3d.lib"
# End Source File
# Begin Source File

SOURCE="C:\Program Files\root\lib\libGraf.lib"
# End Source File
# Begin Source File

SOURCE="C:\Program Files\root\lib\libGui.lib"
# End Source File
# Begin Source File

SOURCE="C:\Program Files\root\lib\libHist.lib"
# End Source File
# Begin Source File

SOURCE="C:\Program Files\root\lib\libMatrix.lib"
# End Source File
# Begin Source File

SOURCE="C:\Program Files\root\lib\libPhysics.lib"
# End Source File
# Begin Source File

SOURCE="C:\Program Files\root\lib\libPostscript.lib"
# End Source File
# Begin Source File

SOURCE="C:\Program Files\root\lib\libRint.lib"
# End Source File
# Begin Source File

SOURCE="C:\Program Files\root\lib\libTree.lib"
# End Source File
# Begin Source File

SOURCE="C:\Program Files\root\lib\libCore.lib"
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
