// SniffUSBDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "SniffUSB.h"
#include "SniffUSBDlg.h"

#include <setupapi.h>
#include <winsvc.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSniffUSBDlg dialog

CSniffUSBDlg::CSniffUSBDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSniffUSBDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSniffUSBDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_sFilterName.LoadString(IDS_FILTERNAME);
    m_sFilterName.MakeLower();
    m_sLowerFilters.LoadString(IDS_LOWERFILTERS);
}

void CSniffUSBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSniffUSBDlg)
	DDX_Control(pDX, IDC_USBDEVS, m_cDevs);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSniffUSBDlg, CDialog)
	//{{AFX_MSG_MAP(CSniffUSBDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_BN_CLICKED(IDC_INSTALL, OnInstall)
	ON_BN_CLICKED(IDC_UNINSTALL, OnUninstall)
	ON_NOTIFY(NM_RCLICK, IDC_USBDEVS, OnRclickUsbdevs)
	ON_COMMAND(ID_SNOOPUSB_INSTALL, OnSnoopusbInstall)
	ON_COMMAND(ID_SNOOPUSB_UNINSTALL, OnSnoopusbUninstall)
	ON_BN_CLICKED(IDC_REPLUG, OnReplug)
	ON_COMMAND(ID_SNOOPUSB_REPLUG, OnSnoopusbReplug)
	ON_BN_CLICKED(IDC_FILTERINSTALL, OnFilterInstall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSniffUSBDlg message handlers

BOOL CSniffUSBDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
    // we want report style and more...
    m_cDevs.ModifyStyle(LVS_TYPEMASK, 
        LVS_AUTOARRANGE | LVS_SORTASCENDING | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
    
    // setup the columns
    CString sHeading;
    sHeading.LoadString(IDS_COL_VIDPID);
    m_cDevs.InsertColumn(0, sHeading);
    m_nCX0 = 5*m_cDevs.GetStringWidth(sHeading)/4;

    sHeading.LoadString(IDS_COL_FILTERINSTALLED);
    m_cDevs.InsertColumn(1, sHeading, LVCFMT_CENTER);
    m_nCX1 = 5*m_cDevs.GetStringWidth(sHeading)/4;

    sHeading.LoadString(IDS_COL_DESCRIPTION);
    m_cDevs.InsertColumn(2, sHeading);
    m_nCX2 = 5*m_cDevs.GetStringWidth(sHeading)/4;

    m_cDevs.SetColumnWidth(0, m_nCX0);
    m_cDevs.SetColumnWidth(1, m_nCX1);
    m_cDevs.SetColumnWidth(2, m_nCX2);

    // fill it up, please
    OnRefresh();
	CheckService();
    
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSniffUSBDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSniffUSBDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSniffUSBDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSniffUSBDlg::OnRefresh() 
{
    UINT nSelCount = m_cDevs.GetSelectedCount();
    int nSelection = -1;
    CString sSelection;
    if(0 < nSelCount)
    {
        nSelection = m_cDevs.GetNextItem(-1, LVNI_SELECTED);
        sSelection = m_cDevs.GetItemText(nSelection, 0);
    }
    m_cDevs.DeleteAllItems();
    int cx0 = m_nCX0;
    int cx1 = m_nCX1;
    int cx2 = m_nCX2;
    
 	HDEVINFO hdev;
	
	hdev = SetupDiGetClassDevs(0,NULL,0,DIGCF_ALLCLASSES);
	if (hdev == INVALID_HANDLE_VALUE )
	{
		MessageBox("Unable to enumerate USB device");
		TRACE("SetupDiGetClassDevs = %d\n",GetLastError());
		return ;
	}

	for (DWORD idx=0;;idx++)
	{
		SP_DEVINFO_DATA  devinfo;
		devinfo.cbSize = sizeof(devinfo);

		CString sName, sFilter, sDescription;

		BYTE Buffer[200];
		DWORD BufferSize = 0;
		DWORD DataType;

		if (!SetupDiEnumDeviceInfo(hdev,idx,&devinfo))
		{
			if (GetLastError() != ERROR_NO_MORE_ITEMS)
			{
				MessageBox("Error while enumerating USB devices");
				TRACE("SetupDiEnumDeviceInfo = %d\n",GetLastError());
			}
			break;
		}

		if (SetupDiGetDeviceRegistryProperty(hdev,&devinfo,SPDRP_ENUMERATOR_NAME,
			&DataType,Buffer,sizeof(Buffer),&BufferSize))
		{
			if (strcmp((const char *)Buffer,"USB")!=0)
				continue;
		}
 
		if (SetupDiGetDeviceRegistryProperty(hdev,&devinfo,SPDRP_HARDWAREID  ,
			&DataType,Buffer,sizeof(Buffer),&BufferSize))
		{
			sName = (const char *)Buffer;
		}

		if (SetupDiGetDeviceRegistryProperty(hdev,&devinfo,SPDRP_DEVICEDESC,
			&DataType,Buffer,sizeof(Buffer),&BufferSize))
		{
			sDescription = (const char *) Buffer;
		}

		sFilter.LoadString(IDS_NOTINSTALLED);
		if (SetupDiGetDeviceRegistryProperty(hdev,&devinfo,SPDRP_LOWERFILTERS ,
			&DataType,Buffer,sizeof(Buffer),&BufferSize))
		{
			if (strcmp((const char *)Buffer,"usbsnoop")==0)
				sFilter.LoadString(IDS_INSTALLED);
		}

        int nIndex = m_cDevs.InsertItem(-1, sName);
        m_cDevs.SetItemText(nIndex, 1, sFilter);
        m_cDevs.SetItemText(nIndex, 2, sDescription);
        cx0 = max(cx0, 5*m_cDevs.GetStringWidth(sName)/4);
        cx1 = max(cx1, 5*m_cDevs.GetStringWidth(sFilter)/4);
        cx2 = max(cx2, 5*m_cDevs.GetStringWidth(sDescription)/4);


    }

	SetupDiDestroyDeviceInfoList(hdev);

    m_cDevs.SetColumnWidth(0, cx0);
    m_cDevs.SetColumnWidth(1, cx1);
    m_cDevs.SetColumnWidth(2, cx2);

    if(-1 != nSelection)
    {
        LVFINDINFO fi;
        ZeroMemory(&fi, sizeof(fi));
        fi.flags = LVFI_STRING;
        fi.psz = sSelection;
        nSelection = m_cDevs.FindItem(&fi);
        m_cDevs.SetItemState(nSelection, LVIS_SELECTED, LVIS_SELECTED);
    }
}

void CSniffUSBDlg::GetDescriptionForVidPid(LPCTSTR szVidPid, CString &sDescription)
{
    sDescription = "?";
    TCHAR szEnumKey[MAX_PATH];
    _tcscpy(szEnumKey, "Enum\\USB\\");
    _tcscat(szEnumKey, szVidPid);
    HKEY hKey = NULL;
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szEnumKey, 0, KEY_ALL_ACCESS, &hKey))
    {
        DWORD dwIndex = 0;
        TCHAR sName[MAX_PATH];
        while((sDescription == "?") && (ERROR_SUCCESS == RegEnumKey(hKey, dwIndex, sName, MAX_PATH)))
        {
            TRACE("Enumerated >%s<\n", sName);
            HKEY hInstKey = NULL;
            TCHAR szInstanceKey[MAX_PATH];
            _tcscpy(szInstanceKey, szEnumKey);
            _tcscat(szInstanceKey, _T("\\"));
            _tcscat(szInstanceKey, sName);
            if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szInstanceKey, 0, KEY_ALL_ACCESS, &hInstKey))
            {
                TCHAR szData[MAX_PATH];
                DWORD dwType = REG_SZ;
                DWORD dwSize = MAX_PATH;
                if(ERROR_SUCCESS == RegQueryValueEx(hInstKey, "DeviceDesc", NULL, &dwType, (LPBYTE) szData, &dwSize))
                {
                    sDescription = szData;
                }
                RegCloseKey(hInstKey);
                hInstKey = NULL;
            }
            dwIndex++;
        }
        RegCloseKey(hKey);
        hKey = NULL;
    }
}

