//************************************************************************
//
// DriverEntry.cpp
//
//************************************************************************

#include "StdDCls.h"
#include "BuildNum.h"
#include "Interface.h"
#include "Driver.h"

// 98 DDK stuff
extern "C"
{
#define WANTVXDWRAPS
#include <basedef.h>
#include <vmm.h>
#include <vwin32.h>
#include <vxdldr.h>
#include <vxdwraps.h>
};


VOID DriverUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS CompleteRequest(IN PIRP Irp, IN NTSTATUS status, IN ULONG *info);
NTSTATUS AddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT pdo);
NTSTATUS DispatchAny(IN PDEVICE_OBJECT fido, IN PIRP Irp);
NTSTATUS DispatchPower(IN PDEVICE_OBJECT fido, IN PIRP Irp);
NTSTATUS DispatchPnp(IN PDEVICE_OBJECT fido, IN PIRP Irp);
NTSTATUS DispatchInternalIOCTL(IN PDEVICE_OBJECT fido, IN PIRP Irp);
NTSTATUS CallSnoopiesDriverSynchronously(PDEVICE_OBJECT DeviceObject,
                                         ULONG          uIoControlCode,
                                         void          *pInputBuffer,
                                         ULONG          szInputBuffer,
                                         void          *pOutputBuffer,
                                         ULONG          szOutputBuffer,
                                         BOOLEAN        fIsInternalIoCtl);

NTSTATUS ConnectToUSBSnpys(void);
void ReleaseSnoopies(PDEVICE_OBJECT DeviceObject);
ULONG EnableLogging(PDEVICE_OBJECT DeviceObject, ULONG bEnable);


NTSTATUS AddPacket(IN PDEVICE_OBJECT fido, IN PURB pUrb, ULONG uSequenceNumber, BOOLEAN bReturnedFromHCD);

struct USBSNOOP_GLOBALS GlobalData;
BOOLEAN bIsThisWin9x;


///////////////////////////////////////////////////////////////////////////////

#pragma INITCODE

NTSTATUS GetRegistryDword(IN PWCHAR pRegPath, IN PWCHAR ValueName, IN OUT PULONG Value)
{
    RTL_QUERY_REGISTRY_TABLE paramTable[2];     //zero'd second table terminates parms
    ULONG lDefault = *Value;                    // default

    RtlZeroMemory(paramTable, sizeof(paramTable));
    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = ValueName;
    paramTable[0].EntryContext = Value;
    paramTable[0].DefaultType = REG_DWORD;
    paramTable[0].DefaultData = &lDefault;
    paramTable[0].DefaultLength = sizeof(lDefault);
    
    return RtlQueryRegistryValues(RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL, pRegPath, paramTable, NULL, NULL);
}

extern "C" NTSTATUS __stdcall DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    DbgPrint("USBSnoop " _VERSION_STR_  " - Entering DriverEntry: DriverObject %8.8lX\n", DriverObject);

    // Insist that OS support at least the WDM 1.0 (Win98 DDK)
    if (!IoIsWdmVersionAvailable(1, 0))
    {
        KdPrint(("USBSnoop - Expected version of WDM (%d.%2.2d) not available\n", 1, 0));
        return STATUS_UNSUCCESSFUL;
    }

    // See if we're running under Win98 or NT:
    bIsThisWin9x = !IoIsWdmVersionAvailable(1, 10);
    KdPrint(("USBSnoop - Running under Windows %s\n", bIsThisWin9x ? "98" : "NT"));

    // Initialize the global stuff
    RtlZeroMemory(&GlobalData.Snoopies, sizeof(GlobalData.Snoopies));

    if(NT_SUCCESS(GetRegistryDword(L"USBSnoop", L"StealEntrypoints", &GlobalData.lStealEntrypoints)))
    {
        KdPrint(("USBSnoop - lStealEntrypoints is %d -> we are%sstealing...\n", 
            GlobalData.lStealEntrypoints, GlobalData.lStealEntrypoints ? " " : " not "));
    }

    if(!NT_SUCCESS(ConnectToUSBSnpys()))
    {
        KdPrint(("USBSnoop - couldn't connect to USBSnpys...\n"));
    }

    // Initialize function pointers
    DriverObject->DriverUnload = DriverUnload;
    DriverObject->DriverExtension->AddDevice = AddDevice;

    for (int i = 0; i < arraysize(DriverObject->MajorFunction); ++i)
        DriverObject->MajorFunction[i] = DispatchAny;

    DriverObject->MajorFunction[IRP_MJ_POWER] = DispatchPower;
    
    if(!GlobalData.lStealEntrypoints)
    {
        DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
        DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = DispatchInternalIOCTL;
    }

    return STATUS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

#pragma PAGEDCODE

struct DeviceInfo *VxDInfo = NULL;

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(DriverObject);

    if(NULL != GlobalData.Snoopies.pfnUSBSnoopUnloading)
    {
        GlobalData.Snoopies.pfnUSBSnoopUnloading();
    }

    if(bIsThisWin9x)
    {
        // we release the VxD here...
        if(NULL != VxDInfo)
        {
            if(VxDInfo->DI_ModuleName)
            {
                KdPrint(("USBSnoop - unloading VxD...\n"));
                VXDLDR_UnloadDevice(0xffff, VxDInfo->DI_ModuleName);
                VxDInfo = NULL;
            }
        }
    }

    DbgPrint("USBSnoop - unloaded.\n");

}

