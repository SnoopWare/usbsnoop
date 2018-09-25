// DevicesDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "SnoopyPro.h"
#include "DevicesDlg.h"
#include "SetupDIMgr.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDevicesDlg dialog

CDevicesDlg::CDevicesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDevicesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDevicesDlg)
	//}}AFX_DATA_INIT
    
    m_nCX0 = 0;
    m_nCX1 = 0;
    m_nCX2 = 0;

    m_bIsWin2K = (0 == (GetVersion() & 0x80000000));

    if(m_bIsWin2K)
    {
        m_sFilterName.LoadString(IDS_FILTERNAME_2K);
    }
    else
    {
        m_sFilterName.LoadString(IDS_FILTERNAME_9X);
    }
    m_sFilterName.MakeLower();

    m_dwUsbSnoopSize = 0;
    m_pUsbSnoop = NULL;
    m_szWinDir = _T("");
    m_hSniffer = INVALID_HANDLE_VALUE;
}

CDevicesDlg::~CDevicesDlg(void)
{
    if(!m_bIsWin2K)
    {
        CloseSniffer();
    }
}

void CDevicesDlg::OpenSniffer(void)
{
    ASSERT(INVALID_HANDLE_VALUE == m_hSniffer);
    ASSERT(!m_bIsWin2K);

    m_hSniffer = CreateFile(USBSNPYS_W32NAME_9X,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, // no SECURITY_ATTRIBUTES structure
        OPEN_EXISTING,
        FILE_FLAG_DELETE_ON_CLOSE,
        NULL);
    if(INVALID_HANDLE_VALUE != m_hSniffer)
    {
        TRACE("CDeviceDlg::Loaded Sniffer VxD...\n");
    }
}

void CDevicesDlg::CloseSniffer(void)
{
    if(INVALID_HANDLE_VALUE != m_hSniffer)
    {
        TRACE("CDeviceDlg::Unloaded Sniffer VxD...\n");
        CloseHandle(m_hSniffer);
        m_hSniffer = INVALID_HANDLE_VALUE;
    }
}

void CDevicesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDevicesDlg)
	DDX_Control(pDX, IDC_DRIVER_STATUS, m_cDriverStatus);
	DDX_Control(pDX, IDC_DEVICES, m_cDevs);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDevicesDlg, CDialog)
	//{{AFX_MSG_MAP(CDevicesDlg)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	ON_COMMAND(ID_INSTALL_SNIFFER, OnInstallSniffer)
	ON_COMMAND(ID_UNINSTALL_SNIFFER, OnUninstallSniffer)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_NOTIFY(NM_RCLICK, IDC_DEVICES, OnRclickDevices)
	ON_COMMAND(ID_UNINSTALL_SERVICE, OnUninstallService)
	ON_COMMAND(ID_INSTALL_SERVICE, OnInstallService)
	ON_COMMAND(ID_UNPACK_DRIVERS, OnUnpackDrivers)
	ON_COMMAND(ID_RESTART_DEVICE, OnRestartDevice)
	ON_COMMAND(ID_VIEW_DEVICES, OnViewDevices)
	ON_WM_TIMER()
	ON_COMMAND(ID_INSTALL_RESTART, OnInstallRestart)
	ON_COMMAND(ID_UNINSTALL_RESTART, OnUninstallRestart)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDevicesDlg message handlers

