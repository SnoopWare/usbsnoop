# Microsoft Developer Studio Project File - Name="SnoopyPro" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=SnoopyPro - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SnoopyPro.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SnoopyPro.mak" CFG="SnoopyPro - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SnoopyPro - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "SnoopyPro - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/CD/Entertainment/Tools/Snoopy/SnoopyPro", WHACAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SnoopyPro - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W4 /GX /O2 /I "..\Shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\Shared" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 setupapi.lib /nologo /subsystem:windows /map /machine:I386
# Begin Custom Build
InputPath=.\Release\SnoopyPro.exe
InputName=SnoopyPro
SOURCE="$(InputPath)"

"..\..\$(InputName).exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\$(InputName).exe

# End Custom Build

!ELSEIF  "$(CFG)" == "SnoopyPro - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W4 /Gm /GX /ZI /Od /I "..\Shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\Shared" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 setupapi.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "SnoopyPro - Win32 Release"
# Name "SnoopyPro - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\DevicesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Exporter.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportLogDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MyMemFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressStatusBar.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadmeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SetupDIMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SnoopyPro.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\URB.cpp
# End Source File
# Begin Source File

SOURCE=.\URBLogListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\URLStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\USBLogDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\USBLogView.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\DevicesDlg.h
# End Source File
# Begin Source File

SOURCE=.\Exporter.h
# End Source File
# Begin Source File

SOURCE=.\ExportLogDlg.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MyMemFile.h
# End Source File
# Begin Source File

SOURCE=.\ProgressStatusBar.h
# End Source File
# Begin Source File

SOURCE=.\ReadmeDlg.h
# End Source File
# Begin Source File

SOURCE=.\SetupDIMgr.h
# End Source File
# Begin Source File

SOURCE=.\SnoopyPro.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\URB.h
# End Source File
# Begin Source File

SOURCE=.\URBLogListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\URLStatic.h
# End Source File
# Begin Source File

SOURCE=.\USBLogDoc.h
# End Source File
# Begin Source File

SOURCE=.\USBLogView.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Closed.ico
# End Source File
# Begin Source File

SOURCE=.\res\ClosedInv.ico
# End Source File
# Begin Source File

SOURCE=.\res\Open.ico
# End Source File
# Begin Source File

SOURCE=.\res\OpenInv.ico
# End Source File
# Begin Source File

SOURCE=.\Res\Pause.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Play.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\res\SnoopyPro.ico
# End Source File
# Begin Source File

SOURCE=.\SnoopyPro.rc
# End Source File
# Begin Source File

SOURCE=.\res\SnoopyPro.rc2
# End Source File
# Begin Source File

SOURCE=.\Res\Stop.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\USBLogDoc.ico
# End Source File
# Begin Source File

SOURCE=.\Res\USBLogo.ico
# End Source File
# End Group
# Begin Group "Shared Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\BuildNum.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Interface.h
# End Source File
# Begin Source File

SOURCE=..\Shared\RingBuffer.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\SnoopyPro.reg
# End Source File
# End Target
# End Project
