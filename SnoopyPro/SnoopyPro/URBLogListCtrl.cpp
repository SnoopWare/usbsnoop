// URBLogListCtrl.cpp : implementation file
//

#include "StdAfx.h"
#include "SnoopyPro.h"
#include "URBLogListCtrl.h"
#include "URB.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CURBLogListCtrl

CURBLogListCtrl::CURBLogListCtrl()
: m_parURB(NULL)
{
    m_pImageList = new CImageList();
    m_pImageList->Create(16, 16, TRUE, 4, 1);
    m_pImageList->Add(GetApp().LoadIcon(IDI_CLOSED));
    m_pImageList->Add(GetApp().LoadIcon(IDI_OPEN));
    m_pImageList->Add(GetApp().LoadIcon(IDI_CLOSEDINV));
    m_pImageList->Add(GetApp().LoadIcon(IDI_OPENINV));

    m_pCourier = new CFont();
    m_pCourier->CreatePointFont(100, "Courier");
}

CURBLogListCtrl::~CURBLogListCtrl()
{
    m_parURB = NULL;

    if(NULL != m_pImageList)
    {
        delete m_pImageList;
        m_pImageList = NULL;
    }

    if(NULL != m_pCourier)
    {
        delete m_pCourier;
        m_pCourier = NULL;
    }
}

void CURBLogListCtrl::OnInitialUpdate(void)
{
    SetImageList(m_pImageList, LVSIL_STATE);

    m_ColWidths.SetSize(LOGCOL_MAX_COLUMN, -1);
    m_ColWidths[LOGCOL_EXPAND] =  30;
    m_ColWidths[LOGCOL_SEQUENCE] =  30;
    m_ColWidths[LOGCOL_DIRECTION] =  80;
    m_ColWidths[LOGCOL_ENDPOINT] = 30;
    m_ColWidths[LOGCOL_TIME] =  50;
    m_ColWidths[LOGCOL_FUNCTION] = 100;
    m_ColWidths[LOGCOL_DATA] = 100;
    m_ColWidths[LOGCOL_RESULT] =  30;
    
    InsertColumn(LOGCOL_EXPAND,     "*",          LVCFMT_CENTER, m_ColWidths[LOGCOL_EXPAND]);
    InsertColumn(LOGCOL_SEQUENCE,   "Seq",        LVCFMT_RIGHT,  m_ColWidths[LOGCOL_SEQUENCE]);
    InsertColumn(LOGCOL_DIRECTION,  "Dir",        LVCFMT_LEFT,   m_ColWidths[LOGCOL_DIRECTION]);
    InsertColumn(LOGCOL_ENDPOINT,   "Endpoint",   LVCFMT_CENTER, m_ColWidths[LOGCOL_ENDPOINT]);
    InsertColumn(LOGCOL_TIME,       "Time",       LVCFMT_RIGHT,  m_ColWidths[LOGCOL_TIME]);
    InsertColumn(LOGCOL_FUNCTION,   "Function",   LVCFMT_LEFT,   m_ColWidths[LOGCOL_FUNCTION]);
    InsertColumn(LOGCOL_DATA,       "Data",       LVCFMT_LEFT,   m_ColWidths[LOGCOL_DATA]);
    InsertColumn(LOGCOL_RESULT,     "Result",     LVCFMT_RIGHT,  m_ColWidths[LOGCOL_RESULT]);
}

void CURBLogListCtrl::AutoFitColumns(void)
{
    for(int nCol = 0; nCol < LOGCOL_MAX_COLUMN; ++nCol)
    {
        SetColumnWidth(nCol, m_ColWidths[nCol]);
    }
    Invalidate();
}


void CURBLogListCtrl::OnDestroy() 
{
    SetImageList(NULL, LVSIL_STATE);
    CListCtrl::OnDestroy();
}

