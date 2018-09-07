//************************************************************************
//
// USBSnpyV.h
//
//************************************************************************

#ifndef _USBSNPYV_H_INCLUDED_
#define _USBSNPYV_H_INCLUDED_

//////////////////////////////////////////////////////////////////////////
// Globals for the lobby driver (USBSnpys)

typedef struct USBSNPYS_GLOBALS
{
    // as soon as USBSnoop is loaded, it knocks on USBSnpyV's door and
    // exchanges some functions. They are stored here:
    SNOOPY_FUNCTIONS    Snoopy;

    // contains all the known (snooped) devices
    SNOOPED_DEVICES     Snooped;

    // contains true if the device at that index has been removed
    // (the buffer might still be valid though...)
    BOOLEAN HasBeenRemoved[MAX_SNOOPY_DRIVERS];

    // contains the next available device ID
    LONG nNextDeviceID;

} USBSNPYS_GLOBALS, *PUSBSNPYS_GLOBALS;

#endif // !_USBSNPYV_H_INCLUDED_


//** end of USBSnpyV.h ***************************************************
/*************************************************************************

  $Log: USBSnpyV.h,v $
  Revision 1.1  2002/08/14 23:09:43  rbosa
  the VxD driver bridge (used under Windows 9x)

 * 
 * 1     2/05/02 9:08p Rbosa
 * - base for USBSnoopies VxD

*************************************************************************/
