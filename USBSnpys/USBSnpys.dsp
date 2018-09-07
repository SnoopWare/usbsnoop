# Microsoft Developer Studio Project File - Name="USBSnpys" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=USBSnpys - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "USBSnpys.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "USBSnpys.mak" CFG="USBSnpys - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "USBSnpys - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "USBSnpys - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/CD/Entertainment/Tools/Snoopy/USBSnpys", DNJCAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "USBSnpys - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Gz /W4 /Z7 /O2 /I "..\Shared" /I "D:\WINDDK\inc\ddk\wdm\w2k" /I "D:\WINDDK\inc\w2k" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D _WIN32_IE=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DBG=1 /D DEVL=1 /D FPO=0 /D _DLL=1 /D "DRIVER" /D "_IDWBUILD" /D "RDRDBG" /D "SRVDBG" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\Shared" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wdm.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry@8" /subsystem:windows /pdb:none /map /debug /debugtype:both /machine:I386 /nodefaultlib /out:"Release\USBSnpys.sys" /libpath:"d:\WINDDK\lib\w2k\i386" -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -MERGE:.rdata=.text -SECTION:INIT,d -OPT:REF -FORCE:MULTIPLE -RELEASE -FULLBUILD -IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 -osversion:4.00 -optidata -driver -align:0x20 -subsystem:native,4.00 -debug:notmapped,FULL
# Begin Custom Build
InputPath=.\Release\USBSnpys.sys
SOURCE="$(InputPath)"

"C:\WinNT\System32\Drivers\USBSnpys.sys" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ECHO copy $(InputPath) %WINDIR%\System32\Drivers 
	copy $(InputPath) %WINDIR%\System32\Drivers 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "USBSnpys - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Gz /W4 /Z7 /Oi /Gy /I "D:\WINDDK\inc\ddk\wdm\w2k" /I "D:\WINDDK\inc\w2k" /I "..\Shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D _WIN32_IE=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DBG=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D "DRIVER" /D "_IDWBUILD" /D "RDRDBG" /D "SRVDBG" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\Shared" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wdm.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry@8" /subsystem:windows /pdb:none /map /debug /debugtype:both /machine:I386 /nodefaultlib /out:"Debug\USBSnpys.sys" /libpath:"d:\WINDDK\lib\w2k\i386" -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -MERGE:.rdata=.text -SECTION:INIT,d -OPT:REF -FORCE:MULTIPLE -RELEASE -FULLBUILD -IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 -osversion:4.00 -optidata -driver -align:0x20 -subsystem:native,4.00 -debug:notmapped,FULL
# Begin Custom Build
InputPath=.\Debug\USBSnpys.sys
SOURCE="$(InputPath)"

"C:\WinNT\System32\Drivers\USBSnpys.sys" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ECHO copy $(InputPath) %WINDIR%\System32\Drivers 
	copy $(InputPath) %WINDIR%\System32\Drivers 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "USBSnpys - Win32 Release"
# Name "USBSnpys - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DriverEntry.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Driver.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\USBSnpys.rc
# End Source File
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

SOURCE=..\Shared\BuildNum.h
# End Source File
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
# End Target
# End Project
