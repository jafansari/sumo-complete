# Microsoft Developer Studio Project File - Name="z_libutils_geom" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=z_libutils_geom - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "z_libutils_geom.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "z_libutils_geom.mak" CFG="z_libutils_geom - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "z_libutils_geom - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "z_libutils_geom - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "z_libutils_geom - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\src" /D "HAVE_CONFIG_H" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "z_libutils_geom - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\src" /D "HAVE_CONFIG_H" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
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

# Name "z_libutils_geom - Win32 Release"
# Name "z_libutils_geom - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\utils\geom\bezier.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\Boundary.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\Bresenham.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\GeomConvHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\GeomHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\Helper_ConvexHull.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\Line2D.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\shapes\Polygon2D.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\Position2DVector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\shapes\ShapeContainer.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\utils\geom\AbstractPoly.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\bezier.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\Boundary.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\Bresenham.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\GeomConvHelper.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\GeomHelper.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\HaveBoundary.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\Helper_ConvexHull.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\Line2D.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\shapes\PointOfInterest.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\shapes\Polygon2D.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\Position2D.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\geom\Position2DVector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\shapes\ShapeContainer.h
# End Source File
# End Group
# End Target
# End Project
