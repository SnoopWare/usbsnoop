//************************************************************************
//
// USBSnoopies_Common.inl
// 
// Use by USBSnpys and USBSnpyV kernel drivers.
//
//************************************************************************


NTSTATUS IOCTL_GetSnoopedDevs(PVOID outBuffer, ULONG outputBufferLength)
{
    KdPrint(("USBSnpys - IOCTL: USBSNOOP_GET_SNOOPED_DEVS\n"));
    if(!outBuffer)
    {
        KdPrint(("USBSnpys - ERROR: no output buffer!\n"));
        return STATUS_INVALID_PARAMETER;
    }

    if(outputBufferLength != sizeof(SNOOPED_DEVICES))
    {
        KdPrint(("USBSnpys - ERROR: output buffer not big enough! (need SNOOPED_DEVICES %d, got %d)\n",sizeof(SNOOPED_DEVICES),outputBufferLength));
        return STATUS_INVALID_PARAMETER;
    }
    
    RtlCopyMemory(outBuffer, &GlobalData.Snooped, sizeof(SNOOPED_DEVICES));
    return STATUS_SUCCESS;
}

int IsDIDRegistered(ULONG DeviceID, int *nIndex)
{
    KdPrint(("USBSnpys - IsDIDRegistered\n"));
    for(ULONG nSnooped = 0; nSnooped < GlobalData.Snooped.uCount; ++nSnooped)
    {
        if(GlobalData.Snooped.Entry[nSnooped].uDeviceID == DeviceID)
        {
            if(NULL != nIndex)
            {
                *nIndex = nSnooped;
            }
            return TRUE;
        }
    }
    return FALSE;
}

int IsDORegistered(PDEVICE_OBJECT DeviceObject, int *nIndex)
{
    KdPrint(("USBSnpys - IsDORegistered\n"));
    for(ULONG nSnooped = 0; nSnooped < GlobalData.Snooped.uCount; ++nSnooped)
    {
        if(GlobalData.Snooped.Entry[nSnooped].DeviceObject == DeviceObject)
        {
            if(NULL != nIndex)
            {
                *nIndex = nSnooped;
            }
            return TRUE;
        }
    }

    return FALSE;
}

LONG RemovePacket(IN CRingBuffer *pRingBuffer, PUCHAR pOutBuffer, PULONG pBufferSizeLeft)
{
    ASSERT(NULL != pOutBuffer);
    ASSERT(NULL != pBufferSizeLeft);
    ASSERT(NULL != pRingBuffer);
    KdPrint(("USBSnpys - RemovePacket, %d bytes left\n",*pBufferSizeLeft));

    ULONG uPacketLen = 0;
    if(pRingBuffer->PeekBytes(&uPacketLen, sizeof(ULONG)))
    {
        if(uPacketLen < *pBufferSizeLeft)
        {
            if(pRingBuffer->ReadBytes(pOutBuffer, uPacketLen))
            {
                *pBufferSizeLeft -= uPacketLen;
			    KdPrint(("USBSnpys - RemovePacket %d bytes\n",uPacketLen));
                return uPacketLen;
            }
	    } else {
		    KdPrint(("USBSnpys - Arrrgg!!! No space left in buffer : - packet = %d bytes, buffer left = %d bytes - Need to increase buffer in SnoopyPro application IoControll call!!\n",uPacketLen,*pBufferSizeLeft));
		}

    } else {
	    KdPrint(("USBSnpys - pRingBuffer->PeekBytes returned zero bytes\n"));
	}

    KdPrint(("USBSnpys - RemovePacket 0 bytes\n"));
    return 0;
}

