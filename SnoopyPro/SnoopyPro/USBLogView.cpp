// USBLogView.cpp : implementation of the CUSBLogView class
//

#include "StdAfx.h"
#include "SnoopyPro.h"
#include "USBLogDoc.h"
#include "USBLogView.h"
#include "BuildNum.h"
#include "MainFrm.h"
#include "Resource.h"
#include "ExportLogDlg.h"
#include "Exporter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUSBLogView

IMPLEMENT_DYNCREATE(CUSBLogView, CFormView)

BEGIN_MESSAGE_MAP(CUSBLogView, CFormView)
    //{{AFX_MSG_MAP(CUSBLogView)
	ON_WM_SIZE()
	ON_COMMAND(ID_COLLAPSE, OnCollapse)
	ON_COMMAND(ID_EXPAND, OnExpand)
	ON_NOTIFY(NM_CLICK, IDC_USBLOG, OnClickUsblog)
	ON_COMMAND(ID_AUTOFIT_COLUMNS, OnAutofitColumns)
	ON_COMMAND(ID_EDIT_SELECTALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
	ON_COMMAND(ID_EDIT_SELECTALL_DIR_DOWN, OnEditSelectAllDirDown)
	ON_COMMAND(ID_EDIT_SELECTALL_DIR_UP, OnEditSelectAllDirUp)
	ON_BN_CLICKED(IDC_TIMEFORMAT, OnTimeFormat)
	ON_BN_CLICKED(IDC_PLAY_PAUSE, OnPlayPause)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_COMMAND(ID_TOOLS_ANALYZE_LOG, OnToolsAnalyzeLog)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	//}}AFX_MSG_MAP
    // Standard printing commands
    ON_COMMAND(ID_FILE_PRINT, CFormView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, CFormView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, CFormView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUSBLogView construction/destruction

CUSBLogView::CUSBLogView()
: CFormView(CUSBLogView::IDD)
{
    //{{AFX_DATA_INIT(CUSBLogView)
	//}}AFX_DATA_INIT
    // TODO: add construction code here
    m_nTotalPackets = -1;
    m_nTimeFormat = -1;

    m_hbmPlay = (HBITMAP) LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_PLAY), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
    m_hbmPause = (HBITMAP) LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_PAUSE), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
    m_hbmStop = (HBITMAP) LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_STOP), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
}

CUSBLogView::~CUSBLogView()
{
    DeleteObject(m_hbmPlay);
    DeleteObject(m_hbmPause);
    DeleteObject(m_hbmStop);
}

void CUSBLogView::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CUSBLogView)
	DDX_Control(pDX, IDC_STOP, m_cStop);
	DDX_Control(pDX, IDC_PLAY_PAUSE, m_cPlayPause);
	DDX_Control(pDX, IDC_HARDWAREID, m_cHardwareID);
	DDX_Control(pDX, IDC_TIMEFORMAT, m_cTimeFormat);
	DDX_Control(pDX, IDC_STATUS, m_cStatus);
	DDX_Control(pDX, IDC_USBLOG, m_Log);
	//}}AFX_DATA_MAP
}

void CUSBLogView::OnInitialUpdate()
{
    TRACE("CUSBLogView::OnInitialUpdate()\n");

	CFormView::OnInitialUpdate();
	ResizeParentToFit();

    CUSBLogDoc *pDoc = GetDocument();

    m_Log.OnInitialUpdate();

    m_Log.SetURBArray(&pDoc->m_arURB);

    for(int nURB = 0; nURB < pDoc->m_arURB.GetSize(); nURB++)
    {
        CURB *pURB = pDoc->m_arURB.GetAt(nURB);
        if(pURB->IsExpanded())
        {
            CollapseItem(m_Log.FindURB(nURB));
            ExpandItem(m_Log.FindURB(nURB));
        }
    }

    m_cStop.SetBitmap(m_hbmStop);
    m_cPlayPause.SetBitmap(m_hbmStop);

    OnUpdate(NULL, 0, NULL);
}