void CURBLogListCtrl::SetURBArray(CArrayURB *parURB)
{
    m_parURB = parURB;
    if(NULL != m_parURB)
    {
        int nLineCnt = m_parURB->GetSize();
        m_arData.SetSize(nLineCnt);
        for(int nLine = 0; nLine < nLineCnt; nLine++)
        {
            m_arData[nLine] = URB2DATA(nLine, 0, 1);
            CURB *pURB = m_parURB->ElementAt(nLine);
            ASSERT(NULL != pURB);
            if(NULL != pURB)
            {
                pURB->SetExpanded(FALSE);
            }
        }
        SetItemCount(nLineCnt);
    }
}

int CURBLogListCtrl::FindURB(int nURB)
{
    for(int nIndex = 0; nIndex < m_arData.GetSize(); nIndex++)
    {
        if((unsigned) nURB == m_arData[nIndex].nURB)
            return nIndex;
    }
    return -1;
}

int CURBLogListCtrl::GetLine(int nIndex)
{
    ASSERT(0 <= nIndex);
    ASSERT(nIndex < m_arData.GetSize());
    
    return m_arData[nIndex].nLine;
}

int CURBLogListCtrl::GetURB(int nIndex)
{
    ASSERT(0 <= nIndex);
    ASSERT(nIndex < m_arData.GetSize());

    return m_arData[nIndex].nURB;
}

int CURBLogListCtrl::GetState(int nIndex)
{
    ASSERT(0 <= nIndex);
    ASSERT(nIndex < m_arData.GetSize());
    
    return m_arData[nIndex].nState;
}

int CURBLogListCtrl::FindFocusedURB(void)
{
    int nItemCount = GetItemCount();
    for(int nIndex = 0; nIndex < nItemCount; ++nIndex)
    {
        if(0 != GetItemState(nIndex, LVIS_FOCUSED))
        {
            return GetURB(nIndex);
        }
    }
    return -1;
}

void CURBLogListCtrl::SetFocusedURB(int nURB, BOOL bSetFocus /* = TRUE */)
{
    if(-1 != nURB)
    {
        int nIndex = FindURB(nURB);
        if(-1 != nIndex)
        {
            SetItemState(nIndex, bSetFocus ? LVIS_FOCUSED : 0, LVIS_FOCUSED);
        }
    }
}

BOOL CURBLogListCtrl::GetSelectionArrays(CArrayData &arData, CMyDWORDArray &arURBs)
{
    POSITION pos = GetFirstSelectedItemPosition();
    if(NULL == pos)
    {
        TRACE("Not proceeding, since no selection...\n");
        return FALSE;
    }

    arData.SetSize(GetSelectedCount());
    arURBs.SetSize(GetSelectedCount());
    int nURBCnt = 0;
    while(pos)
    {
        int nIndex = GetNextSelectedItem(pos);
        arData.SetAt(nURBCnt, m_arData[nIndex]);
        arURBs.SetAt(nURBCnt, m_arData[nIndex].nURB);
        nURBCnt++;
    }
    arURBs.RemoveDuplicates();
    return TRUE;
}

void CURBLogListCtrl::ExpandItem(int nIndex)
{
    int nLine = GetLine(nIndex);
    ASSERT(0 == nLine);

    int nURB = GetURB(nIndex);

    CURB *pURB = m_parURB->GetAt(nURB);

    int nState = GetState(nIndex);
    if(1 == nState)
    {
        m_arData[nIndex].nState = 2;
        int nInsert = pURB->GetExpandSize();

        int nPrevSize = m_arData.GetSize();
        int nNewSize = nPrevSize + nInsert;
        m_arData.SetSize(nNewSize);
        int i = 0;
        for(i = nPrevSize - 1; i > nIndex; i--)
        {
            m_arData[i + nInsert] = m_arData[i];
        }

        for(i = 0; i < nInsert; i++)
        {
            m_arData[nIndex + i + 1].nURB = nURB;
            m_arData[nIndex + i + 1].nLine = i + 1;
            m_arData[nIndex + i + 1].nState = 0;
        }

        SetItemCount(nNewSize);
    }
}

