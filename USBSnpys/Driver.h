//************************************************************************
//
// Driver.h
//
//************************************************************************

#ifndef _DRIVER_H_INCLUDED_
#define _DRIVER_H_INCLUDED_

//////////////////////////////////////////////////////////////////////////
// Globals for the lobby driver (USBSnpys)

typedef struct USBSNPYS_GLOBALS
{
    // this is the device object which is accessed by ring 3 applications
    PDEVICE_OBJECT      ControlObject;

    // as soon as USBSnoop is loaded, it knocks on USBSnpys door and
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

#endif // !_DRIVER_H_INCLUDED_


//** end of Driver.h *****************************************************
/*************************************************************************

  $Log: Driver.h,v $
  Revision 1.1  2002/08/14 23:10:42  rbosa
  the WDM driver to connect to the snooper filter (used under Win2K)

 * 
 * 2     2/05/02 9:09p Rbosa
 * - added unique DeviceID instead of DeviceObject as ID
 * 
 * 1     1/25/02 2:45p Rbosa

*************************************************************************/
