//************************************************************************
//
// ProgressStatusBar.cpp
//
//************************************************************************

#include "StdAfx.h"
#include "ProgressStatusBar.h"


CProgressStatusBar::CProgressStatusBar(void)
{
    m_Percent = 0;
}

void CProgressStatusBar::ActivateProgressBar(UINT nCommand)
{
    SetPaneStyle(CommandToIndex(nCommand), SBPS_OWNERDRAW);
}

void CProgressStatusBar::DrawItem(LPDRAWITEMSTRUCT pDIS)
{
    //TRACE("CProgressStatusBar::DrawItem: %d percent!\n", m_Percent);
    CDC dc;
    dc.Attach(pDIS->hDC);
    CRect rc(pDIS->rcItem);
    CRect rc2(rc);
    rc.right = rc.left + MulDiv(rc.Width(), m_Percent, 100);
    rc2.left = rc.right;
    dc.FillSolidRect(&rc, GetSysColor(COLOR_BTNTEXT));
    dc.FillSolidRect(&rc2, GetSysColor(COLOR_BTNFACE));
    dc.Detach();
}

void CProgressStatusBar::SetPercent(int nPercent)
{
    if(nPercent != m_Percent)
    {
        m_Percent = nPercent;
        UpdateAllPanes(TRUE, TRUE);
        Invalidate(FALSE);
        UpdateWindow();
    }
}


//** end of ProgressStatusBar.cpp ****************************************
/*************************************************************************

  $Log: not supported by cvs2svn $
 * 
 * 2     2/05/02 8:59p Rbosa
 * - removed debug output
 * 
 * 1     1/25/02 2:43p Rbosa

*************************************************************************/