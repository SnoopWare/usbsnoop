#if !defined(AFX_URBLOGLISTBOX_H__6CBCBF9E_BB12_47ED_BB41_DFF76FA55103__INCLUDED_)
#define AFX_URBLOGLISTBOX_H__6CBCBF9E_BB12_47ED_BB41_DFF76FA55103__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// URBLogListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CURBLogListBox window

class CURBLogListBox : public CListBox
{
// Construction
public:
	CURBLogListBox();

// Attributes
public:

// Operations
public:
    BOOL Create(const RECT &rect, CWnd *pParentWnd);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CURBLogListBox)
	public:
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CURBLogListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CURBLogListBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_URBLOGLISTBOX_H__6CBCBF9E_BB12_47ED_BB41_DFF76FA55103__INCLUDED_)