BOOL CSniffUSBDlg::IsThereAFilter(LPCTSTR szVidPid)
{
    BOOL bThereIsAFilter = FALSE;
    TCHAR szEnumKey[MAX_PATH];
    _tcscpy(szEnumKey, "Enum\\USB\\");
    _tcscat(szEnumKey, szVidPid);
    HKEY hKey = NULL;
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szEnumKey, 0, KEY_ALL_ACCESS, &hKey))
    {
        DWORD dwIndex = 0;
        TCHAR sName[MAX_PATH];
        while(ERROR_SUCCESS == RegEnumKey(hKey, dwIndex, sName, MAX_PATH))
        {
            TRACE("Enumerated >%s<\n", sName);
            HKEY hInstKey = NULL;
            TCHAR szInstanceKey[MAX_PATH];
            _tcscpy(szInstanceKey, szEnumKey);
            _tcscat(szInstanceKey, _T("\\"));
            _tcscat(szInstanceKey, sName);
            if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szInstanceKey, 0, KEY_ALL_ACCESS, &hInstKey))
            {
                TCHAR szData[MAX_PATH];
                DWORD dwType = REG_SZ;
                DWORD dwSize = MAX_PATH;
                if(ERROR_SUCCESS == RegQueryValueEx(hInstKey, m_sLowerFilters, NULL, &dwType, (LPBYTE) szData, &dwSize))
                {
                    CString sData = szData;
                    sData.MakeLower();
                    if(NULL != _tcsstr(szData, m_sFilterName))
                    {
                        TRACE("Found filter!\n");
                        bThereIsAFilter = TRUE;
                    }
                }
                RegCloseKey(hInstKey);
                hInstKey = NULL;
            }
            dwIndex++;
        }
        RegCloseKey(hKey);
        hKey = NULL;
    }

    return bThereIsAFilter;
}

