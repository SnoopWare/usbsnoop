// USBLogDoc.h : interface of the CUSBLogDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_USBLOGDOC_H__549F75AA_C09D_4CED_90B6_946189F6925C__INCLUDED_)
#define AFX_USBLOGDOC_H__549F75AA_C09D_4CED_90B6_946189F6925C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "URB.h"

class CUSBLogDoc : public CDocument
{
protected: // create from serialization only
	CUSBLogDoc();
	DECLARE_DYNCREATE(CUSBLogDoc)

// Attributes
public:
    CArrayURB m_arURB;

    void SetTimeFormat(BOOL bRelative);
    BOOL IsTimeFormatRelative(void);
    void SetTimeStampZero(ULONG uTimeStampZero);

    void SetExpanded(int nURB, BOOL bIsExpanded);

    HANDLE m_hSniffer;
    ULONG m_uDeviceID;
    CString m_HardwareID;
    ULONG GetSnifferDevice(void) { return m_uDeviceID; }
    void AccessSniffer(ULONG uDeviceID, LPCWSTR sHardwareID);
    void StopAccessSniffer(void);
    BOOL GetNewURBSFromSniffer(void);
    void InsertURBs(LONG nNumOfURBs, PVOID data);
    int GetBufferFullnessEstimate(void);
    LPCTSTR GetHardwareID(void) { return m_HardwareID; }

    BOOL m_bSniffingEnabled;

    void OnPlayPause(void);
    void OnStop(void);

    BOOL IsSniffing(void);
    BOOL IsPaused(void);
    
    void AnalyzeLog(void);
    
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUSBLogDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUSBLogDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CUSBLogDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USBLOGDOC_H__549F75AA_C09D_4CED_90B6_946189F6925C__INCLUDED_)

//** end of USBLogDoc.h **************************************************
/*************************************************************************

  $Log: USBLogDoc.h,v $
  Revision 1.2  2002/10/05 01:10:43  rbosa
  Added the basic framework for exporting a log into an XML file. The
  output written is fairly poor. This checkin is mainly to get the
  framework in place and get feedback on it.

 * 
 * 4     2/22/02 6:12p Rbosa
 * - added some analyzing log functionality: find out endpoint address and
 * some directions of urbs

*************************************************************************/
