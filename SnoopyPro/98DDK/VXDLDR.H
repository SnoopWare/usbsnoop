/*
;******************************************************************************
;
;   (C) Copyright MICROSOFT Corp., 1992-1992
;
;   Title:	VXDLDR.H
;
;   Version:	1.00
;
;   Date:	29-Apr-1992
;
;   Author:	SMR
;
;------------------------------------------------------------------------------
;
;   Change log:
;
;      DATE	REV		    DESCRIPTION
;   ----------- --- -----------------------------------------------------------
;   2/15/94     BAW Added PELDR definitions
;
;==============================================================================
*/

#ifndef	_VXDLDR_H
#define	_VXDLDR_H

/*XLATOFF*/
#ifdef __cplusplus
extern "C" {
#endif
/*XLATON*/

#ifndef	Not_VxD

/*XLATOFF*/
#define	VXDLDR_Service	Declare_Service
/*XLATON*/

/*MACROS*/
Begin_Service_Table	(VXDLDR, VxD)
 VXDLDR_Service	(VXDLDR_GetVersion, LOCAL)
 VXDLDR_Service	(VXDLDR_LoadDevice, LOCAL)
 VXDLDR_Service	(VXDLDR_UnloadDevice, LOCAL)
 VXDLDR_Service	(VXDLDR_DevInitSucceeded, LOCAL)
 VXDLDR_Service	(VXDLDR_DevInitFailed, LOCAL)
 VXDLDR_Service	(VXDLDR_GetDeviceList, LOCAL)
 VXDLDR_Service   (VXDLDR_UnloadMe, LOCAL)

 VXDLDR_Service   (_PELDR_LoadModule, VxD_PAGEABLE_CODE)
 VXDLDR_Service   (_PELDR_GetModuleHandle, VxD_PAGEABLE_CODE)
 VXDLDR_Service   (_PELDR_GetModuleUsage, VxD_PAGEABLE_CODE)
 VXDLDR_Service   (_PELDR_GetEntryPoint, VxD_PAGEABLE_CODE)
 VXDLDR_Service   (_PELDR_GetProcAddress, VxD_PAGEABLE_CODE)
 VXDLDR_Service   (_PELDR_AddExportTable, VxD_PAGEABLE_CODE)
 VXDLDR_Service   (_PELDR_RemoveExportTable, VxD_PAGEABLE_CODE)
 VXDLDR_Service   (_PELDR_FreeModule, VxD_PAGEABLE_CODE)

 VXDLDR_Service   (VXDLDR_Notify, LOCAL)

 VXDLDR_Service   (_PELDR_InitCompleted, VXD_PAGEABLE_CODE)
 VXDLDR_Service   (_PELDR_LoadModuleEx, VxD_PAGEABLE_CODE)
 VXDLDR_Service   (_PELDR_LoadModule2, VxD_PAGEABLE_CODE)

End_Service_Table	(VXDLDR, VxD)
/*ENDMACROS*/

#endif


#define	VXDLDR_INIT_DEVICE	0x000000001

/*
* Error codes returned by VXDLDR
*/

#define	VXDLDR_ERR_OUT_OF_MEMORY	1
#define	VXDLDR_ERR_IN_DOS		2
#define	VXDLDR_ERR_FILE_OPEN_ERROR	3
#define	VXDLDR_ERR_FILE_READ		4
#define	VXDLDR_ERR_DUPLICATE_DEVICE	5
#define	VXDLDR_ERR_BAD_DEVICE_FILE	6
#define	VXDLDR_ERR_DEVICE_REFUSED	7
#define	VXDLDR_ERR_NO_SUCH_DEVICE	8
#define	VXDLDR_ERR_DEVICE_UNLOADABLE	9
#define	VXDLDR_ERR_ALLOC_V86_AREA	10
#define	VXDLDR_ERR_BAD_API_FUNCTION	11

#define	VXDLDR_ERR_MAX			11

/*
* VXDLDR_ObjectNotify notification codes
*/
#define	VXDLDR_NOTIFY_OBJECTUNLOAD	0
#define	VXDLDR_NOTIFY_OBJECTLOAD	1

/*
* API entry point function codes
*/
#define	VXDLDR_APIFUNC_GETVERSION	0
#define	VXDLDR_APIFUNC_LOADDEVICE	1
#define	VXDLDR_APIFUNC_UNLOADDEVICE	2

/*
* Structures used by VXDLDR to maintain the list dynamically loaded vxds
*/

/*XLATOFF*/
#include <pshpack1.h>
/*XLATON*/
struct	ObjectInfo {
 ULONG OI_LinearAddress;	// start addr of object
 ULONG OI_Size;			// size of object in bytes
 ULONG OI_ObjType;			// obj type, defined in loader.h
 ULONG OI_Resident;		// Static object ?
} ;

struct DeviceInfo {
 struct DeviceInfo     *DI_Next;
 UCHAR 			DI_LoadCount;	// Reference Count
 struct VxD_Desc_Block *DI_DDB;	// pointer to DDB
 USHORT 		DI_DeviceID;
 CHAR 		       *DI_ModuleName;	// module name as stored in LE header
 ULONG 			DI_Signature;	// signature to verify the struc
 ULONG 			DI_ObjCount;	// number of objects
 struct ObjectInfo     *DI_ObjInfo;	// pointer to array of ObjectInfos
 ULONG			DI_V86_API_CSIP;// Save area for v86 api entry point
 ULONG			DI_PM_API_CSIP;	// Save area for pm api entry point
 UCHAR                  DI_InitCount;   // number of times (re)initialized
} ;

#define	DI_Loaded	DI_LoadCount

struct _W32IoctlPkt {
 USHORT	W32IO_ErrorCode ;
 USHORT	W32IO_DeviceID ;
 UCHAR		W32IO_ModuleName[1] ;
} ;

/*XLATOFF*/
#include <poppack.h>
/*XLATON*/

#ifndef	Not_VxD

/*XLATOFF*/

typedef	DWORD	VXDLDRRET;

#ifndef	WANTVXDWRAPS

#pragma warning (disable:4035)		// turn off no return code warning

WORD VXDINLINE
DL_GetVersion()
{
	VxDCall(VXDLDR_GetVersion);
}

VXDLDRRET VXDINLINE
DL_LoadDevice(struct VxD_Desc_Block **DDB, struct DeviceInfo **DeviceHandle, PCHAR Filename, BOOL InitDevice)
{
	VXDLDRRET	dlResult;

	_asm	mov	edx, Filename
	_asm	mov	eax, InitDevice
	VxDCall(VXDLDR_LoadDevice);
	_asm	mov	dlResult, eax
	_asm	jc	LoadDeviceError
	_asm	mov	ecx, DDB
	_asm	mov	dword ptr [ecx], eax
	_asm	mov	ecx, DeviceHandle
	_asm	mov	dword ptr [ecx], edx
	_asm	mov	dlResult, 0

LoadDeviceError:
	return(dlResult);
}

VXDLDRRET VXDINLINE
DL_UnloadDevice(USHORT DevID, PCHAR szName)
{
	_asm	mov	bx, DevID
	_asm	mov	edx, szName
	VxDCall(VXDLDR_UnloadDevice);
}

VXDLDRRET VXDINLINE
DL_DevInitSucceeded(struct DeviceInfo *DeviceHandle)
{
	_asm	mov	edx, DeviceHandle
	VxDCall(VXDLDR_DevInitSucceeded);
}

VXDLDRRET VXDINLINE
DL_DevInitFailed(struct DeviceInfo *DeviceHandle)
{
	_asm	mov	edx, DeviceHandle
	VxDCall(VXDLDR_DevInitFailed);
}

struct DeviceInfo VXDINLINE
*DL_GetDeviceList()
{
	VxDCall(VXDLDR_GetDeviceList);
}

#pragma warning (default:4035)		// turn on no return code warning

#endif	// WANTVXDWRAPS

/*XLATON*/

#endif	// Not_VxD

//===========================================================================
//                             PELDR definitions...
//===========================================================================

/*XLATOFF*/

// data types

#ifndef DECLARE_HANDLE
#ifdef STRICT
#define DECLARE_HANDLE(name)    struct name##__ { int unused; }; \
                                typedef const struct name##__ * name
#else   /* STRICT */
#define DECLARE_HANDLE(name)    typedef DWORD name
#endif  /* !STRICT */
#endif

#pragma warning (disable:4209)	// turn off redefinition warning

typedef char *               PSTR ;
typedef void                 *PVOID ;
typedef LONG                 LRESULT ;

#pragma warning (default:4209)	// turn off redefinition warning

// generic reference to a module export table...

DECLARE_HANDLE( HPEEXPORTTABLE ) ;
typedef HPEEXPORTTABLE *PHPEEXPORTTABLE ;

// generic reference to module image

DECLARE_HANDLE( HPEMODULE ) ;
typedef HPEMODULE *PHPEMODULE ;

// generic reference to lists

DECLARE_HANDLE( HLIST ) ;
typedef HLIST *PHLIST ;

/*XLATON*/

// manifest constants

#define PELDR_ERR_NOERROR                0L
#define PELDR_ERR_INVALIDHANDLE          -1L
                                         
#define PELDR_ERR_MEM_NOMEMORY           -2L
                                         
#define PELDR_ERR_FILE_UNABLETOOPEN      -3L
#define PELDR_ERR_FILE_READERROR         -4L
                                         
#define PELDR_ERR_IMAGE_INVALIDSIZE      -5L
#define PELDR_ERR_IMAGE_INVALIDFORMAT    -6L
#define PELDR_ERR_IMAGE_UNKNOWNFIXUP     -7L
#define PELDR_ERR_IMAGE_INVALIDFIXUP     -8L
#define PELDR_ERR_IMAGE_UNDEFINEDIMPORT  -9L

#define PELDR_ERR_EXPORT_DUPLICATE       -10L

#define PELDR_ERR_INIT_WASCOMPLETED      -11L

#define PELDR_ERR_IMAGE_UNDEFINEDDLL     -12L 

#define PELDR_LOADFLAG_FREERESOURCES    0x00000001
#define PELDR_LOADFLAG_PREEMPTABLE      0x00000002
#define PELDR_LOADFLAG_KEEP_INIT        0x00000004      // For NDIS 4.0 compatibility

/*XLATOFF*/

#ifndef WANTVXDWRAPS

// function prototypes

#pragma warning (disable:4035)		// turn on no return code warning

LRESULT VXDINLINE CDECL _PELDR_LoadModule
(
    PHPEMODULE      phl,
    PSTR            pFileName,
    PHLIST          phetl
)
{
   Touch_Register( ecx ) ;
   Touch_Register( edx ) ;

   _asm push phetl
   _asm push pFileName
   _asm push phl
   VxDCall( _PELDR_LoadModule ) ;
   _asm add esp, 3*4
} ;

LRESULT VXDINLINE CDECL _PELDR_LoadModuleEx
(
    PHPEMODULE      phl,
    PSTR            pFileName,
    PHLIST          phetl,
    DWORD           dwFlags
)
{
   Touch_Register( ecx ) ;
   Touch_Register( edx ) ;

   _asm push dwFlags
   _asm push phetl
   _asm push pFileName
   _asm push phl
   VxDCall( _PELDR_LoadModuleEx ) ;
   _asm add esp, 4*4
} ;

LRESULT VXDINLINE CDECL _PELDR_LoadModule2
(
    PHPEMODULE      phl,
    PSTR            pFileName,
    PHLIST          phetl,
    DWORD           cbUnresolvedLibraryBufferSize,
    PSTR            pUnresolvedLibraryBuffer,
    DWORD           cbUnresolvedImportBufferSize,
    PSTR            pUnresolvedImportBuffer,
    DWORD           dwFlags
)
{
   Touch_Register( ecx ) ;
   Touch_Register( edx ) ;

   _asm push dwFlags
   _asm push pUnresolvedImportBuffer
   _asm push cbUnresolvedImportBufferSize
   _asm push pUnresolvedLibraryBuffer
   _asm push cbUnresolvedLibraryBufferSize
   _asm push phetl
   _asm push pFileName
   _asm push phl
   VxDCall( _PELDR_LoadModuleEx ) ;
   _asm add esp, 8*4
} ;

HPEMODULE VXDINLINE CDECL _PELDR_GetModuleHandle
(
    PSTR            pFileName
)
{
   Touch_Register( ecx ) ;
   Touch_Register( edx ) ;

   _asm push pFileName
   VxDCall( _PELDR_GetModuleHandle ) ;
   _asm add esp, 1*4
} ;

LRESULT VXDINLINE CDECL _PELDR_GetModuleUsage
(
    HPEMODULE       hl
) 
{
   Touch_Register( ecx ) ;
   Touch_Register( edx ) ;

   _asm push hl
   VxDCall( _PELDR_GetModuleUsage ) ;
   _asm add esp, 1*4
} ;

PVOID VXDINLINE CDECL _PELDR_GetEntryPoint
(
    HPEMODULE       hl
)
{
   Touch_Register( ecx ) ;
   Touch_Register( edx ) ;

   _asm push hl
   VxDCall( _PELDR_GetEntryPoint ) ;
   _asm add esp, 1*4
} ;


PVOID VXDINLINE CDECL _PELDR_GetProcAddress
(
    HPEMODULE       hl,
    PVOID           pFuncName,
    PHLIST          phetl
)
{
   Touch_Register( ecx ) ;
   Touch_Register( edx ) ;

   _asm push phetl
   _asm push pFuncName
   _asm push hl
   VxDCall( _PELDR_GetProcAddress ) ;
   _asm add esp, 3*4
} ;

LRESULT VXDINLINE CDECL _PELDR_AddExportTable
(
    PHPEEXPORTTABLE     pht,
    PSTR                pszModuleName,
    ULONG               cExportedFunctions,
    ULONG               cExportedNames,
    ULONG               ulOrdinalBase,
    PVOID               *pExportNameList,
    PUSHORT             pExportOrdinals,
    PVOID               *pExportAddrs,
    PHLIST              phetl
)
{
   Touch_Register( ecx ) ;
   Touch_Register( edx ) ;

   _asm push phetl
   _asm push pExportAddrs
   _asm push pExportOrdinals
   _asm push pExportNameList
   _asm push ulOrdinalBase
   _asm push cExportedNames
   _asm push cExportedFunctions
   _asm push pszModuleName
   _asm push pht
   VxDCall( _PELDR_AddExportTable ) ;
   _asm add esp, 9*4
} ;

LRESULT VXDINLINE CDECL _PELDR_RemoveExportTable
(
    HPEEXPORTTABLE      ht,
    PHLIST              phetl
)
{
   Touch_Register( ecx ) ;
   Touch_Register( edx ) ;

   _asm push phetl
   _asm push ht
   VxDCall( _PELDR_RemoveExportTable ) ;
   _asm add esp, 2*4
} ;


LRESULT VXDINLINE CDECL _PELDR_FreeModule
(
    HPEMODULE       hl,
    PHLIST          phetl
)
{
   Touch_Register( ecx ) ;
   Touch_Register( edx ) ;

   _asm push phetl
   _asm push hl
   VxDCall( _PELDR_FreeModule ) ;
   _asm add esp, 2*4
} ;


LRESULT VXDINLINE CDECL _PELDR_InitCompleted
(
    HPEMODULE       hl
)
{
   Touch_Register( ecx ) ;
   Touch_Register( edx ) ;

   _asm push hl
   VxDCall( _PELDR_InitCompleted ) ;
   _asm add esp, 1*4
} ;

#pragma warning (default:4035)		// turn on no return code warning

#endif // WANTVXDWRAPS

#ifdef __cplusplus
}
#endif

/*XLATON*/

#endif	// _VXDLDR_H
