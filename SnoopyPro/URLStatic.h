#if !defined(AFX_URLSTATIC_H__7C804781_6C67_11D4_A3B3_00A0C9971EFC__INCLUDED_)
#define AFX_URLSTATIC_H__7C804781_6C67_11D4_A3B3_00A0C9971EFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// URLStatic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CURLStatic window

class CURLStatic : public CStatic
{
// Construction
public:
	CURLStatic();

// Attributes
public:
	BOOL m_bFontInitialized;
	CFont m_fBlueUnderlined;

// Operations
public:
    BOOL CheckMouse(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CURLStatic)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CURLStatic();

	// Generated message map functions
protected:
	//{{AFX_MSG(CURLStatic)
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg UINT OnNcHitTest(CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_URLSTATIC_H__7C804781_6C67_11D4_A3B3_00A0C9971EFC__INCLUDED_)