NTSTATUS CallSnoopiesDriverSynchronously(PDEVICE_OBJECT DeviceObject,
                                         ULONG          uIoControlCode,
                                         void          *pInputBuffer,
                                         ULONG          szInputBuffer,
                                         void          *pOutputBuffer,
                                         ULONG          szOutputBuffer,
                                         BOOLEAN        fIsInternalIoCtl)
{
    PAGED_CODE();

    // event to signal once IRP is processed...
    KEVENT event;
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    // Allocates and sets up an IRP for a device control request.
    IO_STATUS_BLOCK ioStatus;
    PIRP irp = IoBuildDeviceIoControlRequest(
        uIoControlCode,         // IoControlCode
        DeviceObject,           // Points to USBSnpys device object.
        pInputBuffer,           // InputBuffer
        szInputBuffer,          // InputBufferLen
        pOutputBuffer,          // OutputBuffer
        szOutputBuffer,         // OutputBufferLen
        fIsInternalIoCtl,       // Send to IRP_MJ_INTERNAL_DEVICE_CONTROL entry.
        &event,                 // recipient to signal this event when done.
        &ioStatus);             // PIO_STATUS_BLOCK
    if(NULL == irp)
    {
        KdPrint(("USBSnoop - Error BuildingIoControlRequest.\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

#ifdef DEBUG
    PIO_STACK_LOCATION nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);
#endif

    // Call the lobby USBSnpys driver to perform the operation.
    KdPrint(("USBSnoop - Calling IoCallDriver over to USBSnpys\n"));
    NTSTATUS ntStatus = IoCallDriver(DeviceObject, irp);
    KdPrint(("USBSnoop - Return from IoCallDriver with %x\n", ntStatus));

    // If the returned status is PENDING, wait for the request to complete.
    if(ntStatus == STATUS_PENDING)
    {
        KdPrint(("USBSnoop - IOCTL IRP IN PROGRESS: Wait until completed...\n"));

        // Puts the current execution thread in a wait state until the 'event' is signalled.  Allows
        // the processor to dispatch other threads for execution.
        ntStatus = KeWaitForSingleObject(
                           &event,              // The event awaiting a signal.
                           Suspended,           // WaitReason??
                           KernelMode,          // WaitMode
                           FALSE,               // Thread cannot be alerted in the wait state.
                           NULL);               // Block until IRP completed

        KdPrint(("USBSnoop - EXIT PENDING STATUS: Wait for single object, returned %x", ntStatus));
    }
    else
    {
        ioStatus.Status = ntStatus;
    }

    ntStatus = ioStatus.Status;
    return ntStatus;
}

///////////////////////////////////////////////////////////////////////////////

NTSTATUS GetHardwareID(PDEVICE_OBJECT PDO, PCHAR Buffer, ULONG BufferLength, PULONG ResultLength)
/*++

    Routine Description:

        This is the implementation for IOCTL_LHIDFLTR_GET_HARDWARE_ID.
        It is modeled after IOCTL_HID_GET_HARDWARE_ID.

    Arguments:

        PDO - base of the device stack for which we need a hardware ID
        Buffer - caller supplied buffer to hold the hardware ID string
        BufferLength - length of the caller supplied buffer
        Resultlength - pointer to a ULONG in which the required length 
                       is stored

    Return Value:

        NT status code. 
        This procedure may fail with STATUS_BUFFER_TOO_SMALL. In that
        case ResultLength will contain the required size of Buffer.

--*/
{
    NTSTATUS       status;
    ULONG          buffer_length, result_length;
    PCHAR          PropString;

    status = IoGetDeviceProperty(PDO,
                                 DevicePropertyHardwareID,
                                 0,
                                 NULL,
                                 &result_length);

    ASSERT  (STATUS_BUFFER_TOO_SMALL == status);

    buffer_length = result_length;
    PropString = (PCHAR) ExAllocatePool(NonPagedPool, buffer_length);

    if( NULL == PropString )
        return STATUS_INSUFFICIENT_RESOURCES;

    status = IoGetDeviceProperty(PDO,
                        DevicePropertyHardwareID,
                        buffer_length,
                        PropString,
                        &result_length);

    if(NT_SUCCESS(status))
    {
        *ResultLength = result_length;
        if(result_length < BufferLength)
        {
            RtlCopyMemory(Buffer,PropString,result_length);
        }
        else
        {
            status = STATUS_BUFFER_TOO_SMALL;
        }

        ExFreePool(PropString);
    }

    return status;
}

ULONG GetCurrentTimeStamp(void)
{
    LARGE_INTEGER uCurrentTime;
    uCurrentTime.QuadPart = KeQueryInterruptTime();
    uCurrentTime.QuadPart /= 10 * 1000; // 100ns -> 1ms
    return uCurrentTime.LowPart;
}

NTSTATUS AddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT pdo)
{
    PAGED_CODE();

    KdPrint(("USBSnoop - AddDevice: DriverObject %8.8lX, pdo %8.8lX\n", DriverObject, pdo));

    char hardwareid[MAX_PATH];
    ULONG length = 0;
    NTSTATUS status = GetHardwareID(pdo, hardwareid, sizeof(hardwareid), &length);
    if(NT_SUCCESS(status))
    {
        DbgPrint("USBSnoop - Loaded for %S\n", hardwareid);
    }

    PDEVICE_OBJECT fido;
    status = IoCreateDevice(
        DriverObject, 
        sizeof(DEVICE_EXTENSION), 
        NULL,
        FILE_DEVICE_UNKNOWN, 
        0, 
        FALSE, 
        &fido);

    if(!NT_SUCCESS(status))
    {
        KdPrint(("USBSnoop - Error: IoCreateDevice failed - 0x%08x\n", status));
        return status;
    }

    PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fido->DeviceExtension;
    RtlZeroMemory(pdx, sizeof(*pdx));

    // From this point forward, any error will have side effects that need to
    // be cleaned up. Using a try-finally block allows us to modify the program
    // easily without losing track of the side effects.

    __try
    {
        // finish initialization
        MyIoInitializeRemoveLock(&pdx->RemoveLock, 0, 0, 255);

        // Add our device object to the stack and propagate critical settings
        // from the immediately lower device object

        PDEVICE_OBJECT fdo = IoAttachDeviceToDeviceStack(fido, pdo);
        pdx->LowerDeviceObject = fdo;
        fido->Flags |= fdo->Flags & (DO_DIRECT_IO | DO_BUFFERED_IO | DO_POWER_PAGABLE | DO_POWER_INRUSH);
        fido->DeviceType = fdo->DeviceType;
        fido->Characteristics = fdo->Characteristics;
        fido->AlignmentRequirement = fdo->AlignmentRequirement;
        fido->StackSize = (CCHAR)(fdo->StackSize + 1);

        // Clear the "initializing" flag so that we can get IRPs
        fido->Flags &= ~DO_DEVICE_INITIALIZING;

        if(GlobalData.lStealEntrypoints)
        {
            // make a copy of fdo->DriverObject
            if(NULL == GlobalData.PatchedTable)
            {
                GlobalData.PatchedTable = (PDRIVER_OBJECT) ExAllocatePool(PagedPool, sizeof(DRIVER_OBJECT));
                if(NULL != GlobalData.PatchedTable)
                {
                    *GlobalData.PatchedTable = *fdo->DriverObject;

                    // we make some changes to this copy
                    GlobalData.PatchedTable->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = DispatchInternalIOCTL;
                    GlobalData.PatchedTable->MajorFunction[IRP_MJ_PNP]                     = DispatchPnp;

                    // here is the trick : we save the original DriverObject
                    // and next, it points to our modified copy
                    pdx->OriginalDriverObject = fdo->DriverObject;
                    fdo->DriverObject = GlobalData.PatchedTable;
                }
            }
        }
    }
    __finally
    {
        if (!NT_SUCCESS(status))
        {
            IoDeleteDevice(fido);
        }
    }

    // try several sizes of non-paged memory, if allocation doesn't 
    // succeed, try with half as much... but at least 512 bytes should
    // work, otherwise just give up    
    ULONG uBufSize = 1 << 20;
    PUCHAR pBuffer = NULL;
    while((NULL == pBuffer) && (uBufSize > 1<<9))
    {
        KdPrint(("USBSnoop - AddDevice(): trying to grab %d non-paged bytes... ", uBufSize));
        pBuffer = (PUCHAR) ExAllocatePool(NonPagedPool, uBufSize);
        if(NULL == pBuffer)
        {
            uBufSize = uBufSize >> 1;
        }
        KdPrint(("%s\n", NULL == pBuffer ? "failed":"success"));
    }

    if(NULL == pBuffer)
    {
        KdPrint(("USBSnoop - AddDevice(): ******* SERIOUS - couldn't get any memory for buffering!! ******\n"));
    }
    else
    {
        KdPrint(("USBSnoop - AddDevice(): grabbed %d bytes of non-paged memory ;-)\n", uBufSize));
    }

    pdx->uTimeStampZero = GetCurrentTimeStamp();
    pdx->LocalBuffer.Initialize(pBuffer, uBufSize);
    pdx->pCurrentBuffer = &pdx->LocalBuffer;
    pdx->bLoggingEnabled = TRUE;

    // if we didn't connect in DriverEntry (at load time), or if we got
    // kicked out in the meantime: try to get back to USBSnpys now.
    ConnectToUSBSnpys();
    if(NULL != GlobalData.Snoopies.pfnDeviceArrival)
    {
        CRingBuffer *pNewBuffer = GlobalData.Snoopies.pfnDeviceArrival(
            fido,                       // DeviceObject of ours
            hardwareid,                 // string with hardware id
            pdx->uTimeStampZero,        // Time of plugin
            pdx->pCurrentBuffer);
        if(NULL != pNewBuffer)
        {
            // this means that USBSnpys has taken ownership of the
            // ringbuffer...
            pdx->pCurrentBuffer = pNewBuffer;
        }
    }

    return status;
}

///////////////////////////////////////////////////////////////////////////////

#pragma LOCKEDCODE

NTSTATUS ConnectToUSBSnpys(void)
{
    if(NULL != GlobalData.Snoopies.pfnDeviceArrival)
    {
        // we are already connected...
        return STATUS_SUCCESS;
    }

    NTSTATUS status = STATUS_SUCCESS;
    SNOOPY_FUNCTIONS Snoopy;
    Snoopy.pfnReleaseSnoopies = ReleaseSnoopies;    
    Snoopy.pfnEnableLogging = EnableLogging;
    
    if(bIsThisWin9x)
    {
        // connect to a VxD
        VxDInfo = NULL;
	    struct VxD_Desc_Block *pDDB;
	    ULONG loaderReturn;
    	if(0 != (loaderReturn = VXDLDR_LoadDevice(&pDDB, &VxDInfo, USBSNPYS_WDMNAME_9X, TRUE)))
        {
            KdPrint(("USBSnoopies: Error 0x%%08x loading VxD >%s<\n", loaderReturn, USBSNPYS_WDMNAME_9X));
            return status;
        }        
        if(VxDInfo->DI_DDB)
        {
			DIOCPARAMETERS IoctlParams;
			DWORD dwBytesReturned = 0;
			IoctlParams.hDevice          = (DWORD)NULL;
			IoctlParams.dwIoControlCode  = (DWORD)USBSNOOPIES_HELLO;
			IoctlParams.lpvInBuffer      = (DWORD)&Snoopy;
			IoctlParams.cbInBuffer       = (DWORD)sizeof(Snoopy);
			IoctlParams.lpvOutBuffer     = (DWORD)&GlobalData.Snoopies;
			IoctlParams.cbOutBuffer      = (DWORD)sizeof(GlobalData.Snoopies);
			IoctlParams.lpcbBytesReturned= (DWORD)&dwBytesReturned;
			IoctlParams.lpoOverlapped    = (DWORD)NULL;

			if(0 == Directed_Sys_Control(
                VxDInfo->DI_DDB,
                W32_DEVICEIOCONTROL,
                0,
                0,
                (DWORD)&IoctlParams,
                (DWORD)&IoctlParams))
            {
				status = STATUS_SUCCESS;
			}
            else
            {
				status = STATUS_INVALID_PARAMETER;
			}
		}
        // we will unload the VxD as soon as we unload...
    }
    else
    {
        // connect to a NT4 driver
        UNICODE_STRING USBSnpysName;
        RtlInitUnicodeString(&USBSnpysName, USBSNPYS_SYMNAME);

        PFILE_OBJECT  SnoopiesFileObject = NULL;
        PDEVICE_OBJECT SnoopiesDeviceObject = NULL;
        status = IoGetDeviceObjectPointer(
            &USBSnpysName,
            FILE_ALL_ACCESS,
            &SnoopiesFileObject,
            &SnoopiesDeviceObject);
        if(NT_SUCCESS(status))
        {
            KdPrint(("USBSnoop - connected to USBSnpys...\n"));

            status = CallSnoopiesDriverSynchronously(SnoopiesDeviceObject,
                USBSNOOPIES_HELLO,
                &Snoopy, sizeof(Snoopy),
                &GlobalData.Snoopies, sizeof(GlobalData.Snoopies),
                TRUE);

            ObDereferenceObject(SnoopiesFileObject);
        }
    }

    KdPrint(("USBSnoop - IOCTL to snoopies returned %08x\n", status));
    return status;
}

void DisconnectFromUSBSnpys(PDEVICE_OBJECT fido, PDEVICE_EXTENSION pdx)
{
    // disconnect from USBSnpys service
    if(NULL != GlobalData.Snoopies.pfnDeviceRemoval)
    {
        // we transfer the ownership of the buffer over to USBSnpys
        GlobalData.Snoopies.pfnDeviceRemoval(fido);
    }
    else
    {
        // we free the memory, if nobody has take ownership of the
        // ringbuffer...
        if(pdx->pCurrentBuffer == &pdx->LocalBuffer)
        {
            ExFreePool(pdx->LocalBuffer.GetBase());
        }
    }
}

void ReleaseSnoopies(PDEVICE_OBJECT DeviceObject)
{
    // USBSnpys calls here to tell us, that it is being unloaded.
    // We just make sure that nobody is trying to access it anymore...
    RtlZeroMemory(&GlobalData.Snoopies, sizeof(GlobalData.Snoopies));

    if(NULL != DeviceObject)
    {
        // additionally, we might have given away the ownership of the
        // ringbuffer... in that case, invalidate our pointer to it
        PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
        pdx->bLoggingEnabled = FALSE;
        pdx->pCurrentBuffer = NULL;
    }
}

ULONG EnableLogging(PDEVICE_OBJECT DeviceObject, ULONG bEnable)
{
    PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    pdx->bLoggingEnabled = (0 != bEnable);
    // KdPrint(("USBSnoop - Logging is %sabled for device %08x\n", pdx->bLoggingEnabled ? "en" : "dis", DeviceObject));
    return pdx->bLoggingEnabled;
}

void CopyUserData(PVOID pDest, ULONG &uOffset, PVOID pSrc, ULONG uSize)
{
    // set a reasonable size limit, and "verify" src/dest pointers
    if((NULL != pDest) && (NULL != pSrc) && (uSize <= 1<<16))
    {
        PUCHAR pMyDest = ((PUCHAR) pDest) + uOffset;
        RtlCopyMemory(pMyDest, pSrc, uSize);
    }
    uOffset += uSize;
}

static UCHAR PointerPresent = 1;
static UCHAR PointerNull = 0;

void CopyTransferBuffer(PVOID pDest, ULONG &uOffset, PUCHAR pBuffer, PMDL pMdl, ULONG uSize)
{
    CopyUserData(pDest, uOffset, &PointerPresent, sizeof(PointerPresent));

    if(pBuffer)
    {
        if(pMdl)
        {
            // KdPrint(("??? weird transfer buffer, both MDL and flat specified. Ignoring MDL\n"));
        }
        CopyUserData(pDest, uOffset, &PointerPresent, sizeof(PointerPresent));
        CopyUserData(pDest, uOffset, &uSize, sizeof(ULONG));
        CopyUserData(pDest, uOffset, pBuffer, uSize);
    }
    else if(pMdl)
    {
        PUCHAR pMDLBuf = (PUCHAR)MmGetSystemAddressForMdl(pMdl);
        if(pMDLBuf)
        {
            CopyUserData(pDest, uOffset, &PointerPresent, sizeof(PointerPresent));
            CopyUserData(pDest, uOffset, &uSize, sizeof(ULONG));
            CopyUserData(pDest, uOffset, pMDLBuf, uSize);
        }
        else
        {
            CopyUserData(pDest, uOffset, &PointerNull, sizeof(PointerNull));
        }
    }
    else
    {
        CopyUserData(pDest, uOffset, &PointerNull, sizeof(PointerNull));
    }
}

void CopyNoTransferBuffer(PVOID pDest, ULONG &uOffset)
{
    CopyUserData(pDest, uOffset, &PointerNull, sizeof(PointerNull));
}

void CopyMemoryBlock(PVOID pDest, ULONG &uOffset, PVOID pBuffer, ULONG uOfs, ULONG uSize)
{
    if(pBuffer && (0 != uSize))
    {
        CopyUserData(pDest, uOffset, &uSize, sizeof(uSize));
        PUCHAR pData = (PUCHAR) pBuffer;
        CopyUserData(pDest, uOffset, &pData[uOfs], uSize);
    }
    else
    {
        uSize = 0;
        CopyUserData(pDest, uOffset, &uSize, sizeof(uSize));
    }
}

void CopyBufferOrMdl(PVOID pDest, ULONG &uOffset, PVOID pBuffer, PMDL pMdl, ULONG uOfs, ULONG uSize)
{
    if(!pBuffer)
    {
        if(pMdl)
        {
            pBuffer = MmGetSystemAddressForMdl(pMdl);
        }
    }
    CopyMemoryBlock(pDest, uOffset, pBuffer, uOfs, uSize);
}

ULONG DecodeContents(IN PURB pUrb, IN PUCHAR pDest, BOOLEAN bReturnedFromHCD)
{
    ULONG uUrbUserDataSize = 0;
    switch(pUrb->UrbHeader.Function)
    {
    case URB_FUNCTION_SELECT_CONFIGURATION:
        {
            struct _URB_SELECT_CONFIGURATION *pSelectConfiguration = (struct _URB_SELECT_CONFIGURATION*) pUrb;
            if(pSelectConfiguration->Hdr.Length >= FIELD_OFFSET(_URB_SELECT_CONFIGURATION, ConfigurationHandle))
            {
                PUSB_CONFIGURATION_DESCRIPTOR pCD = pSelectConfiguration->ConfigurationDescriptor;
                ULONG uCDLength = pCD ? sizeof(USB_CONFIGURATION_DESCRIPTOR) : 0;
                CopyUserData(pDest, uUrbUserDataSize, (PUCHAR) pCD, uCDLength);
            }
        }
        break;

    case URB_FUNCTION_SELECT_INTERFACE:
    case URB_FUNCTION_ABORT_PIPE:
    case URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL:
    case URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL:
    case URB_FUNCTION_GET_FRAME_LENGTH:
    case URB_FUNCTION_SET_FRAME_LENGTH:
    case URB_FUNCTION_GET_CURRENT_FRAME_NUMBER:
    case URB_FUNCTION_RESET_PIPE:
    case URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE:
    case URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT:
    case URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE:
    case URB_FUNCTION_SET_FEATURE_TO_DEVICE:
    case URB_FUNCTION_SET_FEATURE_TO_INTERFACE:
    case URB_FUNCTION_SET_FEATURE_TO_ENDPOINT:
    case URB_FUNCTION_SET_FEATURE_TO_OTHER:
    case URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE:
    case URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE:
    case URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT:
    case URB_FUNCTION_CLEAR_FEATURE_TO_OTHER:
    case URB_FUNCTION_GET_CONFIGURATION:
    case URB_FUNCTION_GET_INTERFACE:
        // no user data apart from the stuff in the URB itself
        break;

    case URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE:
    case URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT:
    case URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE:
    case URB_FUNCTION_GET_STATUS_FROM_DEVICE:
    case URB_FUNCTION_GET_STATUS_FROM_INTERFACE:
    case URB_FUNCTION_GET_STATUS_FROM_ENDPOINT:
    case URB_FUNCTION_GET_STATUS_FROM_OTHER:
    case URB_FUNCTION_VENDOR_DEVICE:
    case URB_FUNCTION_VENDOR_INTERFACE:
    case URB_FUNCTION_VENDOR_ENDPOINT:
    case URB_FUNCTION_VENDOR_OTHER:
    case URB_FUNCTION_CLASS_DEVICE:
    case URB_FUNCTION_CLASS_INTERFACE:
    case URB_FUNCTION_CLASS_ENDPOINT:
    case URB_FUNCTION_CLASS_OTHER:
    case URB_FUNCTION_CONTROL_TRANSFER:
        {
            struct _URB_CONTROL_TRANSFER   *pControlTransfer = (struct _URB_CONTROL_TRANSFER *) pUrb;
            BOOLEAN bReadFromDevice = (BOOLEAN)(pControlTransfer->TransferFlags & USBD_TRANSFER_DIRECTION_IN);
            if(((!bReadFromDevice) && (!bReturnedFromHCD)) || (bReadFromDevice && bReturnedFromHCD))
            {
                CopyTransferBuffer(pDest, uUrbUserDataSize, 
                    (PUCHAR) pControlTransfer->TransferBuffer, 
                    pControlTransfer->TransferBufferMDL, 
                    pControlTransfer->TransferBufferLength);
            }
            else
            {
                CopyNoTransferBuffer(pDest, uUrbUserDataSize);
            }
        }
        break;

    case URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER:
        {
            struct _URB_BULK_OR_INTERRUPT_TRANSFER *pBulkOrInterruptTransfer = (struct _URB_BULK_OR_INTERRUPT_TRANSFER *) pUrb;
            BOOLEAN bReadFromDevice = (BOOLEAN)(pBulkOrInterruptTransfer->TransferFlags & USBD_TRANSFER_DIRECTION_IN);
            if(((!bReadFromDevice) && (!bReturnedFromHCD)) || (bReadFromDevice && bReturnedFromHCD))
            {
                CopyTransferBuffer(pDest, uUrbUserDataSize,
                    (PUCHAR) pBulkOrInterruptTransfer->TransferBuffer,
                    pBulkOrInterruptTransfer->TransferBufferMDL,
                    pBulkOrInterruptTransfer->TransferBufferLength);
            }
            else
            {
                CopyNoTransferBuffer(pDest, uUrbUserDataSize);
            }
        }
        break;

    case URB_FUNCTION_ISOCH_TRANSFER:
        {
            struct _URB_ISOCH_TRANSFER *pIsochTransfer = (struct _URB_ISOCH_TRANSFER *) pUrb;
            BOOLEAN bReadFromDevice = (BOOLEAN)(pIsochTransfer->TransferFlags & USBD_TRANSFER_DIRECTION_IN);
            if((!bReadFromDevice) && (!bReturnedFromHCD))
            {
                ULONG nPacketCnt = 1;
                CopyUserData(pDest, uUrbUserDataSize, &nPacketCnt, sizeof(nPacketCnt));
                CopyBufferOrMdl(pDest, uUrbUserDataSize, 
                    pIsochTransfer->TransferBuffer,
                    pIsochTransfer->TransferBufferMDL,
                    0,
                    pIsochTransfer->TransferBufferLength);
            }
            else if(bReadFromDevice && bReturnedFromHCD)
            {
                ULONG nPacketCnt = pIsochTransfer->NumberOfPackets;
                CopyUserData(pDest, uUrbUserDataSize, &nPacketCnt, sizeof(nPacketCnt));
                for(ULONG nPacket = 0; nPacket < nPacketCnt; ++nPacket)
                {
                    CopyBufferOrMdl(pDest, uUrbUserDataSize,
                        pIsochTransfer->TransferBuffer,
                        pIsochTransfer->TransferBufferMDL,
                        pIsochTransfer->IsoPacket[nPacket].Offset,
                        pIsochTransfer->IsoPacket[nPacket].Length);
                }
            }
            else
            {
                ULONG nPacketCnt = 0;
                CopyUserData(pDest, uUrbUserDataSize, &nPacketCnt, sizeof(nPacketCnt));
            }
        }
        break;

    default:
        KdPrint(("******* unhandled URB with function code 0x%04x ********\n", pUrb->UrbHeader.Function));
        uUrbUserDataSize = 0;
        break;
    }

    return uUrbUserDataSize;

}

NTSTATUS AddPacket(IN PDEVICE_OBJECT fido, IN PURB pUrb, ULONG uSequenceNumber, BOOLEAN bReturnedFromHCD)
{
    PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fido->DeviceExtension;
    if(!pdx->bLoggingEnabled)
    {
        //KdPrint(("USBSnoop - Logging is disabled\n"));
        return STATUS_SUCCESS;
    }

    ULONG uUrbSize = pUrb->UrbHeader.Length;
    if(uUrbSize < sizeof(_URB_HEADER))
    {
        KdPrint(("USBSnoop - Urb @ 0x%08x is short (length: %d, function: %04x)!\n", pUrb, pUrb->UrbHeader.Length, pUrb->UrbHeader.Function));
        uUrbSize = sizeof(URB);
    }

    ULONG uSerializedUrbContentsSize = DecodeContents(pUrb, NULL, bReturnedFromHCD);
    ULONG uTotalUrbSize = FIELD_OFFSET(PACKET_HEADER, UrbHeader) + uUrbSize + uSerializedUrbContentsSize;
    PPACKET_HEADER pPacket = (PPACKET_HEADER) ExAllocatePool(NonPagedPool, uTotalUrbSize);
    if(pPacket)
    {
        pPacket->uLen = uTotalUrbSize;
        pPacket->uSequenceNumber = uSequenceNumber;
        pPacket->bDirection = bReturnedFromHCD ? DIRECTION_FROM_DEVICE : DIRECTION_TO_DEVICE;
        pPacket->uTimeStamp = GetCurrentTimeStamp();
        
        // fill in the struct _URB_... part
        RtlCopyMemory(&pPacket->UrbHeader, pUrb, uUrbSize);
        
        // fill in the serialized URB contents (user data)
        PUCHAR pData = ((PUCHAR)&pPacket->UrbHeader) + uUrbSize;
        DecodeContents(pUrb, pData, bReturnedFromHCD);

        if(NULL != pdx->pCurrentBuffer)
        {
            if(!pdx->pCurrentBuffer->WriteBytes(pPacket, uTotalUrbSize))
            {
                KdPrint(("USBSnoop - Overflow occurred (URB %d couldn't be added anymore)!\n",uSequenceNumber));
                pdx->pCurrentBuffer->SetOverflowOccurred(true);
            }
        }
        ExFreePool(pPacket);
    }

    return STATUS_SUCCESS;

}

NTSTATUS CompleteRequest(IN PIRP Irp, IN NTSTATUS status, IN ULONG *info)
{
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = (ULONG) info;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

///////////////////////////////////////////////////////////////////////////////

#pragma LOCKEDCODE

NTSTATUS DispatchAny(IN PDEVICE_OBJECT fido, IN PIRP Irp)
{
    PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fido->DeviceExtension;

    if(GlobalData.lStealEntrypoints)
    {
        // special case: we have a unload race here if we block...
        // this will potentially invalidate all the nice things we get
        // from using the remove lock...
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(pdx->LowerDeviceObject, Irp);
    }

    // Pass request down without additional processing
    NTSTATUS status = MyIoAcquireRemoveLock(&pdx->RemoveLock, Irp);
    if (!NT_SUCCESS(status))
    {
        return CompleteRequest(Irp, status, 0);
    }
    IoSkipCurrentIrpStackLocation(Irp);
    status = IoCallDriver(pdx->LowerDeviceObject, Irp);
    MyIoReleaseRemoveLock(&pdx->RemoveLock, Irp);
    return status;
}

NTSTATUS InternalIOCTLCompletion(IN PDEVICE_OBJECT fido, IN PIRP Irp, IN PVOID Context)
{
    if(Irp->PendingReturned)
    {
        IoMarkIrpPending(Irp);
    }
    
    ULONG uSequenceNumber = (ULONG)Context;
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG dwControlCode = stack->Parameters.DeviceIoControl.IoControlCode;
    if(IOCTL_INTERNAL_USB_SUBMIT_URB == dwControlCode)
    {
        PURB pUrb = (PURB)stack->Parameters.Others.Argument1;
        AddPacket(fido, pUrb, uSequenceNumber, TRUE);
    }
    // TODO: check this line with the one in the downward irp handler!
    //MyIoReleaseRemoveLock(&pdx->RemoveLock, Irp);
    return STATUS_SUCCESS;
}

NTSTATUS StealingInternalIOCTLCompletion(IN PDEVICE_OBJECT fido, IN PIRP Irp, IN PVOID inContext)
{
    //KdPrint(("UsbSnoop - StealingInternalIOCTLCompletion(%p): fido=%p, Irp=%p, Context=%p\n", fido, Irp, inContext));
    
    PCONTEXT Context = (PCONTEXT)inContext;
   
    // restoring information.
    Context->Stack->CompletionRoutine = Context->CompletionRoutine;
    Context->Stack->Context           = Context->Context;
    Context->Stack->Control           = Context->Control;
    PDEVICE_OBJECT snoop_fido         = Context->SnoopFido;
    
    // dumping URB
    AddPacket(snoop_fido, Context->pUrb, Context->uSequenceNumber, TRUE);
    
    // saving some field before freeing the structure
    PVOID OldContext = Context->Context;
    PIO_COMPLETION_ROUTINE OldCompletionRoutine = Context->CompletionRoutine;
    
    // freeing the allocated structure
    ExFreePool(Context);
    
    // calling the old CompletionRoutine, if there was one
    if(NULL != OldCompletionRoutine)
    {
        return OldCompletionRoutine(fido, Irp, OldContext);
    }
    
    return STATUS_SUCCESS;
}


NTSTATUS DispatchInternalIOCTL(IN PDEVICE_OBJECT fido, IN PIRP Irp)
{
    PDEVICE_OBJECT snoop_fido = fido;
    if(GlobalData.lStealEntrypoints)
    {
        snoop_fido = fido->AttachedDevice;
    }

    PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) snoop_fido->DeviceExtension;
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG dwControlCode = stack->Parameters.DeviceIoControl.IoControlCode;

    NTSTATUS status = MyIoAcquireRemoveLock(&pdx->RemoveLock, Irp);
    if(!NT_SUCCESS(status))
    {
        return CompleteRequest(Irp, status, 0);
    }

    if((IOCTL_INTERNAL_USB_SUBMIT_URB == dwControlCode) && (pdx->bLoggingEnabled))
    {
        ULONG uSequenceNumber = InterlockedIncrement((PLONG) &pdx->uSequenceNumber);
        PURB pUrb = (PURB) stack->Parameters.Others.Argument1;
        AddPacket(snoop_fido, pUrb, uSequenceNumber, FALSE);
        
        if(GlobalData.lStealEntrypoints)
        {
            PCONTEXT Context = (PCONTEXT) ExAllocatePool(PagedPool, sizeof(CONTEXT));
            if(NULL != Context)
            {
                Context->CompletionRoutine = stack->CompletionRoutine;
                Context->Context           = stack->Context;
                Context->Control           = stack->Control;
                Context->pUrb              = pUrb;
                Context->uSequenceNumber   = uSequenceNumber;
                Context->Stack             = stack;
                Context->SnoopFido         = snoop_fido;
                
                stack->CompletionRoutine = StealingInternalIOCTLCompletion;
                stack->Context = Context;
                stack->Control = SL_INVOKE_ON_SUCCESS | SL_INVOKE_ON_ERROR | SL_INVOKE_ON_CANCEL;
            }
            else
            {
                KdPrint(("USBSnoop - ExAllocatePool failed! Can't redirect CompletionRoutine\n"));
            }
            if(NULL != pdx->OriginalDriverObject)
                status = pdx->OriginalDriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL](fido, Irp);
            else
                status = CompleteRequest(Irp, STATUS_SUCCESS, 0);
        }
        else
        {
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(Irp, InternalIOCTLCompletion, (PVOID) uSequenceNumber, TRUE, TRUE, TRUE);
            status = IoCallDriver(pdx->LowerDeviceObject, Irp);
        }
    }
    else
    {
        if(GlobalData.lStealEntrypoints)
        {
            if(NULL != pdx->OriginalDriverObject)
                status = pdx->OriginalDriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL](fido, Irp);
            else
                status = CompleteRequest(Irp, STATUS_SUCCESS, 0);
        }
        else
        {
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(pdx->LowerDeviceObject, Irp);
        }
    }
    MyIoReleaseRemoveLock(&pdx->RemoveLock, Irp);
    
    return status;
}

