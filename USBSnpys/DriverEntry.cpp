
// Windows 2K WDM driver

//************************************************************************
//
// DriverEntry.cpp
//
//************************************************************************

#include "StdDCls.h"
#include "BuildNum.h"
#include "Interface.h"
#include "Driver.h"


VOID DriverUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS DispatchCreateClose(IN PDEVICE_OBJECT fido, IN PIRP Irp);
NTSTATUS DispatchIOCTL(IN PDEVICE_OBJECT fido, IN PIRP Irp);
NTSTATUS DispatchInternalIOCTL(IN PDEVICE_OBJECT fido, IN PIRP Irp);
NTSTATUS CreateControllerObject(IN PDRIVER_OBJECT DriverObject);
NTSTATUS RemoveControllerObject(void);

struct USBSNPYS_GLOBALS GlobalData;

// To enable Kernel Messages on a free build:
#undef KdPrint
#define KdPrint(_x_) DbgPrint _x_

//////////////////////////////////////////////////////////////////////////

#pragma INITCODE

extern "C" NTSTATUS __stdcall DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    DbgPrint("USBSnpys " _VERSION_STR_ " - Entering DriverEntry\n");

    // Insist that OS support at least the WDM 1.0 (Win98 DDK)
    if (!IoIsWdmVersionAvailable(1, 0))
    {
        KdPrint(("USBSnpys - Expected version of WDM (%d.%2.2d) not available\n", 1, 0));
        return STATUS_UNSUCCESSFUL;
    }

    // Initialize the global stuff
    RtlZeroMemory(&GlobalData, sizeof(GlobalData));

    // Initialize function pointers
    DriverObject->DriverUnload = DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIOCTL;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = DispatchInternalIOCTL;

	NTSTATUS status = CreateControllerObject(DriverObject);

	KdPrint(("USBSnpys " _VERSION_STR_ " - DriverEntry returned %ld\n", status));

	return status;
}

//////////////////////////////////////////////////////////////////////////

#pragma LOCKEDCODE

#include "USBSnoopies_Common.inl"

//////////////////////////////////////////////////////////////////////////

#pragma PAGEDCODE

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(DriverObject);

    if(NULL != GlobalData.Snoopy.pfnReleaseSnoopies)
    {
        KdPrint(("USBSnpys - About to tell all attached snoopy devices to let go...\n"));
        GlobalData.Snoopy.pfnReleaseSnoopies(NULL);
    }

    RemoveControllerObject();

    while(0 < GlobalData.Snooped.uCount)
    {
        if(!GlobalData.HasBeenRemoved[0])
        {
            if(NULL != GlobalData.Snoopy.pfnReleaseSnoopies)
            {
                GlobalData.Snoopy.pfnReleaseSnoopies(GlobalData.Snooped.Entry[0].DeviceObject);
            }
        }
        DeviceDelete(GlobalData.Snooped.Entry[0].uDeviceID);
    }

    DbgPrint("USBSnpys - unloaded.\n");
}

///////////////////////////////////////////////////////////////////////////////

#pragma LOCKEDCODE