BOOL CDevicesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    // Set big icon
    SetIcon(AfxGetApp()->LoadIcon(IDI_USB_LOGO), TRUE);
    // Set small icon
    SetIcon((HICON)
        LoadImage(
        AfxGetResourceHandle(),
        MAKEINTRESOURCE(IDI_USB_LOGO),
        IMAGE_ICON,
        16,
        16,
        LR_DEFAULTCOLOR), FALSE);

    //TODO:  Adapt Menu to functions available on OS
    //CMenu *pMenu = GetMenu();
	
    // we want report style and more...
    m_cDevs.ModifyStyle(LVS_TYPEMASK, LVS_AUTOARRANGE | LVS_SORTASCENDING | 
        LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
    m_cDevs.SetExtendedStyle(LVS_EX_FULLROWSELECT);
    
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
    
    OnViewRefresh();
    
    RECT rc;
    GetWindowRect(&rc);
    rc.top = GetApp().GetProfileInt(REGSTR_DEVICESWINDOW, REGSTR_POSITIONTOP, rc.top);
    rc.bottom = GetApp().GetProfileInt(REGSTR_DEVICESWINDOW, REGSTR_POSITIONBOTTOM, rc.bottom);
    rc.right = GetApp().GetProfileInt(REGSTR_DEVICESWINDOW, REGSTR_POSITIONRIGHT, rc.right);
    rc.left = GetApp().GetProfileInt(REGSTR_DEVICESWINDOW, REGSTR_POSITIONLEFT, rc.left);
    rc.bottom = max(rc.bottom, rc.top + 20);
    rc.right = max(rc.right, rc.left + 20);
    MoveWindow(&rc);

    BOOL bIsVisible = GetApp().GetProfileInt(REGSTR_DEVICESWINDOW, REGSTR_VISIBLE, TRUE);
    ShowWindow(bIsVisible ? SW_NORMAL : SW_HIDE);

    SetTimer(1971, 1000, NULL);
    
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CDevicesDlg::OnDestroy() 
{
    RECT rc;
    GetWindowRect(&rc);
    GetApp().WriteProfileInt(REGSTR_DEVICESWINDOW, REGSTR_POSITIONTOP, rc.top);
    GetApp().WriteProfileInt(REGSTR_DEVICESWINDOW, REGSTR_POSITIONBOTTOM, rc.bottom);
    GetApp().WriteProfileInt(REGSTR_DEVICESWINDOW, REGSTR_POSITIONRIGHT, rc.right);
    GetApp().WriteProfileInt(REGSTR_DEVICESWINDOW, REGSTR_POSITIONLEFT, rc.left);

    CDialog::OnDestroy();
}

void CDevicesDlg::OnViewRefresh() 
{
    UINT nSelCount = m_cDevs.GetSelectedCount();
    int nSelection = -1;
    CString sSelection;
    if(1 == nSelCount)
    {
        nSelection = m_cDevs.GetNextItem(-1, LVNI_SELECTED);
        sSelection = m_cDevs.GetItemText(nSelection, 0);
    }
    m_cDevs.DeleteAllItems();
    
    CSetupDIMgr mgr;
    while(mgr.Enum())
    {
        CString sName;
        if(mgr.HardwareID(sName))
        {
            //TRACE("Enumerated >%s<\n", sName);
            CString sFilter;
            sFilter.LoadString(mgr.IsLowerFilterPresent(m_sFilterName) ? IDS_INSTALLED : IDS_NOTINSTALLED);
            CString sDescription;
            mgr.DeviceName(sDescription);
            int nIndex = m_cDevs.InsertItem(-1, sName);
            m_cDevs.SetItemText(nIndex, 1, sFilter);
            m_cDevs.SetItemText(nIndex, 2, sDescription);
            m_nCX0 = max(m_nCX0, m_cDevs.GetStringWidth(sName));
            m_nCX1 = max(m_nCX1, m_cDevs.GetStringWidth(sFilter));
            m_nCX2 = max(m_nCX2, m_cDevs.GetStringWidth(sDescription));
        }
    }
    
    const int GAP = 3 * GetSystemMetrics(SM_CXFRAME);
    m_cDevs.SetColumnWidth(0, m_nCX0 + GAP);
    m_cDevs.SetColumnWidth(1, m_nCX1 + GAP);
    m_cDevs.SetColumnWidth(2, m_nCX2 + GAP);
    m_cDevs.Invalidate();
    
    if(-1 != nSelection)
    {
        LVFINDINFO fi;
        ZeroMemory(&fi, sizeof(fi));
        fi.flags = LVFI_STRING;
        fi.psz = sSelection;
        nSelection = m_cDevs.FindItem(&fi);
        m_cDevs.SetItemState(nSelection, LVIS_SELECTED, LVIS_SELECTED);
    }

    OnRefreshSnpysStatus();
}

void CDevicesDlg::OnRefreshSnpysStatus(void)
{
    CString sStatus;
    if(GetApp().IsSnpysPresent())
    {
        sStatus.Format(_T("Snpys bridge is present and accessible (%d out of %d entries used)."), GetApp().SnpysUsage(), MAX_SNOOPY_DRIVERS);
    }
    else
    {
        sStatus.Format(_T("Snpys bridge is not available (check Readme for instructions)."));
    }
    m_cDriverStatus.SetWindowText(sStatus);
}

BOOL CDevicesDlg::HasValidSelection(void)
{
    UINT nSelected = m_cDevs.GetSelectedCount();
    if(0 == nSelected)
    {
        AfxMessageBox(IDS_PLEASE_SELECT_DEVICE);
        return FALSE;
    }
    if(1 != nSelected)
    {
        AfxMessageBox(IDS_PLEASE_SELECT_ONE_DEVICE);
        return FALSE;
    }
    return TRUE;
}

BOOL CDevicesDlg::GetSelectedHardwareID(CString& sHardwareID)
{
    UINT nSelected = m_cDevs.GetSelectedCount();
    if(1 != nSelected)
    {
        return FALSE;
    }
    
    int nIndex = m_cDevs.GetNextItem(-1, LVNI_SELECTED);
    sHardwareID = m_cDevs.GetItemText(nIndex, 0);
    return TRUE;
}

void CDevicesDlg::OnRestartDevice() 
{
    if(!HasValidSelection())
        return;

    CString sHardwareID;
    if(GetSelectedHardwareID(sHardwareID))
    {
        TRACE("Restarting %s\n", sHardwareID);
        CSetupDIMgr mgr;
        if(mgr.FindHardwareID(sHardwareID))
        {
            mgr.RestartDevice();
        }
        OnViewRefresh();
    }
}

void CDevicesDlg::OnInstallSniffer() 
{
    if(!HasValidSelection())
        return;

    CString sHardwareID;
    if(GetSelectedHardwareID(sHardwareID))
    {
        TRACE("Installing on '%s'\n", sHardwareID);
        CSetupDIMgr mgr;
        if(mgr.FindHardwareID(sHardwareID))
        {
	        TRACE("Adding lower fileter '%s'\n", m_sFilterName);
            mgr.AddLowerFilter(m_sFilterName);
        }
        OnViewRefresh();
    }
}

void CDevicesDlg::OnInstallRestart() 
{
    if(!HasValidSelection())
        return;

    OnInstallSniffer();
    OnRestartDevice();
}

void CDevicesDlg::OnUninstallSniffer() 
{
    if(!HasValidSelection())
        return;
    
    CString sHardwareID;
    if(GetSelectedHardwareID(sHardwareID))
    {
        TRACE("Uninstalling from %s\n", sHardwareID);
        CSetupDIMgr mgr;
        if(mgr.FindHardwareID(sHardwareID))
        {
            mgr.RemoveLowerFilter(m_sFilterName);
        }
        OnViewRefresh();
    }
}

void CDevicesDlg::OnUninstallRestart() 
{
    if(!HasValidSelection())
        return;

    OnUninstallSniffer();
    OnRestartDevice();
}

void CDevicesDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
    if(::IsWindow(m_cDevs.m_hWnd))
    {
        CRect devsrc, statusrc;
        m_cDriverStatus.GetWindowRect(&statusrc);
        m_cDevs.GetWindowRect(&devsrc);
        ScreenToClient(&statusrc);
        ScreenToClient(&devsrc);
        statusrc.OffsetRect(-statusrc.TopLeft());
        statusrc.OffsetRect(5, 5);
        statusrc.right = cx - 5;
        devsrc.top = statusrc.bottom + 5;
        devsrc.right = cx - 5;
        devsrc.bottom = cy - 5;
        m_cDriverStatus.MoveWindow(statusrc);
        m_cDevs.MoveWindow(devsrc);
    }
}

