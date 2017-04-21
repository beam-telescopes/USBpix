# Microsoft Developer Studio Project File - Name="PixAnalysis" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=PixAnalysis - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PixAnalysis.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PixAnalysis.mak" CFG="PixAnalysis - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PixAnalysis - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "tmp"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(ROOTSYS)\include" /I "$(PIX_LIB)\PixConfDBInterface" /I "$(PIX_LIB)\Bits" /I "$(PIX_LIB)\Histo" /I "$(PIX_LIB)/PixController" /I "$(PIX_LIB)" /I "$(ROD_DAQ)/CommonWithDsp" /I "$(ROD_DAQ)/CommonWithDsp/Pixel" /I "$(VME_INTERFACE)" /D "_WINDOWS" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "PIXEL_ROD" /D "TSTAMP" /D "I_AM_NT_HOST" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Target

# Name "PixAnalysis - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DataContainer\PixDBData.cxx
# End Source File
# Begin Source File

SOURCE=.\DataContainer\PixelDataContainer.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\DataContainer\PixDBData.h
# End Source File
# Begin Source File

SOURCE=.\DataContainer\PixelDataContainer.h
# End Source File
# End Group
# End Target
# End Project