NTSTATUS CompleteRequest(IN PIRP Irp, IN NTSTATUS status, IN ULONG *info)
{
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = (ULONG)((DWORD_PTR)info);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

///////////////////////////////////////////////////////////////////////////////

#pragma LOCKEDCODE

NTSTATUS DispatchCreateClose(IN PDEVICE_OBJECT fido, IN PIRP Irp)
{
    UNREFERENCED_PARAMETER(fido);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

    switch(stack->MajorFunction)
    {
    case IRP_MJ_CREATE:
//		KdPrint(("USBSnpys::CreateClose() - IRP_MJ_CREATE\n"));
        break;
    case IRP_MJ_CLOSE:
//		KdPrint(("USBSnpys::CreateClose() - IRP_MJ_CLOSE\n"));
        break;
    default:
        break;
    }

    return CompleteRequest(Irp, STATUS_SUCCESS, 0);
}

NTSTATUS DispatchIOCTL(IN PDEVICE_OBJECT fido, IN PIRP Irp)
{
    UNREFERENCED_PARAMETER(fido);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG dwControlCode = stack->Parameters.DeviceIoControl.IoControlCode;
    PVOID inBuffer = Irp->AssociatedIrp.SystemBuffer;
    PVOID outBuffer = Irp->UserBuffer;
    ULONG inputBufferLength  = stack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG outputBufferLength = stack->Parameters.DeviceIoControl.OutputBufferLength;

    NTSTATUS status = STATUS_INVALID_PARAMETER;
    switch (dwControlCode)
    {
    case USBSNOOP_GET_SNOOPED_DEVS:
        {
            status = IOCTL_GetSnoopedDevs(outBuffer, outputBufferLength);
        }
        break;

    case USBSNOOP_ENABLE_LOGGING:
        {
            status = IOCTL_EnableLogging(inBuffer, inputBufferLength);
        }
        break;

    case USBSNOOP_GET_URBS:
        {
            status = IOCTL_GetURBs(inBuffer, inputBufferLength, outBuffer, outputBufferLength);
        }
        break;

    case USBSNOOP_GET_BUFFER_FULLNESS:
        {
            status = IOCTL_GetBufferFullness(inBuffer, inputBufferLength, outBuffer, outputBufferLength);
        }
        break;

    default:
        KdPrint(("USBSnpys - warning: unknown IOCTL: 0x%08x\n", dwControlCode));
        break;
    }

    return CompleteRequest(Irp, status, 0);
}

NTSTATUS DispatchInternalIOCTL(IN PDEVICE_OBJECT fido, IN PIRP Irp)
{
    UNREFERENCED_PARAMETER(fido);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG dwControlCode = stack->Parameters.DeviceIoControl.IoControlCode;
    PVOID inBuffer = Irp->AssociatedIrp.SystemBuffer;
    PVOID outBuffer = Irp->UserBuffer;
    ULONG inputBufferLength  = stack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG outputBufferLength = stack->Parameters.DeviceIoControl.OutputBufferLength;

    NTSTATUS status = STATUS_INVALID_PARAMETER;
    switch(dwControlCode)
    {
    case USBSNOOPIES_HELLO:
        {
            status = IOCTL_Hello(inBuffer, inputBufferLength, outBuffer, outputBufferLength);
        }
        break;

    default:
        KdPrint(("USBSnpys - warning: unknown internal IOCTL: 0x%08x\n", dwControlCode));
        break;
    }

    return CompleteRequest(Irp, status, 0);
}


NTSTATUS CreateControllerObject(IN PDRIVER_OBJECT DriverObject)
{
    UNICODE_STRING uniNtNameString;
    RtlInitUnicodeString(&uniNtNameString, USBSNPYS_NTNAME);

    NTSTATUS status = IoCreateDevice(
        DriverObject,
        0,
        &uniNtNameString,
        FILE_DEVICE_UNKNOWN,
        0,
        FALSE,
        &GlobalData.ControlObject);
    if(!NT_SUCCESS(status))
    {
        KdPrint(("USBSnpys - ERROR creating controller object! (status: 0x%08x)\n", status));
        return status;
    }

    UNICODE_STRING uniWin32NameString;
    RtlInitUnicodeString(&uniWin32NameString, USBSNPYS_SYMNAME);
    status = IoCreateSymbolicLink(&uniWin32NameString, &uniNtNameString);
    if(!NT_SUCCESS(status))
    {
        KdPrint(("USBSnpys - ERROR creating symbolic link! (status: 0x%08x)\n", status));
        IoDeleteDevice (GlobalData.ControlObject);
        GlobalData.ControlObject = NULL;
    }
    
    return status;
}

NTSTATUS RemoveControllerObject()
{
    // Destroying controller object...
    UNICODE_STRING uniWin32NameString;
    RtlInitUnicodeString(&uniWin32NameString, USBSNPYS_SYMNAME);
    IoDeleteSymbolicLink(&uniWin32NameString);
    IoDeleteDevice(GlobalData.ControlObject);
    GlobalData.ControlObject = NULL;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

#ifndef _WIN64
#pragma LOCKEDCODE

extern "C" void __declspec(naked) __cdecl _chkesp()
{
    _asm je okay
        ASSERT(!"Stack pointer mismatch!");
okay:
    _asm ret
}
#endif // _WIN64

//** end of DriverEntry.cpp **********************************************
/*************************************************************************

  $Log: DriverEntry.cpp,v $
  Revision 1.1  2002/08/14 23:10:42  rbosa
  the WDM driver to connect to the snooper filter (used under Win2K)

 * 
 * 2     2/05/02 9:27p Rbosa
 * - moved common code over to inlined file
 * 
 * 1     1/25/02 2:45p Rbosa

*************************************************************************/
