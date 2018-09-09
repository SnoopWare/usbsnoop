//************************************************************************
//
// URLStatic.cpp
//
//************************************************************************

#include "StdAfx.h"
#include "URLStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CURLStatic

CURLStatic::CURLStatic()
{
	m_bFontInitialized = FALSE;
}

CURLStatic::~CURLStatic()
{
	if(m_bFontInitialized)
	{
		m_fBlueUnderlined.DeleteObject();
	}
}


BEGIN_MESSAGE_MAP(CURLStatic, CStatic)
	//{{AFX_MSG_MAP(CURLStatic)
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_NCHITTEST()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CURLStatic message handlers

void CURLStatic::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if(!m_bFontInitialized)
	{
		m_bFontInitialized = TRUE;
		LOGFONT lf;
		ZeroMemory(&lf, sizeof(lf));
		lf.lfUnderline = TRUE;
		_tcscpy(lf.lfFaceName, _T("MS Sans Serif"));
		lf.lfHeight = -MulDiv(8, dc.GetDeviceCaps(LOGPIXELSY), 72);
		m_fBlueUnderlined.CreateFontIndirect(&lf);
	}

	CFont *pOldFont = dc.SelectObject(&m_fBlueUnderlined);
	CString sText;
	GetWindowText(sText);
	dc.SetTextColor(RGB(0, 0, 255));
	dc.SetBkMode(TRANSPARENT);
    dc.SetTextAlign(TA_TOP | TA_CENTER);

    CRect rc;
    GetClientRect(&rc);
	dc.TextOut(rc.Width() / 2, 0, sText);

	dc.SelectObject(pOldFont);
	
	// Do not call CStatic::OnPaint() for painting messages
}

void CURLStatic::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CString s;
	GetWindowText(s);

    HKEY hReg = NULL;
	if(ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,
		_T("http\\shell\\open\\command"), &hReg))
	{
		CString url;
		LONG lSize = MAX_PATH;
		if(ERROR_SUCCESS == RegQueryValue(hReg, NULL, url.GetBuffer(lSize), &lSize))
		{
			url.ReleaseBuffer();
			if(-1 != url.Find(_T("%1")))
            {
                url.Replace(_T("%1"), s);
            }
            else
            {
                url += " ";
                url += s;
            }
			WinExec((LPCSTR)(LPCTSTR)url, SW_NORMAL);
		}
		RegCloseKey(hReg);
		hReg = NULL;
	}
	
	CStatic::OnLButtonUp(nFlags, point);
}

BOOL CURLStatic::CheckMouse(void) 
{
    CPoint ptMouse;
    GetCursorPos(&ptMouse);
    
    CRect rcExtents;
    GetWindowRect(&rcExtents);
    return PtInRect(rcExtents, ptMouse);
}

LRESULT CURLStatic::OnNcHitTest(CPoint point) 
{
	LRESULT nResult = CStatic::OnNcHitTest(point);
    if(HTCLIENT == nResult)
    {
        // tofix: this doesn't seem to have an effect!
        SetCursor(LoadCursor(NULL, IDC_HAND));
    }
    return nResult;
}

//** end of URLStatic.cpp ************************************************
/*************************************************************************

  $Log: URLStatic.cpp,v $
  Revision 1.1  2002/08/14 23:03:34  rbosa
  the application to capture urbs and display them...

 * 
 * 1     1/25/02 2:43p Rbosa

*************************************************************************/
