// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__4AC6AE19_CA66_4FF0_B704_C366379D8AB7__INCLUDED_)
#define AFX_MAINFRM_H__4AC6AE19_CA66_4FF0_B704_C366379D8AB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DevicesDlg.h"
#include "ProgressStatusBar.h"

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:
    DEV_BROADCAST_DEVICEINTERFACE m_NotificationFilter;
    HDEVNOTIFY m_hDevNotify;
// Operations
public:
    CDevicesDlg m_dlgDevices;
    BOOL RegisterForDeviceChange(GUID ClassGuid);

    void GetURBS(void);

    void SetPercent(int nPercent)
    {
        m_wndStatusBar.SetPercent(nPercent);
    }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CProgressStatusBar m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewDevices();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
    afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__4AC6AE19_CA66_4FF0_B704_C366379D8AB7__INCLUDED_)