///////////////////////////////////////////////////////////////////////////////

NTSTATUS DispatchPower(IN PDEVICE_OBJECT fido, IN PIRP Irp)
{
#if DBG
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG fcn = stack->MinorFunction;

    static char* fcnname[] = {
        "IRP_MN_WAIT_WAKE",
        "IRP_MN_POWER_SEQUENCE",
        "IRP_MN_SET_POWER",
        "IRP_MN_QUERY_POWER",
    };
    
    if (fcn == IRP_MN_SET_POWER || fcn == IRP_MN_QUERY_POWER)
    {
        static char* sysstate[] = {
            "PowerSystemUnspecified",
            "PowerSystemWorking",
            "PowerSystemSleeping1",
            "PowerSystemSleeping2",
            "PowerSystemSleeping3",
            "PowerSystemHibernate",
            "PowerSystemShutdown",
            "PowerSystemMaximum",
        };
        
        static char* devstate[] = {
            "PowerDeviceUnspecified",
            "PowerDeviceD0",
            "PowerDeviceD1",
            "PowerDeviceD2",
            "PowerDeviceD3",
            "PowerDeviceMaximum",
        };
        
        ULONG context = stack->Parameters.Power.SystemContext;
        POWER_STATE_TYPE type = stack->Parameters.Power.Type;
        
        KdPrint(("USBSnoop - IRP_MJ_POWER (%s), SystemContext %X", fcnname[fcn], context));
        if (type == SystemPowerState)
            KdPrint((", SystemPowerState = %s\n", sysstate[stack->Parameters.Power.State.SystemState]));
        else
            KdPrint((", DevicePowerState = %s\n", devstate[stack->Parameters.Power.State.DeviceState]));
    }
    else
        KdPrint(("USBSnoop - IRP_MJ_POWER (%s)\n", fcnname[fcn]));
#endif // DBG

    PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fido->DeviceExtension;
    PoStartNextPowerIrp(Irp);   // must be done while we own the IRP
    
    NTSTATUS status = MyIoAcquireRemoveLock(&pdx->RemoveLock, Irp);
    if (!NT_SUCCESS(status))
        return CompleteRequest(Irp, status, 0);
    
    IoSkipCurrentIrpStackLocation(Irp);
    status = PoCallDriver(pdx->LowerDeviceObject, Irp);
    MyIoReleaseRemoveLock(&pdx->RemoveLock, Irp);
    
    return status;
}

