// SnoopyPro.cpp : Defines the class behaviors for the application.
//

#include "StdAfx.h"
#include "SnoopyPro.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "USBLogDoc.h"
#include "USBLogView.h"
#include "URLStatic.h"
#include "ReadmeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// GWG
void gwg_trace(const char *format, ...) {
	FILE *fp;
	va_list args;

	va_start(args, format);
	if ((fp = fopen("log.txt", "a+")) != NULL) {
		vfprintf(fp, format, args);
		fclose(fp);
	}
	va_end(args);
}
// GWG

//////////////////////////////////////////////////////////////////////////
// CSnoopyProApp

BEGIN_MESSAGE_MAP(CSnoopyProApp, CWinApp)
    //{{AFX_MSG_MAP(CSnoopyProApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_HELP_README, OnHelpReadme)
	//}}AFX_MSG_MAP
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
    // Standard print setup command
    ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// CSnoopyProApp construction

CSnoopyProApp::CSnoopyProApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
    m_dwTotalWorkTicks = 1;
    m_dwWorkTicks = 0;
    m_uClipboardFormat = 0;
    VERIFY(m_DocsNeedingUpdate.IsEmpty());
    m_bIsSnpysPresent = FALSE;
    m_nSnpysUsage = 0;
}

//////////////////////////////////////////////////////////////////////////
// The one and only CSnoopyProApp object

CSnoopyProApp theApp;

CSnoopyProApp &GetApp(void)
{
    return (CSnoopyProApp &) theApp;
}

//////////////////////////////////////////////////////////////////////////
// CSnoopyProApp initialization

BOOL CSnoopyProApp::InitInstance()
{
    // Standard initialization
    // If you are not using these features and wish to reduce the size
    //  of your final executable, you should remove from the following
    //  the specific initialization routines you do not need.
    
#ifdef _AFXDLL
    Enable3dControls();         // Call this when using MFC in a shared DLL
#else
    Enable3dControlsStatic();   // Call this when linking to MFC statically
#endif

    // Change the registry key under which our settings are stored.
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization.
    SetRegistryKey(_T("SnoopWare"));
    
    LoadStdProfileSettings();  // Load standard INI file options (including MRU)

    CString sClipboardFormat;
    sClipboardFormat.LoadString(IDS_CLIPBOARDFORMAT);
    m_uClipboardFormat = RegisterClipboardFormat(sClipboardFormat);
    
    // Register the application's document templates.  Document templates
    //  serve as the connection between documents, frame windows and views.
    
    CMultiDocTemplate* pDocTemplate;
    pDocTemplate = new CMultiDocTemplate(
        IDR_USBLOGTYPE,
        RUNTIME_CLASS(CUSBLogDoc),
        RUNTIME_CLASS(CChildFrame), // custom MDI child frame
        RUNTIME_CLASS(CUSBLogView));
    AddDocTemplate(pDocTemplate);
    
    // create main MDI Frame window
    CMainFrame* pMainFrame = new CMainFrame;
    if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
        return FALSE;
    m_pMainWnd = pMainFrame;
    
    // Enable drag/drop open
    m_pMainWnd->DragAcceptFiles();
    
    // Enable DDE Execute open
    EnableShellOpen();
    RegisterShellFileTypes(TRUE);
    
    // Parse command line for standard shell commands, DDE, file open
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);
    
    // avoid opening an empty log on start...
    if(CCommandLineInfo::FileNew == cmdInfo.m_nShellCommand)
    {
        cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
    }
    
    // Dispatch commands specified on the command line
    if (!ProcessShellCommand(cmdInfo))
        return FALSE;
    
    // The main window has been initialized, so show and update it.
    pMainFrame->ShowWindow(m_nCmdShow);
    pMainFrame->UpdateWindow();

    return TRUE;
}

void CSnoopyProApp::ScanForNewSnoopedDevices(void)
{
    TRACE("Accessing usbsnoopies to find new devices...\n");
    HANDLE hSniffer = CreateFile(USBSNPYS_W32NAME_2K, 
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, // no SECURITY_ATTRIBUTES structure
        OPEN_EXISTING, // No special create flags
        0, // No special attributes
        NULL);
    
    if(INVALID_HANDLE_VALUE == hSniffer)
    {
        hSniffer = CreateFile(USBSNPYS_W32NAME_9X,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, // no SECURITY_ATTRIBUTES structure
            OPEN_EXISTING,
            FILE_FLAG_DELETE_ON_CLOSE,
            NULL);
    }

    m_bIsSnpysPresent = (INVALID_HANDLE_VALUE != hSniffer);

    if(INVALID_HANDLE_VALUE != hSniffer)
    {
        DWORD dwBytesReturned = 0;
        SNOOPED_DEVICES SnoopedDevices;
        ZeroMemory(&SnoopedDevices, sizeof(SnoopedDevices));

        BOOL bResult = DeviceIoControl(hSniffer, USBSNOOP_GET_SNOOPED_DEVS, 
                &SnoopedDevices, sizeof(SnoopedDevices),
                &SnoopedDevices, sizeof(SnoopedDevices),
                &dwBytesReturned, NULL);
        CloseHandle(hSniffer);
        hSniffer = INVALID_HANDLE_VALUE;

        TRACE("DeviceIoControl(): got %d snooped devices\n", SnoopedDevices.uCount);
        m_nSnpysUsage = SnoopedDevices.uCount;
        if(bResult)
        {
            TRACE("Devices:\n");
            for(ULONG nSnooped = 0; nSnooped < SnoopedDevices.uCount; ++nSnooped)
            {
                PSNOOPED_DEVICE pSnooped = &SnoopedDevices.Entry[nSnooped];
                TRACE(" Dev[%d] = 0x%08x\n", nSnooped, pSnooped->uDeviceID);
                if(INVALID_DEVICE_ID == pSnooped->uDeviceID)
                {
                    TRACE("  .. skipping\n");
                }
                else
                {
                    BOOL bAlreadySnooped = FALSE;
                    if(!m_DocsNeedingUpdate.IsEmpty())
                    {
                        POSITION pos = m_DocsNeedingUpdate.GetHeadPosition();
                        while(NULL != pos)
                        {
                            CUSBLogDoc *pDoc = m_DocsNeedingUpdate.GetNext(pos);
                            ASSERT(NULL != pDoc);
                            if(NULL != pDoc)
                            {
                                if(pSnooped->uDeviceID == pDoc->GetSnifferDevice())
                                {
                                    bAlreadySnooped = TRUE;
                                    break;
                                }
                            }
                        }
                    }
                    if(!bAlreadySnooped)
                    {
                        CUSBLogDoc *pDoc = CreateNewDocument();
                        pDoc->AccessSniffer(pSnooped->uDeviceID, pSnooped->sHardwareIDs);
                        pDoc->SetTimeStampZero(SnoopedDevices.Entry[nSnooped].uTimeStampZero);
                    }
                }
            }
        }
    }
}