void CUSBLogView::ExpandItem(int nIndex)
{
    int nURB = m_Log.GetURB(nIndex);
    CUSBLogDoc *pDoc = GetDocument();
    pDoc->SetExpanded(nURB, TRUE);
}

void CUSBLogView::CollapseItem(int nIndex)
{
    int nURB = m_Log.GetURB(nIndex);
    CUSBLogDoc *pDoc = GetDocument();
    pDoc->SetExpanded(nURB, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CUSBLogView printing

CArrayData m_arPrintData;

BOOL CUSBLogView::OnPreparePrinting(CPrintInfo* pInfo)
{
    m_arPrintData.RemoveAll();
    m_arPrintData.SetSize(m_Log.m_arData.GetSize());
    for(int nIndex = 0; nIndex < m_Log.m_arData.GetSize(); ++nIndex)
    {
        m_arPrintData[nIndex] = m_Log.m_arData[nIndex];
    }
    TRACE("Printing %d lines of text...\n", m_arPrintData.GetSize());
    pInfo->SetMaxPage(m_arPrintData.GetSize());
    if(pInfo->m_pPD)
    {
        pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION;
    }
	return DoPreparePrinting(pInfo);
}

int m_nPageHeight = -1;
int m_nLinesPerPage = -1;
int m_nTextHeight;
INT nTabStops[LOGCOL_MAX_COLUMN];

void CUSBLogView::OnBeginPrinting(CDC* pDC, CPrintInfo* /*pInfo*/)
{
    if(-1 == m_nPageHeight)
    {
        m_nPageHeight = pDC->GetDeviceCaps(VERTRES);
        TEXTMETRIC TextMetric;
        if(pDC->GetTextMetrics(&TextMetric))
        {
            CSize szTextHeight;
            m_nTextHeight = TextMetric.tmHeight + TextMetric.tmExternalLeading + TextMetric.tmInternalLeading;
            szTextHeight.cy = m_nTextHeight;
            pDC->LPtoDP(&szTextHeight);
            m_nLinesPerPage = m_nPageHeight / szTextHeight.cy;
            TRACE("Lines Per Page: %d\n", m_nLinesPerPage);
        }
    }

    int nTotalColWidth = 0;
    for(int nCol = 0; nCol < LOGCOL_MAX_COLUMN; ++nCol)
    {
        nTotalColWidth += m_Log.m_ColWidths[nCol];
    }
    
    int nPageWidth = pDC->GetDeviceCaps(HORZRES);
    for(int nTabStop = 1; nTabStop < LOGCOL_MAX_COLUMN; ++nTabStop)
    {
        CSize szTab;
        szTab.cx = MulDiv(m_Log.m_ColWidths[nTabStop], nPageWidth, nTotalColWidth);
        szTab.cy = 0;
        pDC->DPtoLP(&szTab);
        nTabStops[nTabStop - 1] = szTab.cx;
    }
}

void CUSBLogView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
    CFormView::OnPrepareDC(pDC, pInfo);
    if(NULL != pInfo)
    {
        int nPBase = (pInfo->m_nCurPage - 1) * m_nLinesPerPage;
        pInfo->m_bContinuePrinting = (nPBase < m_arPrintData.GetSize());
    }
}

void CUSBLogView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
    CUSBLogDoc *pDoc = GetDocument();
    TCHAR sBuffer[MAX_PATH];
    int cy = 0;
    int nPBase = (pInfo->m_nCurPage - 1) * m_nLinesPerPage;
    for(int nPLine = 0; nPLine < m_nLinesPerPage; ++nPLine)
    {
        int nIndex = nPBase + nPLine;
        if(nIndex < m_arPrintData.GetSize())
        {
            int nURB = m_arPrintData[nPLine + nPBase].nURB;
            int nLine = m_arPrintData[nPLine + nPBase].nLine;
            CURB *pURB = pDoc->m_arURB[nURB];
            LPTSTR sOutput = sBuffer;
            sBuffer[0] = 0x00;
            if(0 == nLine)
            {
                for(int nCol = 0; nCol < LOGCOL_MAX_COLUMN; ++nCol)
                {
                    if(m_Log.GetURBText(pURB, nLine, nCol, sOutput))
                    {
                        sOutput += _tcslen(sOutput);
                        if((LOGCOL_MAX_COLUMN - 1) != nCol)
                        {
                            _tcscat(sOutput, "\t");
                            sOutput += _tcslen(sOutput);
                        }
                    }
                }
                pDC->TabbedTextOut(0, cy, sBuffer, _tcslen(sBuffer), nTabStops, 0);
            }
            else
            {
                if(m_Log.GetURBText(pURB, nLine, 0, sOutput))
                {
                    pDC->TextOut(0, cy, sBuffer, _tcslen(sBuffer));
                }
            }
            cy += m_nTextHeight;
        }
    }
}