void CSniffUSBDlg::ModifyFilterOnVIDPID(LPCTSTR szVidPid, BOOL bAddFilter)
{
 	HDEVINFO hdev;
	
	hdev = SetupDiGetClassDevs(0,NULL,0,DIGCF_ALLCLASSES);
	if (hdev == INVALID_HANDLE_VALUE )
	{
		MessageBox("Unable to enumerate USB device");
		TRACE("SetupDiGetClassDevs = %d\n",GetLastError());
		return ;
	}

	for (DWORD idx=0;;idx++)
	{
		SP_DEVINFO_DATA  devinfo;
		devinfo.cbSize = sizeof(devinfo);

		CString sName, sFilter, sDescription;

		BYTE Buffer[200];
		DWORD BufferSize = 0;
		DWORD DataType;

		if (!SetupDiEnumDeviceInfo(hdev,idx,&devinfo))
		{
			if (GetLastError() != ERROR_NO_MORE_ITEMS)
			{
				MessageBox("Error while enumerating USB devices");
				TRACE("SetupDiEnumDeviceInfo = %d\n",GetLastError());
			}
			break;
		}

		if (SetupDiGetDeviceRegistryProperty(hdev,&devinfo,SPDRP_HARDWAREID  ,
			&DataType,Buffer,sizeof(Buffer),&BufferSize))
		{
			if (strcmp((const char *)Buffer,szVidPid)==0)
			{
				// gotcha !
				if (bAddFilter)
				{
					int len = strlen(SERVICE)+2;
					BYTE * LowerFilters = (BYTE *) malloc(len);
					memset(LowerFilters,0,len);
					strcpy((char *)LowerFilters,SERVICE);

					if (!SetupDiSetDeviceRegistryProperty(hdev,&devinfo,SPDRP_LOWERFILTERS ,
						LowerFilters,len))
					{
						MessageBox("Install failed!");
						TRACE("SetupDiSetDeviceRegistryProperty = %d\n",GetLastError());
					}
				}
				else
				{
					if (!SetupDiSetDeviceRegistryProperty(hdev,&devinfo,SPDRP_LOWERFILTERS ,
						NULL,0))
					{
						MessageBox("Uninstall failed!");
						TRACE("SetupDiSetDeviceRegistryProperty = %d\n",GetLastError());
					}
				}
			}
		}
    }

	SetupDiDestroyDeviceInfoList(hdev);
}

BOOL CSniffUSBDlg::GetSelectedVidPid(CString& sVidPid)
{
    UINT nSelected = m_cDevs.GetSelectedCount();
    if(0 == nSelected)
    {
        AfxMessageBox(IDS_SELECT_ITEM_FIRST);
        return FALSE;
    }

    int nIndex = m_cDevs.GetNextItem(-1, LVNI_SELECTED);
    sVidPid = m_cDevs.GetItemText(nIndex, 0);
    return TRUE;
}

void CSniffUSBDlg::OnInstall() 
{
    CString sVidPid;
    if(GetSelectedVidPid(sVidPid))
    {
        TRACE("Installing on %s\n", sVidPid);
        ModifyFilterOnVIDPID(sVidPid, TRUE);
        OnRefresh();
    }
}

void CSniffUSBDlg::OnUninstall() 
{
    CString sVidPid;
    if(GetSelectedVidPid(sVidPid))
    {
        TRACE("Installing on %s\n", sVidPid);
        ModifyFilterOnVIDPID(sVidPid, FALSE);
        OnRefresh();
    }
}

