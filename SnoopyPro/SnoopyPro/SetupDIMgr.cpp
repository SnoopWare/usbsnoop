//************************************************************************
//
// SetupDIMgr.cpp
//
//************************************************************************

#include "StdAfx.h"
#include "SetupDIMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////////
// CSetupDIMgr

CSetupDIMgr::CSetupDIMgr(BOOL bPresentOnly /* = FALSE */)
{
    // get a list of devices which support the given interface
	TCHAR sEnumerator[] = _T("USB");
    m_devInfo = SetupDiGetClassDevs(NULL,
                                   sEnumerator,
                                   0,
                                   DIGCF_ALLCLASSES | (bPresentOnly ? DIGCF_PRESENT : 0));

    if(m_devInfo == INVALID_HANDLE_VALUE )
    {
        TRACE("got INVALID_HANDLE_VALUE!\n");
    }
	m_nDevIndex = 0;

	// if we're running under Win2K/NT we might need this...
	m_bUseMultiSZ = (GetVersion() < 0x80000000);
}

CSetupDIMgr::~CSetupDIMgr(void)
{
    // clean up the device list
    if(m_devInfo != INVALID_HANDLE_VALUE)
    {
        if(!SetupDiDestroyDeviceInfoList(m_devInfo))
        {
            TRACE("unable to delete device info list! error: %u\n",GetLastError());
        }
		m_devInfo = INVALID_HANDLE_VALUE;
    }
}

BOOL CSetupDIMgr::Enum(void)
{
	if(INVALID_HANDLE_VALUE == m_devInfo)
	{
		return FALSE;
	}

    // as per DDK docs on SetupDiEnumDeviceInfo
    m_devInfoData.cbSize = sizeof(m_devInfoData);

    // step through the list of devices for this handle
    // get device info at index deviceIndex, the function returns FALSE
    // when there is no device at the given index.
    if(!SetupDiEnumDeviceInfo(m_devInfo, m_nDevIndex, &m_devInfoData))
	{
		//TRACE("Done enumerating...\n");
		return FALSE;
	}

	m_nDevIndex++;
	return TRUE;
}

BOOL CSetupDIMgr::GetRegProperty(CString &sProperty, DWORD dwWhich)
{
	DWORD regDataType = 0;
	LPTSTR deviceName = (LPTSTR) GetDeviceRegistryProperty(m_devInfo, &m_devInfoData,
		dwWhich, &regDataType );

	if(NULL != deviceName)
	{
		// just to make sure we are getting the expected type of buffer
		if(REG_SZ != regDataType)
		{
			if(REG_MULTI_SZ == regDataType)
			{
				LPTSTR sScanner = deviceName;
				sProperty.Empty();
				while(0 != _tcslen(sScanner))
				{
					sProperty += sScanner;
					sScanner += _tcslen(sScanner) + sizeof(TCHAR);
					if(0 != _tcslen(sScanner))
					{
						sProperty += ",";
					}
				}
				free(deviceName);
				return TRUE;
			}

			TRACE("registry key is not an SZ or MULTI_SZ!\n");
			free(deviceName);
			return FALSE;
		}

		// if the device name starts with \Device, cut that off (all
        // devices will start with it, so it is redundant)
		if(_tcsncmp(deviceName, _T("\\Device"), 7) == 0)
        {
            memmove(deviceName, deviceName+7, (_tcslen(deviceName)-6)*sizeof(_TCHAR));
        }

		sProperty = deviceName;
	    free(deviceName);
		return TRUE;
	}
	return FALSE;
}

BOOL CSetupDIMgr::DeviceName(CString &sDevName)
{
	if(!GetRegProperty(sDevName, SPDRP_FRIENDLYNAME))
		return GetRegProperty(sDevName, SPDRP_DEVICEDESC);
	return TRUE;
}

BOOL CSetupDIMgr::HardwareID(CString &sHardwareID)
{
	return GetRegProperty(sHardwareID, SPDRP_HARDWAREID);
}

