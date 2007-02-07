# Microsoft Developer Studio Project File - Name="y_libnetload" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=y_libnetload - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "y_libnetload.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "y_libnetload.mak" CFG="y_libnetload - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "y_libnetload - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "y_libnetload - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "y_libnetload - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(XERCES)\include" /I "..\..\..\src" /D "NDEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "y_libnetload - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(XERCES)\include" /I "..\..\..\src" /D "_DEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "y_libnetload - Win32 Release"
# Name "y_libnetload - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\netload\NLBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLDetectorBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLDiscreteEventBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLEdgeControlBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLGeomShapeBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLJunctionControlBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLSucceedingLaneBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLTriggerBuilder.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\netload\NLBuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLDetectorBuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLDiscreteEventBuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLEdgeControlBuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLGeomShapeBuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLJunctionControlBuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLLoadFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLSucceedingLaneBuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\netload\NLTriggerBuilder.h
# End Source File
# End Group
# End Target
# End Project