void CDevicesDlg::OnRclickDevices(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
    CMenu ctx;
    ctx.LoadMenu(IDR_DEVICE_CONTEXT);
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
        popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
    }
    
	*pResult = 0;
}

void CDevicesDlg::OnInstallService() 
{
    if(m_bIsWin2K)
    {
        CSetupDIMgr mgr;
        CString sFilterFileName, sFilterDesc;
        sFilterFileName.LoadString(IDS_FILTERFILENAME);
        sFilterFileName = "System32\\Drivers\\" + sFilterFileName;
        sFilterDesc.LoadString(IDS_FILTERSERVICEDESCRIPTION);
        sFilterFileName.MakeLower();
		TRACE("About to install filter name '%s', file '%s', desc '%s'\n",
		                     m_sFilterName, sFilterFileName, sFilterDesc);
        mgr.InstallService(m_sFilterName, sFilterFileName, sFilterDesc);

        CString sBridgeName, sBridgeFileName, sBridgeDesc;
        sBridgeName.LoadString(IDS_BRIDGESERVICENAME);
		
        sBridgeFileName.LoadString(IDS_BRIDGEFILENAME_2K);
        sBridgeFileName = "System32\\Drivers\\" + sBridgeFileName;
        sBridgeDesc.LoadString(IDS_BRIDGESERVICEDESCRIPTION);
        sBridgeName.MakeLower();
        sBridgeFileName.MakeLower();
		TRACE("About to install filter name '%s', file '%s', desc '%s'\n",
		                     sBridgeName, sBridgeFileName, sBridgeDesc);
        mgr.InstallService(sBridgeName, sBridgeFileName, sBridgeDesc);
		TRACE("About to start '%s'\n",sBridgeName);
        mgr.StartService(sBridgeName);
    }
    else
    {
        CloseSniffer();
        OpenSniffer();
    }
}

