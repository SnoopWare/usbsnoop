//************************************************************************
//
// Driver.h
//
//************************************************************************

#ifndef _DRIVER_H_INCLUDED_
#define _DRIVER_H_INCLUDED_

#include "RemoveLock.h"

// This is a nice ringbuffer implementation that Tom gave me yesterday...
// thanks!
#include "RingBuffer.h"

typedef struct DEVICE_EXTENSION
{
    PDEVICE_OBJECT      LowerDeviceObject;  // next lower driver in same stack
    MYIO_REMOVE_LOCK    RemoveLock;

    // is initialized on first load. all the timestamps of the sniffed
    // URBs are offset by this value -> this gives offset from the plugin
    // event at zero.
    ULONG               uTimeStampZero;

    // each URB is numbered:
    ULONG               uSequenceNumber;
    
    CRingBuffer         LocalBuffer;
    CRingBuffer         *pCurrentBuffer;    // points to the buffer used,
                                            // if this is &LocalBuffer, then
                                            // we have to free the memory
    BOOLEAN             bLoggingEnabled;    // TRUE: do logging
                                            // FALSE: just pass through

    // if stealing entrypoints is enabled, this is the original driverobject
    PDRIVER_OBJECT      OriginalDriverObject;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


//////////////////////////////////////////////////////////////////////////
// For our context information

typedef struct CONTEXT
{
    // information for the old CompletionRoutine
    PIO_COMPLETION_ROUTINE CompletionRoutine;
    PVOID Context;
    UCHAR Control;
    
    // what we need : the saved PURB & its number
    PURB pUrb;
    ULONG uSequenceNumber;
    
    // the stack where we should restore information
    PIO_STACK_LOCATION Stack;
    PDEVICE_OBJECT SnoopFido;
} CONTEXT, *PCONTEXT;


//////////////////////////////////////////////////////////////////////////
// Globals for the filter driver (USBSnoop)

typedef struct USBSNOOP_GLOBALS
{
    // If we successfully attach to the lobby (USBSnpys), we get some
    // function pointers that we can call there....
    SNOOPIES_FUNCTIONS Snoopies;
    
    // If this is set to something other than 0, we will steal entrypoints
    // and hopefully be able to snoop 'filterly-challenged' drivers...
    ULONG lStealEntrypoints;

    // This will point to the patched driver object function pointer table
    PDRIVER_OBJECT PatchedTable;

} USBSNOOP_GLOBALS, *PUSBSNOOP_GLOBALS;


#endif // !_DRIVER_H_INCLUDED_

//** end of Driver.h *****************************************************
/*************************************************************************

  $Log: Driver.h,v $
  Revision 1.1  2002/08/14 23:06:08  rbosa
  the WDM driver for snooping USB transactions (filter driver)

 * 
 * 1     1/25/02 2:45p Rbosa

*************************************************************************/
