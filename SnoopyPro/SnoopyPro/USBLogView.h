// USBLogView.h : interface of the CUSBLogView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_USBLOGVIEW_H__F4DBF00A_465D_4E37_8485_D1902D9EC481__INCLUDED_)
#define AFX_USBLOGVIEW_H__F4DBF00A_465D_4E37_8485_D1902D9EC481__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "URBLogListCtrl.h"
#include "MyMemFile.h"

class CUSBLogView : public CFormView
{
protected: // create from serialization only
	CUSBLogView();
	DECLARE_DYNCREATE(CUSBLogView)

public:
	//{{AFX_DATA(CUSBLogView)
	enum { IDD = IDD_SNOOPYPRO_FORM };
	CButton	m_cStop;
	CButton	m_cPlayPause;
	CStatic	m_cHardwareID;
	CButton	m_cTimeFormat;
	CStatic	m_cStatus;
	CURBLogListCtrl	m_Log;
	//}}AFX_DATA

// Attributes
public:
	CUSBLogDoc* GetDocument();

    void ExpandItem(int nIndex);
    void CollapseItem(int nIndex);

// Operations
public:
    void SerializeURBsToArchive(CFile *pFile, CMyDWORDArray &arURBs);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUSBLogView)
	public:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUSBLogView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    int m_nTimeFormat;
    int m_nTotalPackets;
    BOOL m_bSniffing;
    void OnEditCopyCutDelete(BOOL bPlaceInClipboard, BOOL bDeleteFromThis);
    void OnEditSelectNone(void);

    void DoLayout(void);

    HBITMAP m_hbmPlay;
    HBITMAP m_hbmPause;
    HBITMAP m_hbmStop;

// Generated message map functions
protected:
	//{{AFX_MSG(CUSBLogView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCollapse();
	afx_msg void OnExpand();
	afx_msg void OnClickUsblog(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAutofitColumns();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectAllDirDown();
	afx_msg void OnEditSelectAllDirUp();
	afx_msg void OnTimeFormat();
	afx_msg void OnPlayPause();
	afx_msg void OnStop();
	afx_msg void OnToolsAnalyzeLog();
	afx_msg void OnFileExport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in USBLogView.cpp
inline CUSBLogDoc* CUSBLogView::GetDocument()
   { return (CUSBLogDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USBLOGVIEW_H__F4DBF00A_465D_4E37_8485_D1902D9EC481__INCLUDED_)