///////////////////////////////////////////////////////////////////////////////

NTSTATUS DispatchPnp(IN PDEVICE_OBJECT fido, IN PIRP Irp)
{
    PDEVICE_OBJECT snoop_fido = fido;
    if(GlobalData.lStealEntrypoints)
    {
        snoop_fido = fido->AttachedDevice;
    }

    PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) snoop_fido->DeviceExtension;
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG dwMinorFunction = stack->MinorFunction;
    
    NTSTATUS status = MyIoAcquireRemoveLock(&pdx->RemoveLock, Irp);
    if(!NT_SUCCESS(status))
    {
        return CompleteRequest(Irp, status, 0);
    }
    
    if(GlobalData.lStealEntrypoints)
    {
        if(NULL != pdx->OriginalDriverObject)
            status = pdx->OriginalDriverObject->MajorFunction[IRP_MJ_PNP](fido, Irp);
        else
            status = CompleteRequest(Irp, STATUS_SUCCESS, 0);
    }
    else
    {
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(pdx->LowerDeviceObject, Irp);
    }

    if(IRP_MN_REMOVE_DEVICE == dwMinorFunction)
    {
        MyIoReleaseRemoveLockAndWait(&pdx->RemoveLock, Irp);
        if (pdx->LowerDeviceObject)
        {
            IoDetachDevice(pdx->LowerDeviceObject);
        }
        DisconnectFromUSBSnpys(snoop_fido, pdx);
        IoDeleteDevice(snoop_fido);
    }
    else
    {
        MyIoReleaseRemoveLock(&pdx->RemoveLock, Irp);
    }

    return status;
}

//////////////////////////////////////////////////////////////////////////

#pragma LOCKEDCODE

extern "C" void __declspec(naked) __cdecl _chkesp()
{
    _asm je okay
        ASSERT(!"Stack pointer mismatch!");
okay:
    _asm ret
}

//** end of DriverEntry.cpp **********************************************
/*************************************************************************

  $Log: not supported by cvs2svn $
 * 
 * 4     2/22/02 6:16p Rbosa
 * - fixed the SHORT URB problem by packing in more bytes (still not
 * clean, but works most of the time)
 * - added right gathering of isoch in packets... should really clean up
 * all those copy*** functions
 * 
 * 3     2/05/02 9:27p Rbosa
 * - renamed misleading fido to DriverObject
 * 
 * 2     2/05/02 9:06p Rbosa
 * - added Win9X support
 * 
 * 1     1/25/02 2:45p Rbosa

*************************************************************************/
