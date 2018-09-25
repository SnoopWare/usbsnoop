#if !defined(AFX_DEVICESDLG_H__BA191892_8E24_4FF6_9E55_537A748D1BA0__INCLUDED_)
#define AFX_DEVICESDLG_H__BA191892_8E24_4FF6_9E55_537A748D1BA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DevicesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDevicesDlg dialog

class CDevicesDlg : public CDialog
{
// Construction
public:
	CDevicesDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CDevicesDlg(void);

    HANDLE m_hSniffer;

    void OpenSniffer(void);
    void CloseSniffer(void);

    void OnRefreshSnpysStatus(void);

// Dialog Data
	//{{AFX_DATA(CDevicesDlg)
	enum { IDD = IDD_DEVICELIST };
	CStatic	m_cDriverStatus;
	CListCtrl	m_cDevs;
	//}}AFX_DATA

public:
    void ToggleYourself(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDevicesDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    int m_nCX0;
    int m_nCX1;
    int m_nCX2;

    BOOL m_bIsWin2K;
    CString m_sFilterName;
    DWORD m_dwUsbSnoopSize;
    PBYTE m_pUsbSnoop;
    CString m_szWinDir;
    
    BOOL HasValidSelection(void);
    BOOL GetSelectedHardwareID(CString& sHardwareID);

        
	// Generated message map functions
	//{{AFX_MSG(CDevicesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnViewRefresh();
	afx_msg void OnInstallSniffer();
	afx_msg void OnUninstallSniffer();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnRclickDevices(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUninstallService();
	afx_msg void OnInstallService();
	virtual void OnCancel();
	afx_msg void OnUnpackDrivers();
	afx_msg void OnRestartDevice();
	afx_msg void OnViewDevices();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnInstallRestart();
	afx_msg void OnUninstallRestart();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVICESDLG_H__BA191892_8E24_4FF6_9E55_537A748D1BA0__INCLUDED_)