void CURBLogListCtrl::CollapseItem(int nIndex)
{
    int nLine = GetLine(nIndex);
    ASSERT(0 == nLine);
    int nURB = GetURB(nIndex);
    CURB *pURB = m_parURB->GetAt(nURB);
    int nState = GetState(nIndex);
    if(2 == nState)
    {
        m_arData[nIndex].nState = 1;
        int nInsert = pURB->GetExpandSize();

        int nPrevSize = m_arData.GetSize();
        int nNewSize = nPrevSize - nInsert;
        for(int i = nIndex + 1; i < nNewSize; i++)
        {
            m_arData[i] = m_arData[i + nInsert];
        }
        m_arData.SetSize(nNewSize);
        SetItemCount(nNewSize);
    }
}

BEGIN_MESSAGE_MAP(CURBLogListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CURBLogListCtrl)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CURBLogListCtrl message handlers

BOOL CURBLogListCtrl::GetURBText(int nURB, int nLine, int nCol, LPTSTR sBuffer)
{
    if(NULL == m_parURB)
    {
        TRACE("GetURBText: no URB array set!\n");
        return FALSE;
    }

    if(nURB >= m_parURB->GetSize())
    {
        TRACE("GetURBText: URB index (%d) out of bounds (%d)!\n",
            nURB, m_parURB->GetSize());
        return FALSE;
    }

    CURB *pURB = m_parURB->GetAt(nURB);
    return GetURBText(pURB, nLine, nCol, sBuffer);
}

BOOL CURBLogListCtrl::GetURBText(CURB *pURB, int nLine, int nCol, LPTSTR sBuffer)
{
    ASSERT(NULL != pURB);
    if(NULL == pURB)
    {
        TRACE("GetURBText(NULL, ....)!\n");
        return FALSE;
    }

    if(nLine > pURB->GetExpandSize())
    {
        TRACE("GetURBText: line (%d) out of bounds (%d) !\n",
            nLine, pURB->GetExpandSize());
        return FALSE;
    }

    switch(nCol)
    {
    case LOGCOL_EXPAND:
        if(nLine > 0)
            _tcscpy(sBuffer, pURB->GetExpandedLine(nLine - 1));
        else
            return FALSE;
        break;

    case LOGCOL_SEQUENCE:
        pURB->GetSequenceNo(sBuffer);
        break;

    case LOGCOL_DIRECTION:
        pURB->GetDirection(sBuffer);
        break;

    case LOGCOL_ENDPOINT:
        pURB->GetEndpoint(sBuffer);
        break;

    case LOGCOL_TIME:
        _stprintf(sBuffer, "%5.3f", pURB->GetTime(m_parURB) / 1000.0f);
        break;

    case LOGCOL_FUNCTION:
        _tcscpy(sBuffer, pURB->GetFunctionStr());
        break;

    case LOGCOL_DATA:
        pURB->GetDataDump(sBuffer);
        break;

    case LOGCOL_RESULT:
        if(pURB->IsComingUp())
        {
            _stprintf(sBuffer, "0x%08x", pURB->GetStatus());
        }
        else
        {
            sBuffer[0] = 0x00;
        }
        break;
        
    default:
        TRACE("GetURBText: unhandled column!\n");
        return FALSE;
    }

    return TRUE;
}

void CURBLogListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
    if(NULL == m_parURB)
    {
        TRACE("Not drawing anything, since there's no URB array set...\n");
        return;
    }

    ASSERT(NULL != lpDIS);
    ASSERT(ODT_LISTVIEW == lpDIS->CtlType);

    int nItemID = lpDIS->itemID;

/*
    TRACE("DrawItem(%d):", nItemID);
    if(ODA_DRAWENTIRE & lpDIS->itemAction)
        TRACE(" DrawEntire");
    if(ODA_FOCUS & lpDIS->itemAction)
        TRACE(" Focus");
    if(ODA_SELECT & lpDIS->itemAction)
        TRACE(" Select");
    TRACE("\n");

    TRACE(" State:");
    if(ODS_DEFAULT & lpDIS->itemState)
        TRACE(" Default");
    if(ODS_SELECTED & lpDIS->itemState)
        TRACE(" Selected");
    if(ODS_FOCUS & lpDIS->itemState)
        TRACE(" Focus");
    TRACE("\n");

    //int nData = lpDIS->itemData;
*/    
    DATADESC DD = m_arData[nItemID];

    int nURB = DATA2URB(DD);
    int nLine = DATA2LINE(DD);