void CUSBLogView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	m_nPageHeight = -1;
    m_nLinesPerPage = -1;
    m_arPrintData.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CUSBLogView diagnostics

#ifdef _DEBUG
void CUSBLogView::AssertValid(void) const
{
	CFormView::AssertValid();
}

void CUSBLogView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CUSBLogDoc* CUSBLogView::GetDocument(void) // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CUSBLogDoc)));
	return (CUSBLogDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CUSBLogView message handlers


void CUSBLogView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

    DoLayout();
}

void CUSBLogView::DoLayout(void)
{
    if(::IsWindow(m_Log.m_hWnd))
    {

        CRect rc, logrc;
        GetClientRect(&rc);
        m_Log.GetWindowRect(&logrc);
        ScreenToClient(&logrc);
        logrc.right = rc.Width() - 5;
        logrc.bottom = rc.Height() - 5;
        m_Log.MoveWindow(logrc);
        
        CRect statusrc, pauserc, stoprc, hardwareidrc, timeformatrc;
        m_cStatus.GetWindowRect(&statusrc);
        m_cPlayPause.GetWindowRect(&pauserc);
        m_cStop.GetWindowRect(&stoprc);
        m_cHardwareID.GetWindowRect(&hardwareidrc);
        m_cTimeFormat.GetWindowRect(&timeformatrc);
        ScreenToClient(&statusrc);
        ScreenToClient(&pauserc);
        ScreenToClient(&stoprc);
        ScreenToClient(&hardwareidrc);
        ScreenToClient(&timeformatrc);
        timeformatrc.OffsetRect(-timeformatrc.TopLeft());
        timeformatrc.OffsetRect(rc.Width() - 5 - timeformatrc.Width(), statusrc.top);
        if(m_cStop.IsWindowVisible())
        {
            hardwareidrc.left = stoprc.right + 5;
        }
        else
        {
            hardwareidrc.left = statusrc.right + 5;
        }
        hardwareidrc.right = timeformatrc.left - 5;
        m_cPlayPause.MoveWindow(pauserc);
        m_cStop.MoveWindow(stoprc);
        m_cHardwareID.MoveWindow(hardwareidrc);
        m_cTimeFormat.MoveWindow(timeformatrc);
    }
}

void CUSBLogView::OnCollapse(void)
{
    CWaitCursor waitcursor;
    CArrayData arData;
    CMyDWORDArray arURBS;
    if(!m_Log.GetSelectionArrays(arData, arURBS))
    {
        return;
    }

    int nURBCnt = arURBS.GetSize();

    // find the URB which has the focus...
    int nFocusedURB = m_Log.FindFocusedURB();
    m_Log.SetFocusedURB(nFocusedURB, FALSE);

    int nIndex = 0;
    while(nIndex < nURBCnt)
    {
        int nURB = arURBS.GetAt(nIndex);
        int nURBIndex = m_Log.FindURB(nURB);
        TRACE("Collapsing %d @ %d\n", nURB, nURBIndex);
        CollapseItem(nURBIndex);
        nIndex++;
    }

    // unselect all
    OnEditSelectNone();

    // reselect the right things....
    nIndex = 0;
    while(nIndex < arURBS.GetSize())
    {
        m_Log.SetItemState(m_Log.FindURB(arURBS.GetAt(nIndex)), LVIS_SELECTED, LVIS_SELECTED);
        nIndex++;
    }

    // set the focus back...
    m_Log.SetFocusedURB(nFocusedURB);
}

