//************************************************************************
//
// ProgressStatusBar.h
//
//************************************************************************

#ifndef _PROGRESSSTATUSBAR_H_INCLUDED_ 
#define _PROGRESSSTATUSBAR_H_INCLUDED_ 

class CProgressStatusBar : public CStatusBar
{
public:
    CProgressStatusBar(void);

    void ActivateProgressBar(UINT nCommand);

    virtual void DrawItem(LPDRAWITEMSTRUCT pDIS);

    void SetPercent(int nPercent);

protected:
    int m_Percent;
};


#endif // !_PROGRESSSTATUSBAR_H_INCLUDED_ 

//** end of ProgressStatusBar.h ******************************************
/*************************************************************************

  $Log: not supported by cvs2svn $
 * 
 * 1     1/25/02 2:43p Rbosa

*************************************************************************/