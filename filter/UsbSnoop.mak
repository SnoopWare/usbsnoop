# Microsoft Developer Studio Generated NMAKE File, Based on UsbSnoop.dsp
!IF "$(CFG)" == ""
CFG=UsbSnoop - Win32 Debug win98
!MESSAGE No configuration specified. Defaulting to UsbSnoop - Win32 Debug win98.
!ENDIF 

!IF "$(CFG)" != "UsbSnoop - Win32 Debug win2k" && "$(CFG)" != "UsbSnoop - Win32 Debug win98"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "UsbSnoop.mak" CFG="UsbSnoop - Win32 Debug win98"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "UsbSnoop - Win32 Debug win2k" (based on "Win32 (x86) Application")
!MESSAGE "UsbSnoop - Win32 Debug win98" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "UsbSnoop - Win32 Debug win2k"

OUTDIR=.\Debug2k
INTDIR=.\Debug2k
# Begin Custom Macros
OutDir=.\Debug2k
# End Custom Macros

ALL : "$(OUTDIR)\UsbSnoop.sys" "$(OUTDIR)\UsbSnoop.pch" "$(OUTDIR)\UsbSnoop.bsc"


CLEAN :
	-@erase "$(INTDIR)\DriverEntry.obj"
	-@erase "$(INTDIR)\DriverEntry.sbr"
	-@erase "$(INTDIR)\RemoveLock.obj"
	-@erase "$(INTDIR)\RemoveLock.sbr"
	-@erase "$(INTDIR)\stddcls.obj"
	-@erase "$(INTDIR)\stddcls.sbr"
	-@erase "$(INTDIR)\UsbSnoop.pch"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\UsbSnoop.bsc"
	-@erase "$(OUTDIR)\UsbSnoop.sys"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /MLd /W3 /Z7 /Oi /Gy /I "c:\ntddk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D _WIN32_IE=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DBG=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D "DRIVER" /D "_IDWBUILD" /D "RDRDBG" /D "SRVDBG" /D "WIN2K" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /Zel -cbstring /QIfdiv- /QI0f /GF /QIf /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\UsbSnoop.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\DriverEntry.sbr" \
	"$(INTDIR)\RemoveLock.sbr" \
	"$(INTDIR)\stddcls.sbr"

"$(OUTDIR)\UsbSnoop.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=wdm.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry@8" /subsystem:windows /pdb:none /debug /debugtype:both /machine:I386 /nodefaultlib /out:"$(OUTDIR)\UsbSnoop.sys" /libpath:"c:\ntddk\libchk\i386" -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -MERGE:.rdata=.text -SECTION:INIT,d -OPT:REF -FORCE:MULTIPLE -RELEASE -FULLBUILD -IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 -osversion:4.00 -optidata -driver -align:0x20 -subsystem:native,4.00 -debug:notmapped,FULL 
LINK32_OBJS= \
	"$(INTDIR)\DriverEntry.obj" \
	"$(INTDIR)\RemoveLock.obj" \
	"$(INTDIR)\stddcls.obj"

"$(OUTDIR)\UsbSnoop.sys" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

TargetPath=.\debug2k\UsbSnoop.sys
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug2k
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\UsbSnoop.sys" "$(OUTDIR)\UsbSnoop.pch" "$(OUTDIR)\UsbSnoop.bsc"
   copy .\debug2k\UsbSnoop.sys C:\WINDOWS\system32\drivers
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "UsbSnoop - Win32 Debug win98"

OUTDIR=.\Debug98
INTDIR=.\Debug98
# Begin Custom Macros
OutDir=.\Debug98
# End Custom Macros

ALL : "$(OUTDIR)\UsbSnoop.sys" "$(OUTDIR)\UsbSnoop.pch" "$(OUTDIR)\UsbSnoop.bsc"


