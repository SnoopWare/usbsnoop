# Microsoft Developer Studio Project File - Name="USBSnpyV" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=USBSnpyV - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "USBSnpyV.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "USBSnpyV.mak" CFG="USBSnpyV - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "USBSnpyV - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "USBSnpyV - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/CD/Entertainment/Tools/Snoopy/USBSnpyV", WRJCAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "USBSnpyV - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Gz /W4 /GX /Ox /Og /Oi /Os /I "..\Shared" /I "..\98DDK" /D "NODEBUG" /D "WIN32" /D "WIN40COMPAT" /D "VXD" /D "IS_32" /D "_X86_" /D "_WINDOWS" /D "_MBCS" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D _WIN32_IE=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DBG=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D "DRIVER" /D "_IDWBUILD" /D "RDRDBG" /D "SRVDBG" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 vxdwraps.clb wdmvxd.clb /nologo /map /machine:I386 /nodefaultlib /out:"Release/USBSnpyV.VxD" /libpath:"..\98DDK" /vxd /exetype:dynamic
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "USBSnpyV - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Gz /W4 /GX /Zi /Oi /I "..\Shared" /I "..\98DDK" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "WIN40COMPAT" /D "VXD" /D "IS_32" /D "_X86_" /D "_WINDOWS" /D "_MBCS" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D _WIN32_IE=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DBG=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D "DRIVER" /D "_IDWBUILD" /D "RDRDBG" /D "SRVDBG" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 vxdwraps.clb wdmvxd.clb /nologo /incremental:no /map /debug /machine:I386 /nodefaultlib /out:"Debug/USBSnpyV.VxD" /pdbtype:sept /libpath:"..\98DDK" /vxd /exetype:dynamic
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "USBSnpyV - Win32 Release"
# Name "USBSnpyV - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\USBSnpyV.cpp
# End Source File
# Begin Source File

SOURCE=.\USBSnpyV.def
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\USBSnpyV.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Shared Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\RingBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\USBSnoopies_Common.inl
# End Source File
# End Group
# Begin Group "Shared Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\Interface.h
# End Source File
# Begin Source File

SOURCE=..\Shared\RingBuffer.h
# End Source File
# Begin Source File

SOURCE=..\Shared\StdDCls.h
# End Source File
# End Group
# Begin Group "98DDK Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\98DDK\BASEDEF.H
# End Source File
# Begin Source File

SOURCE=..\98DDK\VMM.H
# End Source File
# Begin Source File

SOURCE=..\98DDK\VWIN32.H
# End Source File
# Begin Source File

SOURCE=..\98DDK\VXDWRAPS.H
# End Source File
# End Group
# Begin Source File

SOURCE=.\USV_Ctl.asm

!IF  "$(CFG)" == "USBSnpyV - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
IntDir=.\Release
InputPath=.\USV_Ctl.asm
InputName=USV_Ctl

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo off 
	ml /nologo /DWIN40COMPAT /DWIN32 /coff /DBLD_COFF /c /W3 /Cx /Gd /Zd /Zp1 /DNODEBUG /DIS_32 /DMASM6 /I$(IntDir) /Id:\w98ddk\inc\Win98 /Fo$(IntDir)\$(InputName).obj $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "USBSnpyV - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
IntDir=.\Debug
InputPath=.\USV_Ctl.asm
InputName=USV_Ctl

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo off 
	ml /nologo /DWIN40COMPAT /DWIN32 /coff /DBLD_COFF /c /W3 /Cx /Gd /Zd /Zp1 /DNODEBUG /DIS_32 /DMASM6 /I$(IntDir) /Id:\w98ddk\inc\Win98 /Fo$(IntDir)\$(InputName).obj $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
