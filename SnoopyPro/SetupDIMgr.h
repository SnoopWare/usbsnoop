//************************************************************************
//
// SetupDIMgr.h
//
//************************************************************************

#ifndef _SETUPDIMGR_H_INCLUDED_ 
#define _SETUPDIMGR_H_INCLUDED_ 

class CSetupDIMgr
{
public:
	CSetupDIMgr(BOOL bPresentOnly = FALSE);
	virtual ~CSetupDIMgr(void);

	BOOL Enum(void);
	BOOL GetRegProperty(CString &sProperty, DWORD dwWhich);
	BOOL DeviceName(CString &sDevName);
	BOOL HardwareID(CString &sHardwareID);
	BOOL IsLowerFilterPresent(LPCTSTR sFilterName);
	BOOL FindHardwareID(LPCTSTR sHardwareID, int nStartAt = 0);
	BOOL AddLowerFilter(LPCTSTR sFilterName);
	BOOL RemoveLowerFilter(LPCTSTR sFilterName);
	BOOL SetRegProperty(LPCTSTR sValue, DWORD dwWhich, BOOL bPossiblyMultiSZ);
	BOOLEAN RestartDevice(void);
	PBYTE GetDeviceRegistryProperty(HDEVINFO DeviceInfoSet, 
		PSP_DEVINFO_DATA DeviceInfoData, DWORD Property, PDWORD PropertyRegDataType);
	// win2K specialties...
	BOOL InstallService(LPCTSTR sFilterServiceName, LPCTSTR sFilterBinaryPath, LPCTSTR sFilterDescription);
	BOOL RemoveService(LPCTSTR sFilterServiceName);
    BOOL StartService(LPCTSTR sServiceName);
    BOOL StopService(LPCTSTR sServiceName);

// private:
	BOOL m_bUseMultiSZ;
	BOOL m_bIsVistaOrLater;
	HDEVINFO m_devInfo;
	SP_DEVINFO_DATA m_devInfoData;
	int m_nDevIndex;
};




#endif // !_SETUPDIMGR_H_INCLUDED_ 

//** end of SetupDIMgr.h *************************************************
/*************************************************************************

  $Log: SetupDIMgr.h,v $
  Revision 1.1  2002/08/14 23:03:35  rbosa
  the application to capture urbs and display them...

 * 
 * 2     1/25/02 2:48p Rbosa
 * This is a good state to check in...not everything is implemented, but
 * one can use the Snooper in this state...
  * 
  * 1     8/21/01 7:08p Rbosa
  
*************************************************************************/