CRingBuffer *DeviceArrival(PDEVICE_OBJECT DeviceObject, 
                           char *HardwareID, 
                           ULONG uTimeStampZero, 
                           CRingBuffer *pRingBuffer)
{
    if(GlobalData.Snooped.uCount < MAX_SNOOPY_DRIVERS - 1)
    {
        KdPrint(("USBSnpys - DeviceArrival: DO: 0x%08x on index %d\n", DeviceObject, GlobalData.Snooped.uCount));
        KdPrint(("USBSnpys -   ... HardwareID: %S\n", HardwareID));
        GlobalData.HasBeenRemoved[GlobalData.Snooped.uCount] = FALSE;
        PSNOOPED_DEVICE pSnooped = &GlobalData.Snooped.Entry[GlobalData.Snooped.uCount];
        pSnooped->uDeviceID = GlobalData.nNextDeviceID;
        InterlockedIncrement(&GlobalData.nNextDeviceID);
        pSnooped->DeviceObject = DeviceObject;
        RtlZeroMemory(pSnooped->sHardwareIDs, MAX_PATH);
        RtlCopyMemory(pSnooped->sHardwareIDs, HardwareID, MAX_PATH - 1);
        pSnooped->uTimeStampZero = uTimeStampZero;
        RtlCopyMemory(&pSnooped->Buffer, pRingBuffer, sizeof(CRingBuffer));
        ++GlobalData.Snooped.uCount;
        return &pSnooped->Buffer;
    }

    return NULL;
}

ULONG DeviceRemoval(PDEVICE_OBJECT DeviceObject)
{
    int nSnooped = 0;
    if(IsDORegistered(DeviceObject, &nSnooped))
    {
        KdPrint(("USBSnpys - DeviceRemoval of DO 0x%08x on index %d\n", DeviceObject, nSnooped));
        GlobalData.HasBeenRemoved[nSnooped] = TRUE;
        return 1;
    }

    return 0;
}

void USBSnoopUnloading(void)
{
    KdPrint(("USBSnpys - USBSnoopUnloading\n"));
    // if USBSnoop is unloading, then all the buffers are ours to keep...
    for(ULONG nSnooped = 0; nSnooped < GlobalData.Snooped.uCount; ++nSnooped)
    {
        GlobalData.HasBeenRemoved[nSnooped] = TRUE;
    }

    // make sure we don't call into USBSnoop anymore...
    RtlZeroMemory(&GlobalData.Snoopy, sizeof(GlobalData.Snoopy));
}

void DeviceDelete(LONG nDeviceID)
{
    int nSnooped = 0;
    if(IsDIDRegistered(nDeviceID, &nSnooped))
    {
        KdPrint(("USBSnpys - DeviceDelete of DID 0x%08x on index %d\n", nDeviceID, nSnooped));
        if(NULL != GlobalData.Snooped.Entry[nSnooped].Buffer.GetBase())
        {
            ExFreePool(GlobalData.Snooped.Entry[nSnooped].Buffer.GetBase());
        }
        --GlobalData.Snooped.uCount;
        if(0 < GlobalData.Snooped.uCount)
        {
            PSNOOPED_DEVICE pDeleted = &GlobalData.Snooped.Entry[nSnooped];
            PSNOOPED_DEVICE pLast = &GlobalData.Snooped.Entry[GlobalData.Snooped.uCount];
            KdPrint(("USBSnpys -  ... new DID on index %d is: 0x%08x\n", nSnooped, pLast->uDeviceID));
            RtlCopyMemory(pDeleted, pLast, sizeof(SNOOPED_DEVICE));
            GlobalData.HasBeenRemoved[nSnooped] = GlobalData.HasBeenRemoved[GlobalData.Snooped.uCount];
        }
        else
        {
            KdPrint(("USBSnpys -  ... no more DID's around...\n"));

            // safe to assume that USBSnoop should be gone now...
            USBSnoopUnloading();
        }
    }
}