CLEAN :
	-@erase "$(INTDIR)\DriverEntry.obj"
	-@erase "$(INTDIR)\DriverEntry.sbr"
	-@erase "$(INTDIR)\RemoveLock.obj"
	-@erase "$(INTDIR)\RemoveLock.sbr"
	-@erase "$(INTDIR)\stddcls.obj"
	-@erase "$(INTDIR)\stddcls.sbr"
	-@erase "$(INTDIR)\UsbSnoop.pch"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\UsbSnoop.bsc"
	-@erase "$(OUTDIR)\UsbSnoop.sys"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /MLd /W3 /Z7 /Oi /Gy /I "c:\98ddk\inc\win98" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D _WIN32_IE=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DBG=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D "DRIVER" /D "_IDWBUILD" /D "RDRDBG" /D "SRVDBG" /D "WIN98" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /Zel -cbstring /QIfdiv- /QI0f /GF /QIf /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\UsbSnoop.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\DriverEntry.sbr" \
	"$(INTDIR)\RemoveLock.sbr" \
	"$(INTDIR)\stddcls.sbr"

"$(OUTDIR)\UsbSnoop.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=wdm.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry@8" /subsystem:windows /pdb:none /debug /debugtype:both /machine:I386 /nodefaultlib /out:"$(OUTDIR)\UsbSnoop.sys" /libpath:"c:\98ddk\lib\i386\checked" -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -MERGE:.rdata=.text -SECTION:INIT,d -OPT:REF -FORCE:MULTIPLE -RELEASE -FULLBUILD -IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 -osversion:4.00 -optidata -driver -align:0x20 -subsystem:native,4.00 -debug:notmapped,FULL 
LINK32_OBJS= \
	"$(INTDIR)\DriverEntry.obj" \
	"$(INTDIR)\RemoveLock.obj" \
	"$(INTDIR)\stddcls.obj"

"$(OUTDIR)\UsbSnoop.sys" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

TargetPath=.\debug98\UsbSnoop.sys
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug98
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\UsbSnoop.sys" "$(OUTDIR)\UsbSnoop.pch" "$(OUTDIR)\UsbSnoop.bsc"
   copy .\debug98\UsbSnoop.sys C:\WINDOWS\system32\drivers
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("UsbSnoop.dep")
!INCLUDE "UsbSnoop.dep"
!ELSE 
!MESSAGE Warning: cannot find "UsbSnoop.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "UsbSnoop - Win32 Debug win2k" || "$(CFG)" == "UsbSnoop - Win32 Debug win98"
SOURCE=.\DriverEntry.cpp

"$(INTDIR)\DriverEntry.obj"	"$(INTDIR)\DriverEntry.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RemoveLock.cpp

"$(INTDIR)\RemoveLock.obj"	"$(INTDIR)\RemoveLock.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stddcls.cpp

!IF  "$(CFG)" == "UsbSnoop - Win32 Debug win2k"

CPP_SWITCHES=/nologo /Gz /MLd /W3 /Z7 /Oi /Gy /I "c:\ntddk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D _WIN32_IE=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DBG=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D "DRIVER" /D "_IDWBUILD" /D "RDRDBG" /D "SRVDBG" /D "WIN2K" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\UsbSnoop.pch" /Yc"stddcls.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /Zel -cbstring /QIfdiv- /QI0f /GF /QIf /c 

"$(INTDIR)\stddcls.obj"	"$(INTDIR)\stddcls.sbr"	"$(INTDIR)\UsbSnoop.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "UsbSnoop - Win32 Debug win98"

CPP_SWITCHES=/nologo /Gz /MLd /W3 /Z7 /Oi /Gy /I "c:\98ddk\inc\win98" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D _WIN32_IE=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DBG=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D "DRIVER" /D "_IDWBUILD" /D "RDRDBG" /D "SRVDBG" /D "WIN98" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\UsbSnoop.pch" /Yc"stddcls.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /Zel -cbstring /QIfdiv- /QI0f /GF /QIf /c 

"$(INTDIR)\stddcls.obj"	"$(INTDIR)\stddcls.sbr"	"$(INTDIR)\UsbSnoop.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

