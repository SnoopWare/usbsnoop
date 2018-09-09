// MainFrm.cpp : implementation of the CMainFrame class
//

#include "StdAfx.h"
#include "SnoopyPro.h"
#include "MainFrm.h"
#include "SetupDIMgr.h"
#include "USBLogView.h"
#include "USBLogDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

BOOL g_bIs64bitsys = FALSE;	// TRUE if this is a 64 bit operating system (64 bit drivers)
BOOL g_bIsWow64 = FALSE;	// TRUE if this is 32 bit App and 64 bit kernel

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_DEVICES, OnViewDevices)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
    ON_WM_DEVICECHANGE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_SNPYS_STATUS,
    ID_INDICATOR_PROGRESS,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    ZeroMemory(&m_NotificationFilter, sizeof(m_NotificationFilter));
    m_hDevNotify = NULL;

#ifdef _WIN64
	g_bIs64bitsys = TRUE;
	g_bIsWow64 = FALSE;
#else
	g_bIs64bitsys = FALSE;

	LPFN_ISWOW64PROCESS fnIsWow64Process;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
		GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
	if (fnIsWow64Process != NULL) {
		if (!fnIsWow64Process(GetCurrentProcess(),&g_bIs64bitsys))
			g_bIs64bitsys = FALSE;		// Error - assume it's false
	}
	g_bIsWow64 = g_bIs64bitsys;
#endif // _WIN64
 
}

CMainFrame::~CMainFrame()
{
    if(NULL != m_hDevNotify)
    {
        UnregisterDeviceNotification(m_hDevNotify);
        m_hDevNotify = NULL;        
    }
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE("Failed to create status bar\n");
		return -1;      // fail to create
	}

    m_wndStatusBar.ActivateProgressBar(ID_INDICATOR_PROGRESS);

    if(!m_dlgDevices.Create(m_dlgDevices.IDD, this))
    {
        TRACE("Failed to create devices dialog\n");
        return -1;
    }

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

    RegisterForDeviceChange(GUID_CLASS_USB_DEVICE);

    SetTimer(1971, 1000, NULL);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

BOOL CMainFrame::RegisterForDeviceChange(GUID ClassGuid)
{
    ZeroMemory(&m_NotificationFilter, sizeof(m_NotificationFilter));
    m_NotificationFilter.dbcc_size = sizeof(m_NotificationFilter);
    m_NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    m_NotificationFilter.dbcc_classguid = ClassGuid;

    m_hDevNotify = RegisterDeviceNotification(m_hWnd, &m_NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
    if(NULL == m_hDevNotify)
    {
        DWORD dwLastError = GetLastError();
        TRACE("Failed to register for device notifications for device with %d (0x%08x)...\n", dwLastError, dwLastError);
    }
    return (NULL != m_hDevNotify);
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnViewDevices() 
{
    m_dlgDevices.ToggleYourself();
}

void CMainFrame::GetURBS(void)
{
    CMDIChildWnd *pChild = MDIGetActive();
    if(NULL != pChild)
    {
        // Get the active view attached to the active MDI child
        // window.
        CUSBLogView *pView = (CUSBLogView*) pChild->GetActiveView();
        if(NULL != pView)
        {
            CUSBLogDoc *pDoc = (CUSBLogDoc*) pView->GetDocument();
            if(NULL != pDoc)
            {
                pDoc->GetNewURBSFromSniffer();
            }
        }
    }
}

BOOL CMainFrame::OnDeviceChange(UINT nEventType, DWORD dwData)
{
    TRACE("CMainFrame::OnWMDeviceChange(0x%08x, 0x%08x)\n", nEventType, dwData);
    switch(nEventType)
    {
    case DBT_DEVICEARRIVAL:
        {
            TRACE("DBT_DEVICEARRIVAL:\n");
            PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR) dwData;
            switch(pHdr->dbch_devicetype)
            {
            case DBT_DEVTYP_DEVICEINTERFACE:
                {
                    PDEV_BROADCAST_DEVICEINTERFACE pDevInterface = (PDEV_BROADCAST_DEVICEINTERFACE) pHdr;
                    TRACE("Arrived: %s\n", pDevInterface->dbcc_name);
                    GetApp().ScanForNewSnoopedDevices();
                }
                break;
            default:
                break;
            }
        }
        break;

    case DBT_DEVICEREMOVEPENDING:
        {
            TRACE("DBT_DEVICEREMOVEPENDING:\n");
            PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR) dwData;
            switch(pHdr->dbch_devicetype)
            {
            case DBT_DEVTYP_HANDLE:
                {
                    PDEV_BROADCAST_HANDLE pDevHandle = (PDEV_BROADCAST_HANDLE) pHdr;
                    TRACE("Pending remove on %08x\n", pDevHandle->dbch_handle);
                    CloseHandle(pDevHandle->dbch_handle);
                }
                break;
            }
        }
        break;

    case DBT_DEVICEREMOVECOMPLETE:
        {
            TRACE("DBT_DEVICEREMOVECOMPLETE:\n");
        }
        break;

    default:
        break;
    }
    
    return CMDIFrameWnd::OnDeviceChange(nEventType, dwData);
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
    GetApp().ScanForNewSnoopedDevices();	
	CMDIFrameWnd::OnTimer(nIDEvent);
}
