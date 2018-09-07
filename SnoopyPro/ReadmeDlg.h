#if !defined(AFX_READMEDLG_H__BCBE3762_BEF3_11D4_A3B4_00A0C9971EFC__INCLUDED_)
#define AFX_READMEDLG_H__BCBE3762_BEF3_11D4_A3B4_00A0C9971EFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ReadmeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CReadmeDlg dialog

#include "Resource.h"

class CReadmeDlg : public CDialog
{
// Construction
public:
	CReadmeDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CReadmeDlg(void);

// Dialog Data
	//{{AFX_DATA(CReadmeDlg)
	enum { IDD = IDD_README };
	CEdit	m_cReadme;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReadmeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    CFont m_fCourier;

	// Generated message map functions
	//{{AFX_MSG(CReadmeDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_READMEDLG_H__BCBE3762_BEF3_11D4_A3B4_00A0C9971EFC__INCLUDED_)