void CUSBLogView::OnExpand(void)
{
    CWaitCursor waitcursor;
    CArrayData arData;
    CMyDWORDArray arURBS;
    if(!m_Log.GetSelectionArrays(arData, arURBS))
    {
        return;
    }

    // find the URB which has the focus...
    int nFocusedURB = m_Log.FindFocusedURB();
    m_Log.SetFocusedURB(nFocusedURB, FALSE);

    int nIndex = arURBS.GetUpperBound();
    while(0 <= nIndex)
    {
        ExpandItem(m_Log.FindURB(arURBS.GetAt(nIndex)));
        nIndex--;
    }

    // unselect all
    OnEditSelectNone();

    // reselect the right things....
    nIndex = 0;
    while(nIndex < arURBS.GetSize())
    {
        m_Log.SetItemState(m_Log.FindURB(arURBS.GetAt(nIndex)), LVIS_SELECTED, LVIS_SELECTED);
        nIndex++;
    }

    // set the focus back...
    m_Log.SetFocusedURB(nFocusedURB);
}

void CUSBLogView::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* pHint)
{
    if((NULL == pHint) || (!pHint->IsKindOf(RUNTIME_CLASS(CURB))))
    {
        CUSBLogDoc *pDoc = GetDocument();
        if(pDoc->IsTimeFormatRelative() != m_nTimeFormat)
        {
            m_nTimeFormat = pDoc->IsTimeFormatRelative();
            m_cTimeFormat.SetCheck(pDoc->IsTimeFormatRelative() ? 1 : 0);
            m_Log.Invalidate();
        }

        if(pDoc->m_arURB.GetSize() != m_nTotalPackets)
        {
            m_nTotalPackets = pDoc->m_arURB.GetSize();
            CString sStatus;
            sStatus.Format("%d packets", m_nTotalPackets);
            m_cStatus.SetWindowText(sStatus);
            m_cStatus.Invalidate();
            m_cHardwareID.SetWindowText(pDoc->GetHardwareID());
            m_cHardwareID.Invalidate();
        }

        if(m_bSniffing != pDoc->IsSniffing())
        {
            m_bSniffing = pDoc->IsSniffing();
            m_cPlayPause.ShowWindow(m_bSniffing ? SW_NORMAL : SW_HIDE);
            m_cStop.ShowWindow(m_bSniffing ? SW_NORMAL : SW_HIDE);
            m_cPlayPause.EnableWindow(pDoc->IsSniffing());
            m_cStop.EnableWindow(pDoc->IsSniffing());
            DoLayout();
        }
        if(pDoc->IsSniffing())
        {
            if(pDoc->IsPaused())
            {
                m_cPlayPause.SetBitmap(m_hbmPlay);
            }
            else
            {
                m_cPlayPause.SetBitmap(m_hbmPause);
            }
        }

        if(0 != lHint)
        {
            int nURBCnt = pDoc->m_arURB.GetSize();
            GetApp().InitializeWorkTicks(nURBCnt);
            for(int nURB = 0; nURB < nURBCnt; ++nURB)
            {
                CURB *pURB = pDoc->m_arURB.ElementAt(nURB);
                ASSERT(NULL != pURB);
                if(NULL != pURB)
                {
                    pURB->GetExpandSize(); // this trigger allocation of all strings...
                    CMainFrame *pMainFrame = (CMainFrame*) AfxGetMainWnd();
                    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
                    pMainFrame->SetPercent(MulDiv(nURB, 100, nURBCnt));
                }
                GetApp().IncrementWorkTick();
            }
            GetApp().DeinitializeWorkTicks();
            m_Log.SetURBArray(&pDoc->m_arURB);
            return;
        }
        return;
    }

    CURB *pURB = (CURB *) pHint;

    int nURB = (int) lHint;
    int nIndex = m_Log.FindURB(nURB);
    int nState = m_Log.GetState(nIndex);
    if(pURB->IsExpanded() && (2 == nState))
    {
        // no change here...
        return;
    }

    if(!pURB->IsExpanded() && (1 == nState))
    {
        // no change needed...
        return;
    }

    if(pURB->IsExpanded())
    {
        ASSERT(1 == nState);
        m_Log.ExpandItem(nIndex);
        return;
    }

    if(!pURB->IsExpanded())
    {
        ASSERT(2 == nState);
        m_Log.CollapseItem(nIndex);
        return;
    }
}

