// USBLogDoc.cpp : implementation of the CUSBLogDoc class
//

#include "StdAfx.h"
#include "SnoopyPro.h"
#include "USBLogDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUSBLogDoc

IMPLEMENT_DYNCREATE(CUSBLogDoc, CDocument)

BEGIN_MESSAGE_MAP(CUSBLogDoc, CDocument)
//{{AFX_MSG_MAP(CUSBLogDoc)
// NOTE - the ClassWizard will add and remove mapping macros here.
//    DO NOT EDIT what you see in these blocks of generated code!
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUSBLogDoc construction/destruction

CUSBLogDoc::CUSBLogDoc()
{
    m_hSniffer = INVALID_HANDLE_VALUE;
    m_HardwareID.Empty();
}

CUSBLogDoc::~CUSBLogDoc()
{
    GetApp().RemoveDocumentFromIdleList(this);

    for(int nURB = 0; nURB < m_arURB.GetSize(); nURB++)
    {
        delete m_arURB[nURB];
    }
    m_arURB.RemoveAll();

    if(INVALID_HANDLE_VALUE != m_hSniffer)
    {
        CloseHandle(m_hSniffer);
        m_hSniffer = INVALID_HANDLE_VALUE;
    }
}

BOOL CUSBLogDoc::OnNewDocument()
{
    ASSERT(INVALID_HANDLE_VALUE == m_hSniffer);
    if(INVALID_HANDLE_VALUE != m_hSniffer)
    {
        CloseHandle(m_hSniffer);
        m_hSniffer = INVALID_HANDLE_VALUE;
    }

    if (!CDocument::OnNewDocument())
        return FALSE;

    ASSERT(0 == m_arURB.GetSize());

    m_hSniffer = INVALID_HANDLE_VALUE;
    m_HardwareID.Empty();
    m_bSniffingEnabled = TRUE;

    return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CUSBLogDoc serialization

void CUSBLogDoc::Serialize(CArchive& ar)
{
    m_arURB.Serialize(ar);

    if(ar.IsStoring())
    {
        ar.WriteCount(m_HardwareID.GetLength());
        ar.WriteString(m_HardwareID);
    }
    else
    {
        int nLength = ar.ReadCount();
        ar.ReadString(m_HardwareID.GetBuffer(nLength + 1), nLength + 1);
        m_HardwareID.ReleaseBuffer();
    }
}

/////////////////////////////////////////////////////////////////////////////
// CUSBLogDoc diagnostics

#ifdef _DEBUG
void CUSBLogDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CUSBLogDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CUSBLogDoc commands

void CUSBLogDoc::SetExpanded(int nURB, BOOL bIsExpanded)
{
    ASSERT(nURB < m_arURB.GetSize());
    ASSERT(NULL != m_arURB.GetAt(nURB));

    CURB *pURB = m_arURB.GetAt(nURB);
    pURB->SetExpanded(bIsExpanded);

    UpdateAllViews(NULL, nURB, pURB);
}

void CUSBLogDoc::SetTimeStampZero(ULONG uTimeStampZero)
{
    m_arURB.SetTimeStampZero(uTimeStampZero);
}

void CUSBLogDoc::SetTimeFormat(BOOL bRelative)
{
    m_arURB.SetTimeFormat(bRelative);
}

BOOL CUSBLogDoc::IsTimeFormatRelative(void)
{
    return m_arURB.IsTimeFormatRelative();
}

void CUSBLogDoc::AccessSniffer(ULONG uDeviceID, LPCWSTR sHardwareID)
{
    m_uDeviceID = uDeviceID;
    m_HardwareID = CString(sHardwareID);
    TRACE("Accessing sniffer...\n");
    m_hSniffer = CreateFile(USBSNPYS_W32NAME_2K,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, // no SECURITY_ATTRIBUTES structure
        OPEN_EXISTING, // No special create flags
        0, // No special attributes
        NULL);
    if(INVALID_HANDLE_VALUE == m_hSniffer)
    {
        m_hSniffer = CreateFile(USBSNPYS_W32NAME_9X,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, // no SECURITY_ATTRIBUTES structure
            OPEN_EXISTING,
            FILE_FLAG_DELETE_ON_CLOSE,
            NULL);
    }
    if(INVALID_HANDLE_VALUE != m_hSniffer)
    {
        TRACE("Got handle on it on %08x\n", m_hSniffer);
        GetApp().AddDocumentToIdleList(this);
    }
}

void CUSBLogDoc::StopAccessSniffer(void)
{
    if(INVALID_HANDLE_VALUE != m_hSniffer)
    {
        ENABLE_LOGGING EnableLogging;
        ZeroMemory(&EnableLogging, sizeof(EnableLogging));
        EnableLogging.uDeviceID = m_uDeviceID;
        EnableLogging.bEnable = FALSE;
        EnableLogging.bStopAndForget = TRUE;

        DWORD dwBytesReturned = 0;
        DeviceIoControl(m_hSniffer, USBSNOOP_ENABLE_LOGGING,
            &EnableLogging, sizeof(EnableLogging),
            &EnableLogging, sizeof(EnableLogging),
            &dwBytesReturned, NULL);

        CloseHandle(m_hSniffer);
        m_hSniffer = INVALID_HANDLE_VALUE;
        UpdateAllViews(NULL, 1);
    }
}

BOOL CUSBLogDoc::GetNewURBSFromSniffer(void)
{
    if(INVALID_HANDLE_VALUE != m_hSniffer)
    {
        ENABLE_LOGGING EnableLogging;
        ZeroMemory(&EnableLogging, sizeof(EnableLogging));
        EnableLogging.uDeviceID = m_uDeviceID;
        EnableLogging.bEnable = m_bSniffingEnabled;
        EnableLogging.bStopAndForget = FALSE;
        DWORD dwBytesReturned = 0;
        DeviceIoControl(m_hSniffer, USBSNOOP_ENABLE_LOGGING,
            &EnableLogging, sizeof(EnableLogging),
            &EnableLogging, sizeof(EnableLogging),
            &dwBytesReturned, NULL);

        const int BUFSIZE = 32768;
        BYTE data[BUFSIZE];
        PGET_URBS pParam = (PGET_URBS) data;
        pParam->nNumOfPackets = 1;
        pParam->uDeviceID = m_uDeviceID;
        pParam->nBufferSize = BUFSIZE - sizeof(GET_URBS);
        while(0 != pParam->nNumOfPackets)
        {
            BOOL bResult = DeviceIoControl(m_hSniffer, USBSNOOP_GET_URBS, 
                data, BUFSIZE,
                data, BUFSIZE, &dwBytesReturned, NULL);
            if(!bResult)
            {
                TRACE("Device 0x%08x: IOCTL Error ... closing...\n", GetSnifferDevice());
                StopAccessSniffer();
                break;
            }
            InsertURBs(pParam->nNumOfPackets, &pParam->phURBs[0]);
            if(GETURBS_FLAG_OVERFLOW_OCCURRED & pParam->nFlags)
            {
                TRACE("Device 0x%08x: Overflow occured!\n", GetSnifferDevice());
            }
        }

        if(GETURBS_FLAG_DEVICE_GONE & pParam->nFlags)
        {
            TRACE("Device 0x%08x is gone...\n", GetSnifferDevice());
            StopAccessSniffer();
        }
        UpdateAllViews(NULL);
        return TRUE;
    }
    return FALSE;
}

int CUSBLogDoc::GetBufferFullnessEstimate(void)
{
    if(INVALID_HANDLE_VALUE == m_hSniffer)
    {
        return 0;
    }

    GET_BUFFER_FULLNESS BF;
    ZeroMemory(&BF, sizeof(BF));
    BF.uDeviceID = m_uDeviceID;
    DWORD dwBytesReturned = 0;
    if(DeviceIoControl(m_hSniffer, USBSNOOP_GET_BUFFER_FULLNESS,
        &BF, sizeof(BF),
        &BF, sizeof(BF),
        &dwBytesReturned, NULL))
    {
        BF.nBufferTotalSize = max(100, BF.nBufferTotalSize);
        BF.nBufferUsedSize = max(1, min(BF.nBufferTotalSize, BF.nBufferUsedSize));
        return MulDiv(BF.nBufferUsedSize, 100, BF.nBufferTotalSize);
    }
    return 0;
}

void CUSBLogDoc::InsertURBs(LONG nNumOfURBs, PVOID data)
{
    for(int i = 0; i < nNumOfURBs; i++)
    {
        PPACKET_HEADER ph = (PPACKET_HEADER) data;
        m_arURB.UnserializeURB(ph);
        data = ((PUCHAR) data) + ph->uLen;
    }
}

void CUSBLogDoc::OnPlayPause(void)
{
    m_bSniffingEnabled = !m_bSniffingEnabled;
    UpdateAllViews(NULL);
}

void CUSBLogDoc::OnStop(void)
{
    StopAccessSniffer();
}

BOOL CUSBLogDoc::IsSniffing(void)
{
    return (INVALID_HANDLE_VALUE != m_hSniffer);
}

BOOL CUSBLogDoc::IsPaused(void)
{
    return !m_bSniffingEnabled;
}

void CUSBLogDoc::AnalyzeLog(void)
{
    // find the first select configuration URB
    int nURB = 0;
    CMap<DWORD, DWORD, UCHAR, UCHAR> mapPH2EP;
    while(nURB < m_arURB.GetSize())
    {
        CURB *pURB = m_arURB.GetAt(nURB);
        if((URB_FUNCTION_SELECT_CONFIGURATION == pURB->GetFunction()) && pURB->IsComingUp())
        {
            CURB_SelectConfiguration *pSelectConfig = (CURB_SelectConfiguration *) pURB;
            DWORD dwInterfaceCnt = pSelectConfig->GetInterfaceCountFromConfiguration();
            for(DWORD dwInterface = 0; dwInterface < dwInterfaceCnt; ++dwInterface)
            {
                DWORD dwPipeCnt = pSelectConfig->GetPipeCountFromConfiguration(dwInterface);
                for(DWORD dwPipe = 0; dwPipe < dwPipeCnt; ++ dwPipe)
                {
                    DWORD dwPipeHandle = pSelectConfig->GetPipeHandleFromConfiguration(dwInterface, dwPipe);
                    UCHAR cEndpoint = pSelectConfig->GetEndpointFromConfiguration(dwInterface, dwPipe);
                    TRACE("Added relation: 0x%08x -> 0x%02x\n", dwPipeHandle, cEndpoint);
                    mapPH2EP.SetAt(dwPipeHandle, cEndpoint);
                }
            }
        }
        else if((URB_FUNCTION_SELECT_INTERFACE == pURB->GetFunction()) && pURB->IsComingUp())
        {
            CURB_SelectInterface *pSelectInterface = (CURB_SelectInterface *) pURB;
            DWORD dwPipeCnt = pSelectInterface->GetPipeCountFromInterface();
            for(DWORD dwPipe = 0; dwPipe < dwPipeCnt; ++dwPipe)
            {
                DWORD dwPipeHandle = pSelectInterface->GetPipeHandleFromInterface(dwPipe);
                UCHAR cEndpoint = pSelectInterface->GetEndpointFromInterface(dwPipe);
                TRACE("Added relation: 0x%08x -> 0x%02x\n", dwPipeHandle, cEndpoint);
                mapPH2EP.SetAt(dwPipeHandle, cEndpoint);
            }
        }
        else
        {
            if(pURB->IsPipeHandleAvailable())
            {
                DWORD dwPipeHandle = pURB->GetPipeHandle();
                if(0 == dwPipeHandle)
                {
                    pURB->SetEndpoint(0);
                }
                else
                {
                    UCHAR cEndpoint = 0;
                    TRACE("Setting relation: 0x%08x ->", dwPipeHandle);
                    if(mapPH2EP.Lookup(dwPipeHandle, cEndpoint))
                    {
                        TRACE(" 0x%02x\n", cEndpoint);
                        pURB->SetEndpoint(cEndpoint);
                        pURB->SetDirection(USB_ENDPOINT_DIRECTION_IN(cEndpoint), USB_ENDPOINT_DIRECTION_OUT(cEndpoint));
                    }
                    else
                    {
                        TRACE(" not found\n");
                    }
                }
            }
        }

        ++nURB;
    }

    nURB = 0;
    while(nURB < m_arURB.GetSize())
    {
        CURB *pURB = m_arURB.GetAt(nURB);
        if(!pURB->IsDirectionKnown())
        {
            int nOtherUrb = m_arURB.FindBySequenceAndDirection(pURB->GetSequenceNr(), !pURB->IsComingUp());
            if(-1 != nOtherUrb)
            {
                CURB *pOtherURB = m_arURB.GetAt(nOtherUrb);
                if(pOtherURB->IsDirectionKnown())
                {
                    pURB->SetDirection(pOtherURB->IsDirectionIn(), pOtherURB->IsDirectionOut());
                }
            }
        }
        ++nURB;
    }

    UpdateAllViews(NULL, 1);
}

//** end of CUSBLogDoc.cpp ***********************************************
/*************************************************************************

  $Log: not supported by cvs2svn $
 * 
 * 4     2/22/02 6:12p Rbosa
 * - added some analyzing log functionality: find out endpoint address and
 * some directions of urbs

*************************************************************************/
