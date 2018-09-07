//************************************************************************
//
// RemoveLock.h
//
//************************************************************************

#ifndef _REMOVELOCK_H_INCLUDED_ 
#define _REMOVELOCK_H_INCLUDED_ 

// Currently, the support routines for managing the device remove lock aren't
// defined in the DDK or implemented by the OS. The following declarations are
// for use with the simplified implementation in RemoveLock.cpp.

typedef struct MY_IO_REMOVE_LOCK {
    LONG usage;                 // reference count
    BOOLEAN removing;           // true if removal is pending
    KEVENT evRemove;            // event to wait on
} MYIO_REMOVE_LOCK, *PMYIO_REMOVE_LOCK;

VOID MyIoInitializeRemoveLock(PMYIO_REMOVE_LOCK lock, ULONG tag, ULONG minutes, ULONG maxcount);
NTSTATUS MyIoAcquireRemoveLock(PMYIO_REMOVE_LOCK lock, PVOID tag);
VOID MyIoReleaseRemoveLock(PMYIO_REMOVE_LOCK lock, PVOID tag);
VOID MyIoReleaseRemoveLockAndWait(PMYIO_REMOVE_LOCK lock, PVOID tag);


#endif // !_REMOVELOCK_H_INCLUDED_ 

//** end of RemoveLock.h *************************************************
/*************************************************************************

  $Log: RemoveLock.h,v $
  Revision 1.1  2002/08/14 23:06:08  rbosa
  the WDM driver for snooping USB transactions (filter driver)

 * 
 * 1     1/25/02 2:45p Rbosa

*************************************************************************/
