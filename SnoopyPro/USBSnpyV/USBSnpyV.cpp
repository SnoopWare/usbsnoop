//************************************************************************
//
// USBSnpyV.cpp
//
// main implementation file for the USBSnpyV.vxd
//
//************************************************************************


#include "StdDCls.h"

// 98 DDK stuff
extern "C"
{
#define WANTVXDWRAPS
#include <basedef.h>
#include <vmm.h>
#include <vwin32.h>
#include <vxdwraps.h>
#include <winerror.h>
};

#define DbgPrint        _Debug_Printf_Service
#if DBG

#define KdPrint(_x_) DbgPrint _x_
#define KdBreakPoint() DbgBreakPoint()


#else

#define KdPrint(_x_)
#define KdBreakPoint()

#endif

// warning C4200: nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable: 4200)
#include "Interface.h"

#include "USBSnpyV.h"

struct USBSNPYS_GLOBALS GlobalData;

#include "USBSnoopies_Common.inl"


extern "C"
{


__declspec(naked) void __cdecl _chkesp(void)
{
    _asm
    {
        ret 0;
    }
}

ULONG __stdcall _USBSnoopies_DeviceIOCTL(ULONG /* Service */, ULONG /* dwDDB */, ULONG /* hDevice */, DIOCPARAMETERS* pDIOCParams)
{
	ULONG dwResult = ERROR_NOT_SUPPORTED;
    PVOID inBuffer = (PVOID) pDIOCParams->lpvInBuffer;
    ULONG inputBufferLength = pDIOCParams->cbInBuffer;
    PVOID outBuffer = (PVOID) pDIOCParams->lpvOutBuffer;
    ULONG outputBufferLength = pDIOCParams->cbOutBuffer;

	switch (pDIOCParams->dwIoControlCode)
    {
		case DIOC_OPEN:
			// DIOC_OPEN is sent when VxD is accessed by ring 3 with CreateFile
            KdPrint(("USBSnoopies::OPEN\n"));
			dwResult = ERROR_SUCCESS; // Return 0 to tell WIN32 that this VxD supports DEVIOCTL
			break;
			
		case DIOC_CLOSEHANDLE:
			// DIOC_CLOSEHANDLE is sent when VxD is closed from ring 3 with CloseHandle
			// (service = -1 , vmm.inc)
            KdPrint(("USBSnoopies::Close\n"));
			dwResult = ERROR_SUCCESS;
			break;
			
        case USBSNOOP_GET_SNOOPED_DEVS:
            KdPrint(("USBSnoopies::GetSnoopedDevs\n"));
            if(STATUS_SUCCESS == IOCTL_GetSnoopedDevs(outBuffer, outputBufferLength))
            {
                dwResult = ERROR_SUCCESS;
            }
            break;

        case USBSNOOP_ENABLE_LOGGING:
            KdPrint(("USBSnoopies::EnableLogging\n"));
            if(STATUS_SUCCESS == IOCTL_EnableLogging(inBuffer, inputBufferLength))
            {
                dwResult = ERROR_SUCCESS;
            }
            break;

        case USBSNOOP_GET_URBS:
            KdPrint(("USBSnoopies::GetURBS\n"));
            if(STATUS_SUCCESS == IOCTL_GetURBs(inBuffer, inputBufferLength, outBuffer, outputBufferLength))
            {
                dwResult = ERROR_SUCCESS;
            }
            break;

        case USBSNOOP_GET_BUFFER_FULLNESS:
            KdPrint(("USBSnoopies::GetBufferFullness\n"));
            if(STATUS_SUCCESS == IOCTL_GetBufferFullness(inBuffer, inputBufferLength, outBuffer, outputBufferLength))
            {
                dwResult = ERROR_SUCCESS;
            }
            break;

        case USBSNOOPIES_HELLO:
            KdPrint(("USBSnoopies::Hello!\n"));
            if(STATUS_SUCCESS == IOCTL_Hello(inBuffer, inputBufferLength, outBuffer, outputBufferLength))
            {
                dwResult = ERROR_SUCCESS;
            }
            break;
        
        default:
            KdPrint(("USBSnoopies::unknown IOCTL 0x%08x!\n", pDIOCParams->dwIoControlCode));
            break;
    }

    return dwResult;
}

// This gets called once, when the driver gets loaded the first time.
void __stdcall USBSnoopies_OnLoad(void)
{
    DbgPrint("USBSnoopies::OnLoad()\n");
}

// This gets called once, just before the driver gets unloaded.
void __stdcall USBSnoopies_OnUnload(void)
{
    DbgPrint("USBSnoopies::OnUnload()\n");
}

};

//** end of USBSnpyV.cpp *************************************************
/*************************************************************************

  $Log: not supported by cvs2svn $
 * 
 * 1     2/05/02 9:08p Rbosa
 * - base for USBSnoopies VxD

*************************************************************************/