void CUSBLogView::OnClickUsblog(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    // TRACE("OnClickUsbLog()\n");

    LVHITTESTINFO HitTestInfo;
    ZeroMemory(&HitTestInfo, sizeof(HitTestInfo));
    GetCursorPos(&HitTestInfo.pt);
    m_Log.ScreenToClient(&HitTestInfo.pt);
    int nResult = m_Log.SubItemHitTest(&HitTestInfo);

    /*
    TRACE("Result: %d\n", nResult);
    TRACE(" iItem: %d\n", HitTestInfo.iItem);
    TRACE(" iSubItem: %d\n", HitTestInfo.iSubItem);
    TRACE(" flags: %08x\n", HitTestInfo.flags);
    if(LVHT_ONITEM & HitTestInfo.flags)
        TRACE("  ONITEM\n");
    if(LVHT_ONITEMICON & HitTestInfo.flags)
        TRACE("  ONITEMICON\n");
    if(LVHT_ONITEMLABEL & HitTestInfo.flags)
        TRACE("  ONITEMLABEL\n");
    if(LVHT_ONITEMSTATEICON & HitTestInfo.flags)
        TRACE("  ONITEMSTATEICON\n");
    TRACE("  pt: %d/%d\n", HitTestInfo.pt.x, HitTestInfo.pt.y);
    */

    if(-1 != nResult)
    {
        int nURB = m_Log.GetURB(HitTestInfo.iItem);
        int nLine = m_Log.GetLine(HitTestInfo.iItem);
        if(0 == nLine)
        {
            // TRACE("On 0 line... acting if subitem is also 0...\n");
            if((0 == HitTestInfo.iSubItem) && (HitTestInfo.pt.x < 16))
            {
                CUSBLogDoc *pDoc = GetDocument();
                CURB *pURB = pDoc->m_arURB[nURB];
                if(pURB->IsExpanded())
                    CollapseItem(HitTestInfo.iItem);
                else
                    ExpandItem(HitTestInfo.iItem);
            }
        }
    }

	*pResult = 0;
}

void CUSBLogView::OnAutofitColumns(void)
{
    m_Log.AutoFitColumns();
}

void CUSBLogView::OnTimeFormat(void) 
{
    CUSBLogDoc *pDoc = GetDocument();
    pDoc->SetTimeFormat(1 == m_cTimeFormat.GetCheck());
    pDoc->UpdateAllViews(NULL);
}

void CUSBLogView::OnEditSelectNone(void)
{
    int nItemCount = m_Log.GetItemCount();
    for(int nItem = 0; nItem < nItemCount; ++nItem)
    {
        m_Log.SetItemState(nItem, 0, LVIS_SELECTED);
    }
}

void CUSBLogView::OnEditSelectAll(void)
{
    int nItemCount = m_Log.GetItemCount();
    for(int nItem = 0; nItem < nItemCount; ++nItem)
    {
        m_Log.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
    }
}

