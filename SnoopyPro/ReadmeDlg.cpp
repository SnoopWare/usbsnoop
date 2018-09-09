// ReadmeDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "ReadmeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReadmeDlg dialog


CReadmeDlg::CReadmeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReadmeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReadmeDlg)
	//}}AFX_DATA_INIT
    m_fCourier.CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, 
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN,
        _T("Courier New"));
}

CReadmeDlg::~CReadmeDlg(void)
{
    m_fCourier.DeleteObject();
}

void CReadmeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReadmeDlg)
	DDX_Control(pDX, IDC_README, m_cReadme);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReadmeDlg, CDialog)
	//{{AFX_MSG_MAP(CReadmeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReadmeDlg message handlers

BOOL CReadmeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    m_cReadme.SetWindowText(_T("Couldn't find readme data..."));

	HRSRC hRsrc = FindResource(AfxGetResourceHandle(), _T("README"), _T("TEXT"));
    if(NULL != hRsrc)
    {
        HGLOBAL hGlobal = LoadResource(AfxGetResourceHandle(), hRsrc);
        if(NULL != hGlobal)
        {
            PVOID pReadme = LockResource(hGlobal);
			if (NULL != pReadme)
			{
				DWORD dwSize = SizeofResource(AfxGetResourceHandle(), hRsrc) / sizeof(TCHAR);
				CString readme((TCHAR*)pReadme, dwSize);
				m_cReadme.SetWindowText(readme.GetBuffer());
			}
        }
    }

    m_cReadme.SetFocus();
    m_cReadme.SetSel(-1, 0);

    m_cReadme.SetFont(&m_fCourier);

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
