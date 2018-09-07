#if !defined(AFX_URBLOGLISTCTRL_H__18DC6A89_419B_4438_B1F5_99ADC46D39D8__INCLUDED_)
#define AFX_URBLOGLISTCTRL_H__18DC6A89_419B_4438_B1F5_99ADC46D39D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// URBLogListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CURBLogListCtrl window

#include "URB.h"

typedef struct {
    unsigned nLine:MAX_LINES_PER_URB_LOG2;
    unsigned nURB:32-MAX_LINES_PER_URB_LOG2-2;
    unsigned nState:2;
} DATADESC;

inline DATADESC URB2DATA(int nURB, int nLine, int nState)
{
    DATADESC DD = { nLine, nURB, nState };
    return DD;
}

inline int DATA2URB(const DATADESC DD)
{
    return DD.nURB;
}

inline int DATA2LINE(const DATADESC DD)
{
    return DD.nLine;
}

inline int DATA2STATE(const DATADESC DD)
{
    return DD.nState;
}


class CArrayData : public CArray<DATADESC, DATADESC&>
{
};

typedef enum {
    LOGCOL_EXPAND,
    LOGCOL_SEQUENCE,
    LOGCOL_DIRECTION,
    LOGCOL_ENDPOINT,
    LOGCOL_TIME,
    LOGCOL_FUNCTION,
    LOGCOL_DATA,
    LOGCOL_RESULT,
    LOGCOL_MAX_COLUMN
} LOG_COLUMNS;

class CURBLogListCtrl : public CListCtrl
{
    CArrayURB *m_parURB;

    CImageList *m_pImageList;
    CFont *m_pCourier;

// Construction
public:
	CURBLogListCtrl();

// Attributes
public:
    CArrayData m_arData;
    CArray<int, int> m_ColWidths;

// Operations
public:
    void OnInitialUpdate(void);
    void AutoFitColumns(void);
    
    void SetURBArray(CArrayURB *parURB);

    // given an URB, this returns the index in this list
    int FindURB(int nURB);

    // given an index, this returns the line displayed
    int GetLine(int nIndex);
    
    // given an index, this returns the URB it contains
    int GetURB(int nIndex);

    // given an index, this returns the state of the URB in the listctrl
    int GetState(int nIndex);

    // gives the URB which has the focus, -1 if none has it
    int FindFocusedURB(void);

    // sets the focus on the given URB, does nothing if -1 is given
    void SetFocusedURB(int nURB, BOOL bSetFocus = TRUE);

    // retrieves the selection as DATADESC entries
    BOOL GetSelectionArrays(CArrayData &arData, CMyDWORDArray &arURBs);

    void ExpandItem(int nIndex);
    void CollapseItem(int nIndex);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CURBLogListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CURBLogListCtrl();

    BOOL GetURBText(int nURB, int nLine, int nCol, LPTSTR sBuffer);
    BOOL GetURBText(CURB *pURB, int nLine, int nCol, LPTSTR sBuffer);

	// Generated message map functions
protected:
	//{{AFX_MSG(CURBLogListCtrl)
	afx_msg void OnDestroy();
	//}}AFX_MSG
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_URBLOGLISTCTRL_H__18DC6A89_419B_4438_B1F5_99ADC46D39D8__INCLUDED_)