void CUSBLogView::OnEditSelectAllDirDown(void) 
{
    CUSBLogDoc *pDoc = GetDocument();
    int nItemCount = m_Log.GetItemCount();
    for(int nItem = 0; nItem < nItemCount; ++nItem)
    {
        int nURB = m_Log.GetURB(nItem);
        CURB *pURB = pDoc->m_arURB[nURB];
        if(!pURB->IsComingUp())
        {
            m_Log.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
        }
    }
}

void CUSBLogView::OnEditSelectAllDirUp(void) 
{
    CUSBLogDoc *pDoc = GetDocument();
    int nItemCount = m_Log.GetItemCount();
    for(int nItem = 0; nItem < nItemCount; ++nItem)
    {
        int nURB = m_Log.GetURB(nItem);
        CURB *pURB = pDoc->m_arURB[nURB];
        if(pURB->IsComingUp())
        {
            m_Log.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
        }
    }
}

void CUSBLogView::SerializeURBsToArchive(CFile *pFile, CMyDWORDArray &arURBs)
{
    ASSERT(NULL != pFile);

    CUSBLogDoc *pDoc = GetDocument();
    CArchive ar(pFile, CArchive::store);
    ar.WriteCount(_VERSION_DWORD_);
    int nURBCnt = arURBs.GetSize();
    ar.WriteCount(nURBCnt);
    for(int nURB = 0; nURB < nURBCnt; ++nURB)
    {
        CURB *pURB = pDoc->m_arURB.GetAt(nURB);
        ar.WriteClass(pURB->GetRuntimeClass());
        pURB->Serialize(ar);
        GetApp().IncrementWorkTick();
    }
    ar.Flush();
}