//    TRACE(" Data: %08x (URB: %d, Line: %d)\n", DD, nURB, nLine);
        
    if(ODA_DRAWENTIRE & lpDIS->itemAction)
    {
        CRect rc(lpDIS->rcItem);

        CDC dc;
        dc.Attach(lpDIS->hDC);

        if(ODS_FOCUS & lpDIS->itemState)
        {
            CPen pen(PS_DOT, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
            CPen *old = dc.SelectObject(&pen);
            dc.Rectangle(rc);
            dc.SelectObject(old);
            rc.DeflateRect(1, 1);
        }

        if(ODS_SELECTED & lpDIS->itemState)
        {
            dc.FillSolidRect(rc, GetSysColor(COLOR_HIGHLIGHT));
            dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
        }
        else
        {
            dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
            rc.bottom--;
            CURB *pURB = m_parURB->GetAt(nURB);
            if(pURB->IsComingUp())
                dc.FillSolidRect(rc, RGB(0xC0, 0xDC, 0xC0));
            else
                dc.FillSolidRect(rc, RGB(0xD8, 0xD0, 0xC8));
            CRect line(rc);
            line.top = line.bottom;
            line.bottom++;
            dc.FillSolidRect(line, RGB(0xC0, 0xC0, 0xC0));
        }
        
        TCHAR szText[MAX_PATH];

        LVCOLUMN col;
        ZeroMemory(&col, sizeof(col));
        col.mask = LVCF_FMT | LVCF_WIDTH;
        int nCol = 0;
        GetColumn(nCol, &col);

        if(0 == nLine)
        {
            while(GetColumn(nCol, &col))
            {
                if(LOGCOL_EXPAND == nCol)
                {
                    // draw an image?
                    CImageList *pImgList = GetImageList(LVSIL_STATE);
                    int nState = m_arData[nItemID].nState;
                    if(NULL != pImgList)
                    {
                        if((1 == nState) || (2 == nState))
                        {
                            if(ODS_SELECTED & lpDIS->itemState)
                            {
                                nState += 2;
                            }
                            pImgList->Draw(&dc, nState - 1, rc.TopLeft(), ILD_TRANSPARENT);
                        }
                    }
                    rc.right = rc.left + col.cx;
                }
                else
                {
                    int fmt = (col.fmt & LVCFMT_JUSTIFYMASK);
                    rc.right = rc.left + col.cx;
                    UINT uFlags = DT_VCENTER | DT_SINGLELINE | DT_MODIFYSTRING | DT_END_ELLIPSIS;
                    uFlags |= (LVCFMT_LEFT == fmt) ? DT_LEFT : 0;
                    uFlags |= (LVCFMT_RIGHT == fmt) ? DT_RIGHT : 0;
                    uFlags |= (LVCFMT_CENTER == fmt) ? DT_CENTER : 0;
                    if(GetURBText(nURB, 0, nCol, szText))
                    {
                        CRect drc(rc);
                        CSize size = dc.GetTextExtent(szText);
                        m_ColWidths[nCol] = max(m_ColWidths[nCol], size.cx + 2 * GetSystemMetrics(SM_CXEDGE));
                        drc.DeflateRect(GetSystemMetrics(SM_CXEDGE), (drc.Height() - size.cy) / 2);
                        dc.DrawText(szText, -1, drc, uFlags);
                    }
                }
                rc.left = rc.right;
                nCol++;
            }
        }
        else
        {
            // draw expanded text....
            rc.left += col.cx;
            UINT uFlags = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_MODIFYSTRING | DT_END_ELLIPSIS;
            if(GetURBText(nURB, nLine, 0, szText))
            {
                CRect drc(rc);
                CFont *pOldFont = dc.SelectObject(m_pCourier);
                CSize size = dc.GetTextExtent(szText);
                drc.DeflateRect(GetSystemMetrics(SM_CXEDGE), (drc.Height() - size.cy) / 2);
                dc.DrawText(szText, -1, drc, uFlags);
                dc.SelectObject(pOldFont);
            }
        }
            
        dc.Detach();
    }
}
