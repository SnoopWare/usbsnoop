#if !defined(AFX_EXPORTLOGDLG_H__60536F4F_9D09_43FD_BF64_F2758FF754F6__INCLUDED_)
#define AFX_EXPORTLOGDLG_H__60536F4F_9D09_43FD_BF64_F2758FF754F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportLogDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportLogDlg dialog

#include "Exporter.h"

class CExportLogDlg : public CDialog
{
// Construction
public:
	CExportLogDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportLogDlg)
	enum { IDD = IDD_EXPORT_LOG };
	CEdit	m_cRange;
	CButton	m_cExportSelection;
	CButton	m_cExportRange;
	CButton	m_cExportAll;
	CComboBox	m_cFormat;
	CStatic	m_cFilename;
	BOOL	m_bExportAll;
	BOOL	m_bExportRange;
	BOOL	m_bExportSelection;
	CString	m_sRange;
	//}}AFX_DATA


public:
    enum EXPORTER_TYPE m_ExporterType;
    CString m_sFilename;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportLogDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK();

	// Generated message map functions
	//{{AFX_MSG(CExportLogDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBrowse();
	afx_msg void OnExportAll();
	afx_msg void OnExportRange();
	afx_msg void OnExportSelection();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTLOGDLG_H__60536F4F_9D09_43FD_BF64_F2758FF754F6__INCLUDED_)