NTSTATUS IOCTL_EnableLogging(PVOID inBuffer, ULONG inputBufferLength)
{
    KdPrint(("USBSnpys - IOCTL: USBSNOOP_ENABLE_LOGGING\n"));
    if(!inBuffer)
    {
        KdPrint(("USBSnpys - ERROR: no inbuffer!\n"));
        return STATUS_INVALID_PARAMETER;
    }
    if(inputBufferLength != sizeof(ENABLE_LOGGING))
    {
        KdPrint(("USBSnpys - ERROR: input buffer too small!\n"));
        return STATUS_INVALID_PARAMETER;
    }

    PENABLE_LOGGING pEnableLogging = (PENABLE_LOGGING) inBuffer;
    ULONG uDeviceID = pEnableLogging->uDeviceID;
    int nIndex = 0;
    if(!IsDIDRegistered(uDeviceID, &nIndex))
    {
        KdPrint(("USBSnpys - ERROR: unknown device id 0x%08x!\n", uDeviceID));
        return STATUS_INVALID_PARAMETER;
    }

    ULONG bEnable = pEnableLogging->bEnable;
    if(pEnableLogging->bStopAndForget)
    {
        KdPrint(("USBSnpys - Forgetting about device id 0x%08x...\n", uDeviceID));
        if(!GlobalData.HasBeenRemoved[nIndex])
        {
            if(NULL != GlobalData.Snoopy.pfnReleaseSnoopies)
            {
                GlobalData.Snoopy.pfnReleaseSnoopies(GlobalData.Snooped.Entry[nIndex].DeviceObject);
            }
        }
        DeviceDelete(uDeviceID);
        return STATUS_SUCCESS;
    }

    if(GlobalData.HasBeenRemoved[nIndex])
    {
        KdPrint(("USBSnpys - ERROR: the specified device has been removed already!\n"));
        return STATUS_INVALID_PARAMETER;
    }

    if(NULL == GlobalData.Snoopy.pfnEnableLogging)
    {
        KdPrint(("USBSnpys - ERROR: no function pointer over to UsbSnoopy!\n"));
        return STATUS_INVALID_PARAMETER;
    }

    // since it hasn't been removed yet, the function pointers
    // will be still valid (since USBSnoop hasn't been unloaded yet)
    PDEVICE_OBJECT pDO = GlobalData.Snooped.Entry[nIndex].DeviceObject;
    GlobalData.Snoopy.pfnEnableLogging(pDO, bEnable);
    return STATUS_SUCCESS;
}