void CUSBLogView::OnEditCopyCutDelete(BOOL bPlaceInClipboard, BOOL bDeleteFromThis)
{
    CWaitCursor waitcursor;
    CArrayData arData;
    CMyDWORDArray arURBs;
    if(!m_Log.GetSelectionArrays(arData, arURBs))
    {
        return;
    }

    int nURBCnt = arURBs.GetSize();

    TRACE("URBs marked with selection: %d\n", nURBCnt);
    for(int i = 0; i < nURBCnt; ++i)
    {
        TRACE("  %d: %d\n", i, arURBs[i]);
    }

    GetApp().InitializeWorkTicks((bDeleteFromThis ? nURBCnt : 0) + 2 * (nURBCnt + arData.GetSize()));

    CUSBLogDoc *pDoc = GetDocument();
    if(bPlaceInClipboard)
    {
        CMyMemFile NullFile;
        SerializeURBsToArchive(&NullFile, arURBs);
        UINT nBinaryLength = NullFile.GetLength();
        TRACE("Binary Length: %d bytes\n", nBinaryLength);

        int nDataCnt = arData.GetSize();
        TCHAR sBuffer[MAX_PATH];
        DWORD dwTextLength = 0;
        DWORD dwTextSeparators = 0;
        for(int nIndex = 0; nIndex < nDataCnt; ++nIndex)
        {
            int nURB = arData[nIndex].nURB;
            int nLine = arData[nIndex].nLine;
            CURB *pURB = pDoc->m_arURB[nURB];
            if(0 == nLine)
            {
                // get accumulated length of all header
                // columns
                for(int nCol = 0; nCol < LOGCOL_MAX_COLUMN; ++nCol)
                {
                    if(m_Log.GetURBText(pURB, nLine, nCol, sBuffer))
                    {
                        dwTextLength += _tcslen(sBuffer);
                        dwTextSeparators += 2;
                    }
                }
            }
            else
            {
                if(m_Log.GetURBText(pURB, nLine, 0, sBuffer))
                {
                    dwTextLength += _tcslen(sBuffer);
                    dwTextSeparators += 2;
                }
            }
            GetApp().IncrementWorkTick();
        }
        TRACE("Text Length: %d bytes\n", dwTextLength);
        TRACE("Text Separators: %d bytes\n", dwTextSeparators);

        if(!AfxGetMainWnd()->OpenClipboard())
        {
            TRACE("Error while opening clipboard!\n");
            return;
        }
        // from here on, we have to close the clipboard
        // before exiting this function, otherwise other
        // applications might not get a chance to copy/paste
        // stuff!
        try
        {
            EmptyClipboard();

            DWORD dwTextTotalLength = dwTextLength + dwTextSeparators;
            HGLOBAL hText = GlobalAlloc(GMEM_MOVEABLE, (dwTextTotalLength + 1) * sizeof(TCHAR));
            if(NULL == hText)
            {
                AfxThrowMemoryException();
            }
            HGLOBAL hBinary = GlobalAlloc(GMEM_MOVEABLE, nBinaryLength);
            if(NULL == hBinary)
            {
                GlobalFree(hText);
                AfxThrowMemoryException();
            }

            LPTSTR pText = (LPTSTR) GlobalLock(hText);
            for(int nIndex = 0; nIndex < nDataCnt; ++nIndex)
            {
                int nURB = arData[nIndex].nURB;
                int nLine = arData[nIndex].nLine;
                CURB *pURB = pDoc->m_arURB[nURB];
                if(0 == nLine)
                {
                    for(int nCol = 0; nCol < LOGCOL_MAX_COLUMN; ++nCol)
                    {
                        if(m_Log.GetURBText(pURB, nLine, nCol, pText))
                        {
                            pText += _tcslen(pText);
                            if((LOGCOL_MAX_COLUMN - 1) != nCol)
                            {
                                _tcscat(pText, "\t");
                                pText += _tcslen(pText);
                            }
                        }
                    }
                }
                else
                {
                    if(m_Log.GetURBText(pURB, nLine, 0, pText))
                    {
                        pText += _tcslen(pText);
                    }
                }
                _tcscat(pText, "\n");
                pText += _tcslen(pText);
                GetApp().IncrementWorkTick();
            }
            GlobalUnlock(hText);

            PVOID pData = GlobalLock(hBinary);
            CMyMemFile memFile((PBYTE) pData);
            SerializeURBsToArchive(&memFile, arURBs);
            GlobalUnlock(hBinary);

            SetClipboardData(CF_TEXT, hText);
            SetClipboardData(GetApp().m_uClipboardFormat, hBinary);

            CloseClipboard();
        }
        catch(...)
        {
            CloseClipboard();
            throw;
        }
    }

    if(bDeleteFromThis)
    {
        int nFocusedURB = m_Log.FindFocusedURB();
        OnEditSelectNone();
        for(int nIndex = nURBCnt - 1; 0 <= nIndex; --nIndex)
        {
            int nURB = arURBs[nIndex];
            CURB *pURB = pDoc->m_arURB[nURB];
            delete pURB;
            pDoc->m_arURB.RemoveAt(nURB);
            GetApp().IncrementWorkTick();
        }
        pDoc->UpdateAllViews(NULL, 1);
        m_Log.SetFocusedURB(nFocusedURB);
    }

    GetApp().DeinitializeWorkTicks();
}

void CUSBLogView::OnEditCopy(void)
{
    OnEditCopyCutDelete(TRUE, FALSE);
}

void CUSBLogView::OnEditCut(void) 
{
    OnEditCopyCutDelete(TRUE, TRUE);
}

void CUSBLogView::OnEditDelete(void) 
{
    OnEditCopyCutDelete(FALSE, TRUE);
}