void CDevicesDlg::OnUninstallService() 
{
    if(m_bIsWin2K)
    {
        CSetupDIMgr mgr;
        mgr.RemoveService(m_sFilterName);
        CString sBridgeName;
        sBridgeName.LoadString(IDS_BRIDGESERVICENAME);
        sBridgeName.MakeLower();
        mgr.StopService(sBridgeName);
        mgr.RemoveService(sBridgeName);
    }
    else
    {
        CloseSniffer();
    }
}

void CDevicesDlg::ToggleYourself(void)
{
    if(IsWindowVisible())
    {
        GetApp().WriteProfileInt(REGSTR_DEVICESWINDOW, REGSTR_VISIBLE, FALSE);
	    CDialog::OnCancel();
    }
    else
    {
        ShowWindow(SW_NORMAL);
        SetForegroundWindow();
        GetApp().WriteProfileInt(REGSTR_DEVICESWINDOW, REGSTR_VISIBLE, TRUE);
    }
}

void CDevicesDlg::OnCancel() 
{
    ToggleYourself();
}

BOOL UnpackAndInstall(UINT nID, LPCTSTR sSubDir, LPCTSTR sFilename)
{
    HRSRC hRsrc = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nID), _T("DRVR"));
    if(NULL == hRsrc)
    {
        TRACE("Couldn't locate driver binary in the resources!");
        return FALSE;
    }

    DWORD dwImageSize = SizeofResource(AfxGetResourceHandle(), hRsrc);
    if(0 == dwImageSize)
    {
        TRACE("Size of driver binary image is 0!");
        return FALSE;
    }

    HGLOBAL hBinImage = LoadResource(AfxGetResourceHandle(), hRsrc);
    if(NULL == hBinImage)
    {
        TRACE("Couldn't load driver binary image from resources!");
        return FALSE;
    }

    PVOID pBinImage = LockResource(hBinImage);
    if(NULL == pBinImage)
    {
        TRACE("Couldn't lock driver binary image from resources!");
        return FALSE;
    }

    TCHAR sWinDir[MAX_PATH];
    if(0 == GetWindowsDirectory(sWinDir, MAX_PATH))
    {
        TRACE("There was an error getting the windows directory!\n");
        return FALSE;
    }

    PathAppend(sWinDir, sSubDir);
    PathAppend(sWinDir, sFilename);

