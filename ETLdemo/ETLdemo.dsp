# Microsoft Developer Studio Project File - Name="ETLdemo" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ETLdemo - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ETLdemo.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ETLdemo.mak" CFG="ETLdemo - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ETLdemo - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ETLdemo - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
F90=df.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ETLdemo - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE F90 /compile_only /include:"Release/" /nologo /warn:nofileopt /winapp
# ADD F90 /compile_only /include:"Release/" /nologo /warn:nofileopt /winapp
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /Zi /O2 /I "../libs" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /profile /debug /machine:I386

!ELSEIF  "$(CFG)" == "ETLdemo - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE F90 /check:bounds /compile_only /debug:full /include:"Debug/" /nologo /warn:argument_checking /warn:nofileopt /winapp
# ADD F90 /browser /check:bounds /compile_only /debug:full /include:"Debug/" /nologo /warn:argument_checking /warn:nofileopt /winapp
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /GX /ZI /Od /I "../libs" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 MFC42D.LIB /nologo /subsystem:windows /profile /debug /machine:I386 /nodefaultlib:"MFC42D.LIB"

!ENDIF 

# Begin Target

# Name "ETLdemo - Win32 Release"
# Name "ETLdemo - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;f90;for;f;fpp"
# Begin Source File

SOURCE=.\Categories.cpp
# End Source File
# Begin Source File

SOURCE=.\Customers.cpp
# End Source File
# Begin Source File

SOURCE=.\DataSourceLocator.cpp
# End Source File
# Begin Source File

SOURCE=.\DemoCopyHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgCopyDesign.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgUIChoicePrompt.cpp
# End Source File
# Begin Source File

SOURCE=.\Employees.cpp
# End Source File
# Begin Source File

SOURCE=.\ETLdemo.cpp
# End Source File
# Begin Source File

SOURCE=.\ETLdemo.rc
# End Source File
# Begin Source File

SOURCE=.\ETLdemoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OrderDetails.cpp
# End Source File
# Begin Source File

SOURCE=.\Orders.cpp
# End Source File
# Begin Source File

SOURCE=.\Products.cpp
# End Source File
# Begin Source File

SOURCE=.\radix40.cpp
# End Source File
# Begin Source File

SOURCE=.\Shippers.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Suppliers.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\Categories.h
# End Source File
# Begin Source File

SOURCE=.\Customers.h
# End Source File
# Begin Source File

SOURCE=.\DataSourceLocator.h
# End Source File
# Begin Source File

SOURCE=.\DemoCopyHelper.h
# End Source File
# Begin Source File

SOURCE=.\DlgCopyDesign.h
# End Source File
# Begin Source File

SOURCE=.\DlgUIChoicePrompt.h
# End Source File
# Begin Source File

SOURCE=.\Employees.h
# End Source File
# Begin Source File

SOURCE=.\ETLdemo.h
# End Source File
# Begin Source File

SOURCE=.\ETLdemoDlg.h
# End Source File
# Begin Source File

SOURCE=.\OrderDetails.h
# End Source File
# Begin Source File

SOURCE=.\Orders.h
# End Source File
# Begin Source File

SOURCE=.\Products.h
# End Source File
# Begin Source File

SOURCE=.\radix40.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\Shippers.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Suppliers.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\ETLdemo.ico
# End Source File
# Begin Source File

SOURCE=.\res\ETLdemo.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
