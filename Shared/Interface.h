#ifndef INTERFACE_H
#define INTERFACE_H

#pragma pack(push)
#pragma pack(1)

#define USBSNPYS_NTNAME                 L"\\Device\\USBSnpys"
#define USBSNPYS_SYMNAME                L"\\DosDevices\\USBSnpys"
#define USBSNPYS_W32NAME_2K             "\\\\.\\USBSnpys"
#define USBSNPYS_W32NAME_9X             "\\\\.\\USBSnpyV.VxD"
#define USBSNPYS_WDMNAME_9X             "USBSnpyV.VxD"

#ifndef CTL_CODE
#include <WinIOCTL.h>
#endif

#define USBSNOOP_CODE(_x_)              CTL_CODE(                   \
                                            FILE_DEVICE_UNKNOWN,    \
                                            (0x800 | _x_),          \
                                            METHOD_BUFFERED,        \
                                            FILE_ANY_ACCESS         \
                                        )

#define USBSNOOP_GET_SNOOPED_DEVS       USBSNOOP_CODE(0)
#define USBSNOOP_ENABLE_LOGGING         USBSNOOP_CODE(1)
#define USBSNOOP_GET_URBS               USBSNOOP_CODE(2)
#define USBSNOOP_GET_BUFFER_FULLNESS    USBSNOOP_CODE(3)
#define USBSNOOPIES_HELLO               USBSNOOP_CODE(42)

#define GETURBS_FLAG_DEVICE_GONE        (1)
#define GETURBS_FLAG_OVERFLOW_OCCURRED  (2)

#define MAX_SNOOPY_DRIVERS              (32)

#ifndef MAX_PATH
#define MAX_PATH                        (260)
#endif

#define HWID_MAX_PATH                   (260)

#define INVALID_DEVICE_ID               (0xdeadbabe)


//////////////////////////////////////////////////////////////////////////
// structures used in IOCTLs


#include "RingBuffer.h"

typedef struct SNOOPED_DEVICE
{
    // this one contains a unique ID for each snooped device session...
    // this is what ring3 will see afterwards
    ULONG uDeviceID;

    // this one contains the DeviceObjects of each filter known
    PDEVICE_OBJECT DeviceObject;
#ifndef _WIN64
	ULONG pack1;
#endif

    // and here are the hardware id of the devices snooped... oh well,
    // this one is rather big, so I probably should think of a better
    // scheme here... TOFIX: use less space here...
    WCHAR sHardwareIDs[HWID_MAX_PATH];

    // this contains the current milli sec count at the time of plug-in
    // all time stamps in the sniffed URB packets are relative to this one
    ULONG uTimeStampZero;

    // this contains the buffer as soon as a device connects to USBSnpys
    CRingBuffer Buffer;

} SNOOPED_DEVICE, *PSNOOPED_DEVICE;

typedef struct SNOOPED_DEVICES
{
    ULONG uCount;
    SNOOPED_DEVICE Entry[MAX_SNOOPY_DRIVERS];

} SNOOPED_DEVICES, *PSNOOPED_DEVICES;

typedef struct ENABLE_LOGGING
{
    ULONG uDeviceID;
    ULONG bEnable;
    ULONG bStopAndForget;
} ENABLE_LOGGING, *PENABLE_LOGGING;

typedef enum
{
    DIRECTION_TO_DEVICE = 0,
    DIRECTION_FROM_DEVICE = 1
} URB_DIRECTION;

#include <usbdi.h>

typedef struct PACKET_HEADER
{
    ULONG uLen;                 // total length; pNext = pHere+pHere->uLen
    URB_DIRECTION bDirection;   // direction of this packet
    ULONG uSequenceNumber;      // sequence number of this packet
    ULONG uTimeStamp;           // current timestamp of this URB...
    struct _URB_HEADER UrbHeader;
                                // could be any URB; after this, the 
                                // serialized data follows, depending on
                                // URB type
} PACKET_HEADER, *PPACKET_HEADER;

typedef struct GET_URBS
{
    ULONG nFlags;               // Special flags indicating something...
    ULONG uDeviceID;            // which device object should be queried
    ULONG nBufferSize;          // indicates how big the buffer is (in
                                // bytes, starting at tGetURBS.phURBS[0])
    ULONG nNumOfPackets;        // IOCTL return number of packets that
                                // were returned in phURBs[]...
    PACKET_HEADER phURBs[0];    // the URBs are dumped here (variable
                                // length)
} GET_URBS, *PGET_URBS;


typedef struct GET_BUFFER_FULLNESS
{
    ULONG uDeviceID;
    ULONG nBufferTotalSize;
    ULONG nBufferUsedSize;
} GET_BUFFER_FULLNESS, *PGET_BUFFER_FULLNESS;

// USBSnoop and USBSnpys communicate with function tables:
// SNOOPIES_FUNCTIONS are called by USBSnoop
// SNOOPY_FUNCTIONS are called by USBSnpys

#ifdef DRIVER

typedef struct SNOOPIES_FUNCTIONS {
    
    // to inform USBSnpys about a new device being snooped
    CRingBuffer * (*pfnDeviceArrival)(PDEVICE_OBJECT DeviceObject, char *HardwareID, ULONG uTimeStampZero, CRingBuffer *pRingBuffer);
    // to inform USBSnpys that a specific device is now gone
    ULONG (*pfnDeviceRemoval)(PDEVICE_OBJECT DeviceObject);
    // to inform USBSnpys that USBSnoop is now unloading
    void (*pfnUSBSnoopUnloading)(void);

} SNOOPIES_FUNCTIONS, *PSNOOPIES_FUNCTIONS;



typedef struct SNOOPY_FUNCTIONS {

    // global level function
    // ReleaseSnoopies() is used to tell snoopy, that it should not call
    // any snoopies function anymore...
    void (*pfnReleaseSnoopies)(PDEVICE_OBJECT DeviceObject);

    // per device level functions
    // to tell snoopy to stop logging this particular device
    ULONG (*pfnEnableLogging)(PDEVICE_OBJECT DeviceObject, ULONG bEnable);

} SNOOPY_FUNCTIONS, *PSNOOPY_FUNCTIONS;

#endif

#pragma pack(pop)

#endif