void CUSBLogView::OnEditPaste(void) 
{
    if(!IsClipboardFormatAvailable(GetApp().m_uClipboardFormat))
    {
        TRACE("OnEditPaste(): no suitable clipboard format available!\n");
        return;
    }

    // we need a focused URB as well...
    int nFocusedURB = m_Log.FindFocusedURB();
    if(-1 == nFocusedURB)
    {
        nFocusedURB = 0;
    }

    if(!AfxGetMainWnd()->OpenClipboard())
    {
        TRACE("Error while opening clipboard!\n");
        return;
    }
    // from here on, we have to close the clipboard
    // before exiting this function, otherwise other
    // applications might not get a chance to copy/paste
    // stuff!
    HANDLE hBinary = NULL;
    PVOID pData = NULL;
    try
    {
        hBinary = GetClipboardData(GetApp().m_uClipboardFormat);
        if(NULL == hBinary)
        {
            AfxThrowNotSupportedException();
        }

        pData = GlobalLock(hBinary);
        {
            CMyMemFile memFile(pData);
            CArchive ar(&memFile, CArchive::load);
            DWORD dwVersion = ar.ReadCount();
            if(_VERSION_DWORD_ != dwVersion)
            {
                AfxThrowNotSupportedException();
            }

            DWORD nURBCnt = ar.ReadCount();
            TRACE("Pasting %d URBs...\n", nURBCnt);
            
            CUSBLogDoc *pDoc = GetDocument();
            int nInsertLocation = nFocusedURB;
            while(0 < nURBCnt)
            {
                CRuntimeClass *pClass = ar.ReadClass();
                if(!pClass->IsDerivedFrom(RUNTIME_CLASS(CURB)))
                {
                    TRACE("unknown runtime class!\n");
                    AfxThrowArchiveException(CArchiveException::badClass);
                }

                CURB *pURB = (CURB*) pClass->CreateObject();
                ASSERT(NULL != pURB);
                pURB->SetChunkAllocator(&pDoc->m_arURB.m_ChunkAllocator);
                pURB->Serialize(ar);
                pDoc->m_arURB.InsertAt(nInsertLocation, pURB);
                nInsertLocation++;
                --nURBCnt;
            }
            pDoc->UpdateAllViews(NULL, 1);
            OnEditSelectNone();
            m_Log.SetFocusedURB(nFocusedURB);            
        }

        GlobalUnlock(hBinary);
        hBinary = NULL;

        CloseClipboard();
    }
    catch(...)
    {
        if((NULL != pData) && (NULL != hBinary))
        {
            GlobalUnlock(hBinary);
        }
        CloseClipboard();
        throw;
    }
}

void CUSBLogView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
    POSITION pos = m_Log.GetFirstSelectedItemPosition();
    pCmdUI->Enable(NULL != pos);
}

void CUSBLogView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
    POSITION pos = m_Log.GetFirstSelectedItemPosition();
    pCmdUI->Enable(NULL != pos);
}

void CUSBLogView::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
    POSITION pos = m_Log.GetFirstSelectedItemPosition();
    pCmdUI->Enable(NULL != pos);
}

void CUSBLogView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
    BOOL bPasteOK = IsClipboardFormatAvailable(GetApp().m_uClipboardFormat);
    pCmdUI->Enable(bPasteOK);
}

void CUSBLogView::OnPlayPause(void) 
{
    CUSBLogDoc *pDoc = GetDocument();
    pDoc->OnPlayPause();
}

void CUSBLogView::OnStop(void) 
{
    CUSBLogDoc *pDoc = GetDocument();
    pDoc->OnStop();
}

void CUSBLogView::OnToolsAnalyzeLog(void)
{
    CUSBLogDoc *pDoc = GetDocument();
    pDoc->AnalyzeLog();
}

void CUSBLogView::OnFileExport() 
{
    CUSBLogDoc *pDoc = GetDocument();
    CExportLogDlg dlg;
    if(IDOK == dlg.DoModal())
    {
        CExporter *pExporter = CExporter::Factory(dlg.m_ExporterType);
        if(NULL != pExporter)
        {
            pExporter->SetOutputFilename(dlg.m_sFilename);
            pExporter->SetArrayURB(&pDoc->m_arURB);
            pExporter->Export();
            delete pExporter;
        }
    }
}

//** end of USBLogView.cpp ***********************************************
/*************************************************************************

  $Log: not supported by cvs2svn $
 * 
 * 4     2/22/02 6:12p Rbosa
 * - added log
 * - added defines instead of raw numbers for columns

*************************************************************************/
