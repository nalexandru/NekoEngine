# Microsoft Developer Studio Project File - Name="miwa" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=miwa - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "miwa.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "miwa.mak" CFG="miwa - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "miwa - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "miwa - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "miwa - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\bin"
# PROP Intermediate_Dir "..\..\..\..\build\miwa"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\..\3rdparty\win\vc6\include" /I "..\..\include" /I "..\..\..\..\3rdparty\win\vc9\include" /I "include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "miwa - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\bin"
# PROP Intermediate_Dir "..\..\..\..\build\miwa"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\..\3rdparty\win\vc6\include" /I "..\..\include" /I "..\..\..\..\3rdparty\win\vc9\include" /I "include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "miwa - Win32 Release"
# Name "miwa - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "system"

# PROP Default_Filter ""
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\system\win32\cond_var.c
# End Source File
# Begin Source File

SOURCE=..\..\src\system\win32\mutex.c
# End Source File
# Begin Source File

SOURCE=..\..\src\system\win32\sys_win32.c
# End Source File
# Begin Source File

SOURCE=..\..\src\system\win32\thread.c
# End Source File
# End Group
# Begin Group "compat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\system\compat\explicit_bzero.c
# End Source File
# Begin Source File

SOURCE=..\..\src\system\compat\reallocarray.c
# End Source File
# Begin Source File

SOURCE=..\..\src\system\compat\snprintf.c
# End Source File
# Begin Source File

SOURCE=..\..\src\system\compat\strlcat.c
# End Source File
# Begin Source File

SOURCE=..\..\src\system\compat\strlcpy.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\src\system\config.c
# End Source File
# Begin Source File

SOURCE=..\..\src\system\error.c
# End Source File
# Begin Source File

SOURCE=..\..\src\system\log.c
# End Source File
# Begin Source File

SOURCE=..\..\src\system\system.c
# End Source File
# Begin Source File

SOURCE=..\..\src\system\threadpool.c
# End Source File
# End Group
# Begin Group "runtime"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\runtime\array.c
# End Source File
# Begin Source File

SOURCE=..\..\src\runtime\queue.c
# End Source File
# Begin Source File

SOURCE=..\..\src\runtime\rt_internal.h
# End Source File
# Begin Source File

SOURCE=..\..\src\runtime\runtime.c
# End Source File
# Begin Source File

SOURCE=..\..\src\runtime\string.c
# End Source File
# Begin Source File

SOURCE=..\..\src\runtime\variant.c
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\runtime\array.h
# End Source File
# Begin Source File

SOURCE=..\..\include\system\compat.h
# End Source File
# Begin Source File

SOURCE=..\..\include\system\cond_var.h
# End Source File
# Begin Source File

SOURCE=..\..\include\system\config.h
# End Source File
# Begin Source File

SOURCE=..\..\include\system\defs.h
# End Source File
# Begin Source File

SOURCE=..\..\include\system\error.h
# End Source File
# Begin Source File

SOURCE=..\..\include\system\log.h
# End Source File
# Begin Source File

SOURCE=..\..\include\system\mutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\runtime\queue.h
# End Source File
# Begin Source File

SOURCE=..\..\include\runtime\runtime.h
# End Source File
# Begin Source File

SOURCE=..\..\include\runtime\string.h
# End Source File
# Begin Source File

SOURCE=..\..\include\system\system.h
# End Source File
# Begin Source File

SOURCE=..\..\include\system\thread.h
# End Source File
# Begin Source File

SOURCE=..\..\include\system\threadpool.h
# End Source File
# Begin Source File

SOURCE=..\..\include\runtime\variant.h
# End Source File
# End Group
# End Target
# End Project