NTSTATUS IOCTL_GetURBs(PVOID inBuffer, ULONG inputBufferLength, PVOID outBuffer, ULONG outputBufferLength)
{
            
    KdPrint(("USBSnpys - IOCTL: USBSNOOP_GET_URBS, inBufLen %d, outBufLen %d\n",inputBufferLength,outputBufferLength));
    if(!inBuffer)
    {
        KdPrint(("USBSnpys - ERROR: no input buffer!\n"));
        return STATUS_INVALID_PARAMETER;
    }
    if(!outBuffer)
    {
        KdPrint(("USBSnpys - ERROR: no output buffer!\n"));
        return STATUS_INVALID_PARAMETER;
    }
    if(inputBufferLength < sizeof(GET_URBS))
    {
        KdPrint(("USBSnpys - ERROR: input buffer not big enough! (need GET_URBS %d, got %d)\n",sizeof(GET_URBS),inputBufferLength));
        return STATUS_INVALID_PARAMETER;
    }
    if(outputBufferLength < sizeof(GET_URBS))
    {
        KdPrint(("USBSnpys - ERROR: output buffer not big enough! (need GET_URBS %d, got %d)\n",sizeof(GET_URBS),outputBufferLength));
        return STATUS_INVALID_PARAMETER;
    }
    
    PGET_URBS pInput = (PGET_URBS) inBuffer;
    PGET_URBS pOutput = (PGET_URBS) outBuffer;
    
    LONG nDeviceID = pInput->uDeviceID;
    int nIndex = 0;
    if(!IsDIDRegistered(nDeviceID, &nIndex))
    {
        KdPrint(("USBSnpys - ERROR: unknown device id 0x%08x (device already gone?)\n", nDeviceID));
        return STATUS_INVALID_PARAMETER;
    }
  
    PSNOOPED_DEVICE pEntry = &GlobalData.Snooped.Entry[nIndex];
    CRingBuffer *pRingBuffer = &pEntry->Buffer;
    PUCHAR pParamOutput = (PUCHAR)(&pOutput[1]);
    ULONG BufferSizeLeft = pInput->nBufferSize;
    ULONG nNofPackets = 0;
    LONG lBytesRead = RemovePacket(pRingBuffer, pParamOutput, &BufferSizeLeft);
    while(0 != lBytesRead)
    {
        nNofPackets++;
        pParamOutput += lBytesRead;
        lBytesRead = RemovePacket(pRingBuffer, pParamOutput, &BufferSizeLeft);
    }
    
    pOutput->nNumOfPackets = nNofPackets;
    pOutput->nBufferSize = BufferSizeLeft;
    
    // check if the device was already removed and we're just
    // serving the last couple of bits in the buffer...
    pOutput->nFlags = pInput->nFlags;
    
    if(pRingBuffer->GetOverflowOccured())
    {
        pOutput->nFlags |= GETURBS_FLAG_OVERFLOW_OCCURRED;
    }
    
    if(GlobalData.HasBeenRemoved[nIndex])
    {
        pOutput->nFlags |= GETURBS_FLAG_DEVICE_GONE;
        if(0 == pRingBuffer->GetUsedSize())
        {
            // all gone, cleanup everything
            KdPrint(("USBSnpys - buffer for 0x%08x is empty now...\n", nDeviceID));
            KdPrint(("USBSnpys - getting rid of everything belonging to DID 0x%08x\n", pEntry->uDeviceID));
            DeviceDelete(pEntry->uDeviceID);
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS IOCTL_GetBufferFullness(PVOID inBuffer, ULONG inputBufferLength, PVOID outBuffer, ULONG outputBufferLength)
{
    KdPrint(("USBSnpys - IOCTL: GetBufferFullness\n"));
    if(!inBuffer)
    {
        KdPrint(("USBSnpys - ERROR: no input buffer!\n"));
        return STATUS_INVALID_PARAMETER;
    }
    if(!outBuffer)
    {
        KdPrint(("USBSnpys - ERROR: no output buffer!\n"));
        return STATUS_INVALID_PARAMETER;
    }
    if(inputBufferLength != sizeof(GET_BUFFER_FULLNESS))
    {
        KdPrint(("USBSnpys - ERROR: input buffer not big enough! (need GET_BUFFER_FULLNESS %d, got %d)\n",sizeof(GET_BUFFER_FULLNESS),inputBufferLength));
        return STATUS_INVALID_PARAMETER;
    }
    if(outputBufferLength != sizeof(GET_BUFFER_FULLNESS))
    {
        KdPrint(("USBSnpys - ERROR: output buffer not big enough! (need GET_BUFFER_FULLNESS %d, got %d)\n",sizeof(GET_BUFFER_FULLNESS),outputBufferLength));
        return STATUS_INVALID_PARAMETER;
    }
    
    PGET_BUFFER_FULLNESS pInput = (PGET_BUFFER_FULLNESS) inBuffer;
    PGET_BUFFER_FULLNESS pOutput = (PGET_BUFFER_FULLNESS) outBuffer;
    
    LONG nDeviceID = pInput->uDeviceID;
    int nIndex = 0;
    if(!IsDIDRegistered(nDeviceID, &nIndex))
    {
        KdPrint(("USBSnpys - ERROR: unknown device id 0x%08x (device already gone?)\n", nDeviceID));
        return STATUS_INVALID_PARAMETER;
    }
    
    pOutput->nBufferTotalSize = GlobalData.Snooped.Entry[nIndex].Buffer.GetTotalSize();
    pOutput->nBufferUsedSize = GlobalData.Snooped.Entry[nIndex].Buffer.GetUsedSize();
    
    return STATUS_SUCCESS;
}

NTSTATUS IOCTL_Hello(PVOID inBuffer, ULONG inputBufferLength, PVOID outBuffer, ULONG outputBufferLength)
{
    KdPrint(("USBSnpys - somebody is saying hello to me.\n"));
    KdPrint(("USBSnpys - exchanging function pointers...\n"));

    PSNOOPY_FUNCTIONS SnoopyFunctions = (PSNOOPY_FUNCTIONS) inBuffer;
    if(sizeof(SNOOPY_FUNCTIONS) == inputBufferLength)
    {
        GlobalData.Snoopy.pfnReleaseSnoopies = SnoopyFunctions->pfnReleaseSnoopies;
        GlobalData.Snoopy.pfnEnableLogging = SnoopyFunctions->pfnEnableLogging;

        PSNOOPIES_FUNCTIONS MyFunctions = (PSNOOPIES_FUNCTIONS) outBuffer;
        if(sizeof(SNOOPIES_FUNCTIONS) == outputBufferLength)
        {
            MyFunctions->pfnDeviceArrival = DeviceArrival;
            MyFunctions->pfnDeviceRemoval = DeviceRemoval;
            MyFunctions->pfnUSBSnoopUnloading = USBSnoopUnloading;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_INVALID_PARAMETER;
}

//** end of USBSnoopies_Common.inl ***************************************
/*************************************************************************

  $Log: USBSnoopies_Common.inl,v $
  Revision 1.1  2002/08/14 23:00:58  rbosa
  shared code between the application and driver and drivers themselves...


*************************************************************************/
