// ExportLogDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "SnoopyPro.h"
#include "ExportLogDlg.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////////
// The Export Format type relatation to string...

typedef struct EXPTYPE2STRING
{
    enum EXPORTER_TYPE ExpType;
    UINT nStringID;
} EXPTYPE2STRING;

static const EXPTYPE2STRING g_ExpType2String[] =
{
    { EXPTYPE_XML, IDS_EXPTYPE_XML }
};

const int g_nExpType2StringCount = sizeof(g_ExpType2String) / sizeof(g_ExpType2String[0]);

//////////////////////////////////////////////////////////////////////////
// CExportLogDlg dialog


CExportLogDlg::CExportLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportLogDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportLogDlg)
	m_bExportAll = FALSE;
	m_bExportRange = FALSE;
	m_bExportSelection = FALSE;
	m_sRange = _T("");
	//}}AFX_DATA_INIT
    m_ExporterType = EXPTYPE_XML;
}


void CExportLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportLogDlg)
	DDX_Control(pDX, IDC_RANGE, m_cRange);
	DDX_Control(pDX, IDC_EXPORT_SELECTION, m_cExportSelection);
	DDX_Control(pDX, IDC_EXPORT_RANGE, m_cExportRange);
	DDX_Control(pDX, IDC_EXPORT_ALL, m_cExportAll);
	DDX_Control(pDX, IDC_FORMAT, m_cFormat);
	DDX_Control(pDX, IDC_FILENAME, m_cFilename);
	DDX_Check(pDX, IDC_EXPORT_ALL, m_bExportAll);
	DDX_Check(pDX, IDC_EXPORT_RANGE, m_bExportRange);
	DDX_Check(pDX, IDC_EXPORT_SELECTION, m_bExportSelection);
	DDX_Text(pDX, IDC_RANGE, m_sRange);
	//}}AFX_DATA_MAP
    if(!pDX->m_bSaveAndValidate)
    {
        PathSetDlgItemPath(GetSafeHwnd(), IDC_FILENAME, m_sFilename);
    }
}


BEGIN_MESSAGE_MAP(CExportLogDlg, CDialog)
	//{{AFX_MSG_MAP(CExportLogDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_EXPORT_ALL, OnExportAll)
	ON_BN_CLICKED(IDC_EXPORT_RANGE, OnExportRange)
	ON_BN_CLICKED(IDC_EXPORT_SELECTION, OnExportSelection)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportLogDlg message handlers

BOOL CExportLogDlg::OnInitDialog(void)
{
	CDialog::OnInitDialog();

    m_ExporterType = (enum EXPORTER_TYPE) GetApp().GetProfileInt(REGSTR_EXPORTER, REGSTR_EXPORTERTYPE, EXPTYPE_XML);
    m_ExporterType = min(EXPTYPE_LAST, max(EXPTYPE_FIRST, m_ExporterType));

    CString sPrevExport = GetApp().GetProfileString(REGSTR_EXPORTER, REGSTR_FILENAME, "SnoopyProExport.log");
    m_cFilename.SetWindowText(sPrevExport);

    // for now, we only support exporting everything...
    m_cExportAll.SetCheck(BST_CHECKED);
    m_cExportAll.EnableWindow(FALSE);
    m_cExportRange.SetCheck(BST_UNCHECKED);
    m_cExportRange.EnableWindow(FALSE);
    m_cRange.EnableWindow(FALSE);
    m_cExportSelection.SetCheck(BST_UNCHECKED);
    m_cExportSelection.EnableWindow(FALSE);

    // populate the format combo drop list
    for(int nExpType = 0; nExpType < g_nExpType2StringCount; ++nExpType)
    {
        CString sFormat;
        sFormat.LoadString(g_ExpType2String[nExpType].nStringID);
        m_cFormat.AddString(sFormat);
        if(m_ExporterType == g_ExpType2String[nExpType].ExpType)
        {
            m_cFormat.SelectString(-1, sFormat);
        }
    }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExportLogDlg::OnDestroy(void)
{
    CString sFilename;
    m_cFilename.GetWindowText(sFilename);
    GetApp().WriteProfileString(REGSTR_EXPORTER, REGSTR_FILENAME, sFilename);

    // populate the format combo drop list
    CString sSelection;
    m_cFormat.GetWindowText(sSelection);
    for(int nExpType = 0; nExpType < g_nExpType2StringCount; ++nExpType)
    {
        CString sFormat;
        sFormat.LoadString(g_ExpType2String[nExpType].nStringID);
        if(sSelection == sFormat)
        {
            m_ExporterType = g_ExpType2String[nExpType].ExpType;
            break;
        }
    }
    
    CDialog::OnDestroy();
}

void CExportLogDlg::OnOK(void)
{
    m_cFilename.GetWindowText(m_sFilename);
    m_ExporterType = EXPTYPE_XML;
    CDialog::OnOK();
}
void CExportLogDlg::OnBrowse() 
{
    CFileDialog savedlg(FALSE, "xml", m_sFilename);
    if(IDOK == savedlg.DoModal())
    {
        m_sFilename = savedlg.GetPathName();
        UpdateData(FALSE);
    }
}

void CExportLogDlg::OnExportAll() 
{
	// TODO: Add your control notification handler code here
}

void CExportLogDlg::OnExportRange() 
{
	// TODO: Add your control notification handler code here
}

void CExportLogDlg::OnExportSelection() 
{
	// TODO: Add your control notification handler code here
}