BOOL CSetupDIMgr::IsLowerFilterPresent(LPCTSTR sFilterName)
{
    TCHAR sServiceName[MAX_PATH];
    _tcscpy(sServiceName, sFilterName);
    if(m_bUseMultiSZ)
    {
        // on W2K and up we have to enter the service name
        PathRemoveExtension(sServiceName);
    }

    CString sLowerFilters;
	if(!GetRegProperty(sLowerFilters, SPDRP_LOWERFILTERS))
		return FALSE;

	CString sfiltername(sServiceName);
	sfiltername.MakeLower();
	sLowerFilters.MakeLower();
	return (-1 != sLowerFilters.Find(sfiltername));
}

BOOL CSetupDIMgr::FindHardwareID(LPCTSTR sHardwareID, int nStartAt /* = 0 */)
{
	CString sLookingFor(sHardwareID);
	sLookingFor.MakeLower();
	m_nDevIndex = nStartAt;
	while(Enum())
	{
		CString sID;
		if(HardwareID(sID))
		{
			sID.MakeLower();
			if(-1 != sID.Find(sLookingFor))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CSetupDIMgr::AddLowerFilter(LPCTSTR sFilterName)
{
    TCHAR sServiceName[MAX_PATH];
    _tcscpy(sServiceName, sFilterName);
    if(m_bUseMultiSZ)
    {
        // on W2K and up we have to enter the service name
        PathRemoveExtension(sServiceName);
    }
	CString sHardwareID;
	if(HardwareID(sHardwareID))
	{
		do
		{
			if(!IsLowerFilterPresent(sServiceName))
			{
				CString sLowerFilters;
				if(GetRegProperty(sLowerFilters, SPDRP_LOWERFILTERS))
				{
					if(!sLowerFilters.IsEmpty())
						_tcscat(sServiceName, ",");
					_tcscat(sServiceName, sLowerFilters);
				}
				if(!SetRegProperty(sServiceName, SPDRP_LOWERFILTERS, TRUE))
					return FALSE;
			}
		}
		while(FindHardwareID(sHardwareID, m_nDevIndex));
		return TRUE;
	}
	return FALSE;
}

BOOL CSetupDIMgr::RemoveLowerFilter(LPCTSTR sFilterName)
{
    TCHAR sServiceName[MAX_PATH];
    _tcscpy(sServiceName, sFilterName);
    if(m_bUseMultiSZ)
    {
        // on W2K and up we have to enter the service name
        PathRemoveExtension(sServiceName);
    }
	CString sHardwareID;
	if(HardwareID(sHardwareID))
	{
		do
		{
			CString sLowerFilters;
			if(GetRegProperty(sLowerFilters, SPDRP_LOWERFILTERS))
			{
				CString sLookingFor(sServiceName);

				sLookingFor.MakeLower();
				sLowerFilters.MakeLower();

				while(-1 != sLowerFilters.Find(sLookingFor))
				{
					sLowerFilters.Delete(sLowerFilters.Find(sLookingFor), sLookingFor.GetLength());
				}

				sLookingFor = ",,";
				while(-1 != sLowerFilters.Find(sLookingFor))
				{
					sLowerFilters.Delete(sLowerFilters.Find(sLookingFor), 1);
				}

				while(sLowerFilters.Left(1) == ",")
					sLowerFilters.Delete(0);

				while(sLowerFilters.Right(1) == ",")
					sLowerFilters.Delete(sLowerFilters.GetLength() - 1);
			}
			if(!SetRegProperty(sLowerFilters, SPDRP_LOWERFILTERS, TRUE))
				return FALSE;
		}
		while(FindHardwareID(sHardwareID, m_nDevIndex));
		return TRUE;
	}
	return FALSE;
}

BOOL CSetupDIMgr::SetRegProperty(LPCTSTR sValue, DWORD dwWhich, BOOL bPossiblyMultiSZ)
{
	CString svalue(sValue);

	if((NULL == sValue) || svalue.IsEmpty())
	{
		if(!SetupDiSetDeviceRegistryProperty(m_devInfo, &m_devInfoData, dwWhich, NULL, 0))
		{
			BYTE sEmpty[] = { 0, 0, 0, 0, 0, 0 };
			if(!SetupDiSetDeviceRegistryProperty(m_devInfo, &m_devInfoData, dwWhich, sEmpty, 2 * sizeof(TCHAR)))
			{
				TRACE("Set Property failed with %08x\n", GetLastError());
				return FALSE;
			}
		}
	}
	else
	{
		long length = _tcslen(sValue) + 1;
		long size = (length + 1) * sizeof(TCHAR);
		PBYTE buffer = (PBYTE) malloc(size);
		if(NULL == buffer)
		{
			return FALSE;
		}

		ZeroMemory(buffer, size);
		memcpy(buffer, sValue, size - sizeof(TCHAR));

		if(m_bUseMultiSZ && bPossiblyMultiSZ)
		{
			TRACE("Using MultiSZ on setProperty value...\n");
			LPTSTR sScanner = (LPTSTR) buffer;
			for(int i = 0; i < size; i++, sScanner++)
			{
				if(*sScanner == _T(','))
					*sScanner = _T(0x00);
			}
		}

		if(!SetupDiSetDeviceRegistryProperty(m_devInfo, &m_devInfoData,
			dwWhich, buffer, size))
		{
			TRACE("SetProperty failed with %08x\n", GetLastError());
		}

		free(buffer);
	}

	return TRUE;
}


/*
* restarts the given device
*
* call CM_Query_And_Remove_Subtree (to unload the driver)
* call CM_Reenumerate_DevNode on the _parent_ (to reload the driver)
*
* parameters:
*   DeviceInfoSet  - The device information set which contains DeviceInfoData
*   DeviceInfoData - Information needed to deal with the given device
*/
BOOLEAN CSetupDIMgr::RestartDevice(void)
{
	CString sHardwareID;
	if(HardwareID(sHardwareID))
	{
		do
		{
			SP_PROPCHANGE_PARAMS params;

            // for future compatibility; this will zero out the entire struct, rather
			// than just the fields which exist now
			memset(&params, 0, sizeof(SP_PROPCHANGE_PARAMS));
			
			// initialize the SP_CLASSINSTALL_HEADER struct at the beginning of the
			// SP_PROPCHANGE_PARAMS struct, so that SetupDiSetClassInstallParams will
			// work
			params.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
			params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
			
			// initialize SP_PROPCHANGE_PARAMS such that the device will be stopped.
			params.StateChange = DICS_PROPCHANGE;
			params.Scope       = DICS_FLAG_CONFIGSPECIFIC;
			params.HwProfile   = 0; // current profile
			
			if(!SetupDiSetClassInstallParams(m_devInfo, &m_devInfoData,
				(PSP_CLASSINSTALL_HEADER) &params,
				sizeof(SP_PROPCHANGE_PARAMS)))
			{
				TRACE("in RestartDevice(): couldn't set the install parameters!");
				TRACE(" error: %u\n", GetLastError());
				return FALSE;
			}
			
			if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, m_devInfo, &m_devInfoData))
			{
				TRACE("in RestartDevice(): call to class installer (STOP) failed!");
				TRACE(" error: %u\n", GetLastError() );
				return FALSE;
			}
		}
		while(FindHardwareID(sHardwareID, m_nDevIndex));
		return TRUE;
	}
	return FALSE;
}


/*
* A wrapper around SetupDiGetDeviceRegistryProperty, so that I don't have to
* deal with memory allocation anywhere else
*
* parameters:
*   DeviceInfoSet  - The device information set which contains DeviceInfoData
*   DeviceInfoData - Information needed to deal with the given device
*   Property       - which property to get (SPDRP_XXX)
*   PropertyRegDataType - the type of registry property
*/
PBYTE CSetupDIMgr::GetDeviceRegistryProperty(HDEVINFO DeviceInfoSet, 
	PSP_DEVINFO_DATA DeviceInfoData, DWORD Property, PDWORD PropertyRegDataType)
{
	DWORD length = 0;
	PBYTE buffer = NULL;

	// get the required length of the buffer
	SetupDiGetDeviceRegistryProperty(DeviceInfoSet, DeviceInfoData, Property, PropertyRegDataType,
										  buffer, length, &length);
	if(0 == length)
	{
		//TRACE("no length...\n");
		return NULL;
	}
	//?
	length += sizeof(TCHAR);

	// since we don't have a buffer yet, it is "insufficient"; we allocate
	// one and try again.
	buffer = (PBYTE) malloc(length);
	if( buffer == NULL )
	{
		TRACE("in GetDeviceRegistryProperty(): "
			   "unable to allocate memory!\n");
		return (NULL);
	}
	if( !SetupDiGetDeviceRegistryProperty( DeviceInfoSet,
										   DeviceInfoData,
										   Property,
										   PropertyRegDataType,
										   buffer,
										   length,
										   NULL// required size
		) )
	{
		TRACE("in GetDeviceRegistryProperty(): "
			   "couldn't get registry property! error: %i\n",
			   GetLastError());
		free( buffer );
		return (NULL);
	}

	// ok, we are finally done, and can return the buffer
	return (buffer);
}


// Win2K stuff...

BOOL CSetupDIMgr::InstallService(LPCTSTR sFilterServiceName, LPCTSTR sFilterBinaryPath, LPCTSTR sFilterDescription)
{
	BOOL bResult = FALSE;
	SC_HANDLE hScm = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	
	if(NULL != hScm)
	{
		SC_HANDLE hService = OpenService(hScm, sFilterServiceName, SERVICE_ALL_ACCESS);
		if((NULL == hService) && (ERROR_SERVICE_DOES_NOT_EXIST == GetLastError()))
		{
			// service doesn't exist. Let's create it.
			hService = CreateService(hScm, sFilterServiceName, sFilterDescription,
				SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START,
				SERVICE_ERROR_NORMAL, sFilterBinaryPath, NULL, NULL, NULL, NULL, NULL);
			if(hService)
            {
                bResult = TRUE;
				CloseServiceHandle(hService);
			}
			else
			{
				TRACE("CreateService(Service) failed with %08x\n", GetLastError());
			}
		}
		else if(NULL != hService)
		{
			CloseServiceHandle(hService);
		}
		CloseServiceHandle(hScm);
	}
	return bResult;
}

BOOL CSetupDIMgr::RemoveService(LPCTSTR sFilterServiceName)
{
	BOOL bResult = FALSE;
	SC_HANDLE hScm = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	
	if(NULL != hScm)
	{
		SC_HANDLE hService = OpenService(hScm, sFilterServiceName, SERVICE_ALL_ACCESS);
		if(NULL != hService)
		{
			if(DeleteService(hService))
			{
				bResult = TRUE;
			}
			else
			{
				TRACE("DeleteService() failed with %08x\n", GetLastError());
			}
			CloseServiceHandle(hService);
		}
		CloseServiceHandle(hScm);
	}
	return bResult;
}

BOOL CSetupDIMgr::StartService(LPCTSTR sServiceName)
{
	BOOL bResult = FALSE;
	SC_HANDLE hScm = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	if(NULL != hScm)
	{
		SC_HANDLE hService = OpenService(hScm, sServiceName, SERVICE_ALL_ACCESS);
		if(NULL != hService)
		{
            if(::StartService(hService, 0, NULL))
            {
                TRACE("Started service succesfully!\n");
				bResult = TRUE;
            }
            TRACE("GetLastError(): %d\n", GetLastError());
			CloseServiceHandle(hService);
        }
        CloseServiceHandle(hScm);
    }
    return bResult;
}

BOOL CSetupDIMgr::StopService(LPCTSTR sServiceName)
{
	BOOL bResult = FALSE;
	SC_HANDLE hScm = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	
	if(NULL != hScm)
	{
		SC_HANDLE hService = OpenService(hScm, sServiceName, SERVICE_ALL_ACCESS);
		if(NULL != hService)
		{
            SERVICE_STATUS ServiceStatus;
            ZeroMemory(&ServiceStatus, sizeof(ServiceStatus));
            if(ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus))
            {
                TRACE("Succesfully stopped service...\n");
                bResult = TRUE;
            }
            TRACE("GetLastError(): %d\n", GetLastError());
			CloseServiceHandle(hService);
		}
		CloseServiceHandle(hScm);
	}
	return bResult;
}

//** end of SetupDIMgr.cpp ***********************************************
/*************************************************************************

  $Log: not supported by cvs2svn $
 * 
 * 4     2/21/02 4:29p Rbosa
 * - now installing the filter in front of the LowerFilters entry, so it's
 * loaded first
 * 
 * 3     2/05/02 9:00p Rbosa
 * - removed wrong comment
 * 
 * 2     1/25/02 2:48p Rbosa
 * This is a good state to check in...not everything is implemented, but
 * one can use the Snooper in this state...
  * 
  * 1     8/21/01 7:08p Rbosa

*************************************************************************/