void CSniffUSBDlg::OnReplug() 
{
    CString HardwareId;
    if(GetSelectedVidPid(HardwareId))
    {
 		HDEVINFO hdev;
		hdev = SetupDiGetClassDevs(0,NULL,0,DIGCF_ALLCLASSES);
		if (hdev == INVALID_HANDLE_VALUE )
		{
			MessageBox("Unable to enumerate USB device");
			TRACE("SetupDiGetClassDevs = %d\n",GetLastError());
			return ;
		}

		for (DWORD idx=0;;idx++)
		{
			SP_DEVINFO_DATA  devinfo;
			devinfo.cbSize = sizeof(devinfo);

			CString sName, sFilter, sDescription;

			BYTE Buffer[200];
			DWORD BufferSize = 0;
			DWORD DataType;

			if (!SetupDiEnumDeviceInfo(hdev,idx,&devinfo))
			{
				if (GetLastError() != ERROR_NO_MORE_ITEMS)
				{
					MessageBox("Error while enumerating USB devices");
					TRACE("SetupDiEnumDeviceInfo = %d\n",GetLastError());
				}
				break;
			}

			if (SetupDiGetDeviceRegistryProperty(hdev,&devinfo,SPDRP_HARDWAREID  ,
				&DataType,Buffer,sizeof(Buffer),&BufferSize))
			{
				if (strcmp((const char *)Buffer,HardwareId)==0)
				{
					// gotcha !
	 				if (MessageBox("I will briefly remove/add this device "
								"and associated software "
								"from your system! Are you sure? ",HardwareId,MB_YESNO) == IDYES)
					{
						CWaitCursor waitcursor;

						if (SetupDiRemoveDevice(hdev,&devinfo))
						{
							if (SetupDiUnremoveDevice(hdev,&devinfo))
								MessageBox("Gotcha!");
							else
								TRACE("SetupDiUnremoveDevice = %d\n",GetLastError());
						}
						else
							TRACE("SetupDiRemoveDevice = %d\n",GetLastError());
					}
				}
			}
		}

		SetupDiDestroyDeviceInfoList(hdev);
		
		OnRefresh();
		TRACE("Back to life..\n");
	}
}

void CSniffUSBDlg::OnRclickUsbdevs(NMHDR* pNMHDR, LRESULT* pResult) 
{
    CMenu ctx;
    ctx.LoadMenu(IDR_SNOOPUSB);
    CMenu *popup = ctx.GetSubMenu(0);
    CPoint point;
    GetCursorPos(&point);
    CPoint pt = point;
    m_cDevs.ScreenToClient(&pt);
    UINT uFlags = 0;
    int nIndex = m_cDevs.HitTest(pt, &uFlags);
    if(LVHT_ONITEM & uFlags)
    {
        m_cDevs.SetItem(nIndex, 0, LVIF_STATE, NULL, 0, LVIS_SELECTED, LVIS_SELECTED, 0);
        CString sVidPid = m_cDevs.GetItemText(nIndex, 0);
        if(IsThereAFilter(sVidPid))
        {
            popup->EnableMenuItem(ID_SNOOPUSB_INSTALL, MF_BYCOMMAND | MF_GRAYED);
        }
        else
        {
            popup->EnableMenuItem(ID_SNOOPUSB_UNINSTALL, MF_BYCOMMAND | MF_GRAYED);
        }
        popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
    }
	
	*pResult = 0;
}

void CSniffUSBDlg::OnSnoopusbInstall() 
{
    OnInstall();	
}

void CSniffUSBDlg::OnSnoopusbUninstall() 
{
    OnUninstall();
}

void CSniffUSBDlg::OnSnoopusbReplug() 
{
    OnReplug();	
}

void CSniffUSBDlg::OnFilterInstall() 
{
	// TODO: Add your control notification handler code here
	
}

void CSniffUSBDlg::CheckService()
{
	// check if "usbsnoop" service is installed

	SC_HANDLE hManager = OpenSCManager(NULL,NULL,
		SC_MANAGER_CREATE_SERVICE|SC_MANAGER_ENUMERATE_SERVICE);

	if (hManager == NULL)
	{
		MessageBox("Can't open service manager");
		return ;
	}

	SC_HANDLE hService = OpenService(hManager,"usbsnoop",DELETE);
	if (hService == NULL)
	{
		if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
		{
			if (MessageBox("Service usbsnoop","Would you like to install?",MB_YESNO)==IDYES)
			{
				hService = CreateService(hManager,"usbsnoop","usbsnoop (display)",
					0,SERVICE_KERNEL_DRIVER,SERVICE_DEMAND_START,SERVICE_ERROR_NORMAL,
					"System32\\DRIVERS\\USBSNOOP.SYS",
					NULL,NULL,NULL,NULL,NULL);
				if (hService == NULL)
				{
					MessageBox("Can't create service");
				}
				else
					CloseServiceHandle(hService);
			}
		}
	}
	else
	{
/*
		if (!DeleteService(hService))
			MessageBox("Cannot remove existing usbsnoop service");
*/
		CloseServiceHandle(hService);
	}


	CloseServiceHandle(hManager);
}
