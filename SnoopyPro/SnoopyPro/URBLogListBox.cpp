// URBLogListBox.cpp : implementation file
//

#include "stdafx.h"
#include "SnoopyPro.h"
#include "URBLogListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CURBLogListBox

CURBLogListBox::CURBLogListBox()
{
}

CURBLogListBox::~CURBLogListBox()
{
}

BOOL CURBLogListBox::Create(const RECT &rect, CWnd *pParentWnd)
{
    DWORD dwStyle = LBS_STANDARD | LBS_OWNERDRAWVARIABLE | LBS_NODATA | WS_VISIBLE;
    return CListBox::Create(dwStyle, rect, pParentWnd, 4711);
}

BEGIN_MESSAGE_MAP(CURBLogListBox, CListBox)
	//{{AFX_MSG_MAP(CURBLogListBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CURBLogListBox message handlers

int CURBLogListBox::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct) 
{
    if(lpCompareItemStruct->itemData1 < lpCompareItemStruct->itemData2)
        return -1;
    if(lpCompareItemStruct->itemData1 > lpCompareItemStruct->itemData2)
        return 1;

	// return -1 = item 1 sorts before item 2
	// return 0 = item 1 and item 2 sort the same
	// return 1 = item 1 sorts after item 2

	return 0;
}

void CURBLogListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
    TRACE("CURBLogListBox::DrawItem()\n");

   ASSERT(lpDrawItemStruct->CtlType == ODT_LISTBOX);
   LPCTSTR lpszText = (LPCTSTR) lpDrawItemStruct->itemData;
   //ASSERT(lpszText != NULL);
   CDC dc;
   
   dc.Attach(lpDrawItemStruct->hDC);
   
   // Save these value to restore them when done drawing.
   COLORREF crOldTextColor = dc.GetTextColor();
   COLORREF crOldBkColor = dc.GetBkColor();
   
   // If this item is selected, set the background color 
   // and the text color to appropriate values. Also, erase
   // rect by filling it with the background color.
   if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
       (lpDrawItemStruct->itemState & ODS_SELECTED))
   {
       dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
       dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
       dc.FillSolidRect(&lpDrawItemStruct->rcItem, 
           ::GetSysColor(COLOR_HIGHLIGHT));
   }
   else
       dc.FillSolidRect(&lpDrawItemStruct->rcItem, crOldBkColor);
   
   // If this item has the focus, draw a red frame around the
   // item's rect.
   if ((lpDrawItemStruct->itemAction | ODA_FOCUS) &&
       (lpDrawItemStruct->itemState & ODS_FOCUS))
   {
       CBrush br(RGB(255, 0, 0));
       dc.FrameRect(&lpDrawItemStruct->rcItem, &br);
   }
   
   // Draw the text.
   CString S;
   S.Format("%08x", lpszText);
   dc.DrawText(
       S,
       strlen(S),
       &lpDrawItemStruct->rcItem,
       DT_CENTER|DT_SINGLELINE|DT_VCENTER);
   
   // Reset the background color and the text color back to their
   // original values.
   dc.SetTextColor(crOldTextColor);
   dc.SetBkColor(crOldBkColor);
   
   dc.Detach();
}

void CURBLogListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
    TRACE("CURBLogListBox::MeasureItem()\n");
    
    ASSERT(lpMeasureItemStruct->CtlType == ODT_LISTBOX);
    LPCTSTR lpszText = (LPCTSTR) lpMeasureItemStruct->itemData;
    ASSERT(lpszText != NULL);
    CSize   sz;
    CDC*    pDC = GetDC();
    
    CString S;
    S.Format("%08x", lpszText);
    sz = pDC->GetTextExtent(S);
    
    ReleaseDC(pDC);
    
    lpMeasureItemStruct->itemHeight = 2*sz.cy;
}