TRACE("~1 Copying from nID %d to '%s'\n",nID,sWinDir);
    HANDLE hFile = CreateFile(
        sWinDir,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if(INVALID_HANDLE_VALUE == hFile)
    {
        TRACE("Error creating >%s<: might be in use or protected by some rights!\n", sWinDir);
        return FALSE;
    }

    DWORD dwWritten = 0;
    if(!WriteFile(hFile, pBinImage, dwImageSize, &dwWritten, NULL))
    {
        TRACE("Failed to write %d bytes to >%s<\n", dwImageSize, sWinDir);
        CloseHandle(hFile);
        return FALSE;
    }

    if(!SetEndOfFile(hFile))
    {
        TRACE("Couldn't truncate disk file to %d bytes! ... proceed at your own risk!", dwImageSize);
    }

    CloseHandle(hFile);
    
    return TRUE;
}

void CDevicesDlg::OnUnpackDrivers() 
{
    CString sFilename, sMsg;
    sFilename.LoadString(IDS_FILTERFILENAME);
    if(!UnpackAndInstall(BIN_USBSNOOP,
	                     g_bIsWow64 ? _T("Sysnative\\Drivers") : _T("System32\\Drivers"),
	                     sFilename))
    {
        sMsg.Format(_T("There was an error while unpacking >%s<!"), sFilename);
        AfxMessageBox(sMsg, MB_ICONERROR);
        return;
    }

    sFilename.LoadString(IDS_BRIDGEFILENAME_2K);
    if(!UnpackAndInstall(BIN_USBSNPYS,
	                        g_bIsWow64 ? _T("Sysnative\\Drivers") : _T("System32\\Drivers"),
		                    sFilename))
    {
        sMsg.Format(_T("There was an error while unpacking >%s<!"), sFilename);
        AfxMessageBox(sMsg, MB_ICONERROR);
        return;
    }

	AfxMessageBox(_T("Driver were successfully unpacked..."), MB_ICONINFORMATION);
}

void CDevicesDlg::OnViewDevices() 
{
    ToggleYourself();	
}

BOOL CDevicesDlg::PreTranslateMessage(MSG* pMsg) 
{
    static HACCEL hAccel = LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
    TranslateAccelerator(m_hWnd, hAccel, pMsg);
	return CDialog::PreTranslateMessage(pMsg);
}

void CDevicesDlg::OnTimer(UINT_PTR nIDEvent) 
{
    OnRefreshSnpysStatus();
    
	CDialog::OnTimer(nIDEvent);
}

//** end of DevicesDlg.cpp ***********************************************
/*************************************************************************

  $Log: DevicesDlg.cpp,v $
  Revision 1.2  2002/10/05 01:10:43  rbosa
  Added the basic framework for exporting a log into an XML file. The
  output written is fairly poor. This checkin is mainly to get the
  framework in place and get feedback on it.

  Revision 1.1  2002/08/14 23:03:35  rbosa
  the application to capture urbs and display them...

 * 
 * 4     2/22/02 6:10p Rbosa
 * - added log

*************************************************************************/