CUSBLogDoc *CSnoopyProApp::CreateNewDocument(void)
{
    ASSERT(NULL != m_pDocManager);
    POSITION pos = 	m_pDocManager->GetFirstDocTemplatePosition();
    CDocTemplate* pTemplate = (CDocTemplate*)m_pDocManager->GetNextDocTemplate(pos);
    ASSERT(pTemplate != NULL);
    ASSERT_KINDOF(CDocTemplate, pTemplate);
    CUSBLogDoc *pDocument = (CUSBLogDoc *) pTemplate->OpenDocumentFile(NULL);
    ASSERT(NULL != pDocument);
    ASSERT_KINDOF(CUSBLogDoc, pDocument);
    return pDocument;
}

void CSnoopyProApp::AddDocumentToIdleList(CUSBLogDoc *pDocument)
{
    m_DocsNeedingUpdate.AddTail(pDocument);
}

void CSnoopyProApp::RemoveDocumentFromIdleList(CUSBLogDoc *pDocument)
{
    POSITION pos = m_DocsNeedingUpdate.Find(pDocument);
    if(NULL != pos)
    {
        m_DocsNeedingUpdate.RemoveAt(pos);
    }
}

void CSnoopyProApp::SetPercent(int nPercent)
{
    CMainFrame *pMainFrame = (CMainFrame *) m_pMainWnd;
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    if((NULL != pMainFrame) && (pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame))))
    {
        pMainFrame->SetPercent(nPercent);
    }
}

void CSnoopyProApp::IncrementWorkTick(void)
{
    m_dwWorkTicks = min(m_dwWorkTicks + 1, m_dwTotalWorkTicks);
    ASSERT(0 != m_dwTotalWorkTicks);
    SetPercent(MulDiv(m_dwWorkTicks, 100, m_dwTotalWorkTicks));
}

void CSnoopyProApp::InitializeWorkTicks(DWORD dwTotalTicks)
{
    m_dwTotalWorkTicks = max(1, dwTotalTicks);
    m_dwWorkTicks = 0;
}

void CSnoopyProApp::DeinitializeWorkTicks(void)
{
    m_dwTotalWorkTicks = 1;
    m_dwWorkTicks = 0;
    SetPercent(0);
}

// This is called by the MFC despatch loop ?
BOOL CSnoopyProApp::OnIdle(LONG lCount) 
{
    CWinApp::OnIdle(lCount);

    int nActiveDocs = 0;
    CUSBLogDoc *pGoneInactive = NULL;
    if(!m_DocsNeedingUpdate.IsEmpty())
    {
        POSITION pos = m_DocsNeedingUpdate.GetHeadPosition();
        while(NULL != pos)
        {
            CUSBLogDoc *pDoc = m_DocsNeedingUpdate.GetNext(pos);
            ASSERT(NULL != pDoc);
            if(NULL != pDoc)
            {
                if(pDoc->GetNewURBSFromSniffer())
                {
                    ++nActiveDocs;
                }
                else
                {
                    pGoneInactive = pDoc;
                }
            }
        }
    }

    if(NULL != pGoneInactive)
    {
        POSITION pos = m_DocsNeedingUpdate.Find(pGoneInactive);
        ASSERT(NULL != pos);
        m_DocsNeedingUpdate.RemoveAt(pos);
    }

// ~~99
Sleep(100);
    // return zero for enough time used, non-zero for more processing
    return nActiveDocs;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    //{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CURLStatic	m_cURL;
	CString	m_sAppInfo;
	//}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAboutDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    //{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
	m_sAppInfo = _T("");
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_URL, m_cURL);
	DDX_Text(pDX, IDC_APP_INFO, m_sAppInfo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    m_sAppInfo.LoadString(IDS_APPINFO);
    CString sCopyright;
    sCopyright.LoadString(IDS_COPYRIGHT);
    m_sAppInfo += "\n";
    m_sAppInfo += sCopyright;

    UpdateData(FALSE);

    CString sURL;
    sURL.LoadString(IDS_ABOUT_URL);
    m_cURL.SetWindowText(sURL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// App command to run the dialog
void CSnoopyProApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

void CSnoopyProApp::OnHelpReadme() 
{
    CReadmeDlg readmeDlg;
    readmeDlg.DoModal();
}

