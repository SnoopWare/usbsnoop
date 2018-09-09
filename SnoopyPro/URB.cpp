//************************************************************************
//
// URB.cpp
//
// implementation of the CURB class.
//
//************************************************************************

#include "StdAfx.h"
#include "URB.h"
#include "SnoopyPro.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////////
// CChunkAllocator

CChunkAllocator::CChunkAllocator(void)
{
    m_CurrentChunk = NULL;
    m_nCurrentOffset = 0;
    VERIFY(m_LineMemory.IsEmpty());    
}

CChunkAllocator::~CChunkAllocator(void)
{
    while(!m_LineMemory.IsEmpty())
    {
        LPTSTR sChunk = m_LineMemory.RemoveHead();
        delete [] sChunk;
    }
}

const int CChunkAllocator::LINEMEMORY_CHUNKSIZE = (1 << 20);

LPCTSTR CChunkAllocator::AllocLine(LPCTSTR sLine, int nLen)
{
    if(NULL == sLine)
    {
        TRACE("CChunkAllocator::AllocLine(NULL)!\n");
        return NULL;
    }

    if((nLen + sizeof(TCHAR)) > LINEMEMORY_CHUNKSIZE)
    {
        TRACE("CChunkAllocator::AllocLine(..., %d): size too big!\n", nLen);
        return NULL;
    }

    if(NULL == m_CurrentChunk)
    {
        // chunk exhausted... get a new one...
        m_CurrentChunk = new TCHAR [LINEMEMORY_CHUNKSIZE];
        m_LineMemory.AddTail(m_CurrentChunk);
        m_nCurrentOffset = 0;
    }

    if(m_nCurrentOffset + nLen + sizeof(TCHAR) < LINEMEMORY_CHUNKSIZE)
    {
        LPTSTR sResult = &m_CurrentChunk[m_nCurrentOffset];
        m_nCurrentOffset += nLen + sizeof(TCHAR);
        memcpy(sResult, sLine, nLen + sizeof(TCHAR));
        return sResult;
    }

    // chunk exhausted... get a new one...
    m_CurrentChunk = NULL;
    return AllocLine(sLine, nLen);
}

PVOID CChunkAllocator::AllocBlock(int nLen)
{
    if(nLen > LINEMEMORY_CHUNKSIZE)
    {
        TRACE("CChunkAllocator::AllocBlock(%d): size too big!\n", nLen);
        return NULL;
    }
    
    if(NULL == m_CurrentChunk)
    {
        // chunk exhausted... get a new one...
        m_CurrentChunk = new TCHAR [LINEMEMORY_CHUNKSIZE];
        m_LineMemory.AddTail(m_CurrentChunk);
        m_nCurrentOffset = 0;
    }
    
    if(m_nCurrentOffset + nLen < LINEMEMORY_CHUNKSIZE)
    {
        PVOID sResult = &m_CurrentChunk[m_nCurrentOffset];
        m_nCurrentOffset += nLen;
        return sResult;
    }
    
    // chunk exhausted... get a new one...
    m_CurrentChunk = NULL;
    return AllocBlock(nLen);    
}

//////////////////////////////////////////////////////////////////////////
// attributes of urbs
//////////////////////////////////////////////////////////////////////////

#define IS_SELECT		0x00000001
#define IS_CONTROL      0x00000002
#define IS_BULK         0x00000004
#define IS_DIR_IN       0x00000008
#define IS_DIR_OUT      0x00000010
#define URBFCN(x, s, f)   \
        {\
            sString[x] = s;\
			bIsSelect[x] = (0 != (f & IS_SELECT));\
            bIsControl[x] = (0 != (f & IS_CONTROL));\
            bIsBulk[x] = (0 != (f & IS_BULK)); \
            bIsDirIn[x] = (0 != (f & IS_DIR_IN)); \
            bIsDirOut[x] = (0 != (f & IS_DIR_OUT)); \
        }

class InitURBFCN
{
public:
    InitURBFCN()
    {
        URBFCN(URB_FUNCTION_SELECT_CONFIGURATION, _T("SELECT_CONFIGURATION"), IS_SELECT | IS_DIR_IN | IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SELECT_INTERFACE, _T("SELECT_INTERFACE"), IS_SELECT | IS_DIR_IN | IS_DIR_OUT);
        URBFCN(URB_FUNCTION_ABORT_PIPE, _T("ABORT_PIPE"), 0);
        URBFCN(URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL, _T("TAKE_FRAME_LENGTH_CONTROL"), 0);
        URBFCN(URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL, _T("RELEASE_FRAME_LENGTH_CONTROL"), 0);
        URBFCN(URB_FUNCTION_GET_FRAME_LENGTH, _T("GET_FRAME_LENGTH"), IS_DIR_IN);
        URBFCN(URB_FUNCTION_SET_FRAME_LENGTH, _T("SET_FRAME_LENGTH"), IS_DIR_OUT);
        URBFCN(URB_FUNCTION_GET_CURRENT_FRAME_NUMBER, _T("GET_CURRENT_FRAME_NUMBER"), 0);
        URBFCN(URB_FUNCTION_CONTROL_TRANSFER, _T("CONTROL_TRANSFER"), IS_CONTROL);
        URBFCN(URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER, _T("BULK_OR_INTERRUPT_TRANSFER"), IS_BULK);
        URBFCN(URB_FUNCTION_ISOCH_TRANSFER, _T("ISOCH_TRANSFER"), 0);
        URBFCN(URB_FUNCTION_RESET_PIPE, _T("RESET_PIPE"), 0);
        URBFCN(URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE, _T("GET_DESCRIPTOR_FROM_DEVICE"), IS_DIR_IN);
        URBFCN(URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT, _T("GET_DESCRIPTOR_FROM_ENDPOINT"), IS_DIR_IN);
        URBFCN(URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE, _T("GET_DESCRIPTOR_FROM_INTERFACE"), IS_DIR_IN);
        URBFCN(URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE, _T("SET_DESCRIPTOR_TO_DEVICE"), IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT, _T("SET_DESCRIPTOR_TO_ENDPOINT"), IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE, _T("SET_DESCRIPTOR_TO_INTERFACE"), IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SET_FEATURE_TO_DEVICE, _T("SET_FEATURE_TO_DEVICE"), IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SET_FEATURE_TO_INTERFACE, _T("SET_FEATURE_TO_INTERFACE"), IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SET_FEATURE_TO_ENDPOINT, _T("SET_FEATURE_TO_ENDPOINT"), IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SET_FEATURE_TO_OTHER, _T("SET_FEATURE_TO_OTHER"), IS_DIR_OUT);
        URBFCN(URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE, _T("CLEAR_FEATURE_TO_DEVICE"), IS_DIR_OUT);
        URBFCN(URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE, _T("CLEAR_FEATURE_TO_INTERFACE"), IS_DIR_OUT);
        URBFCN(URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT, _T("CLEAR_FEATURE_TO_ENDPOINT"), IS_DIR_OUT);
        URBFCN(URB_FUNCTION_CLEAR_FEATURE_TO_OTHER, _T("CLEAR_FEATURE_TO_OTHER"), IS_DIR_OUT);
        URBFCN(URB_FUNCTION_GET_STATUS_FROM_DEVICE, _T("GET_STATUS_FROM_DEVICE"), IS_DIR_IN);
        URBFCN(URB_FUNCTION_GET_STATUS_FROM_INTERFACE, _T("GET_STATUS_FROM_INTERFACE"), IS_DIR_IN);
        URBFCN(URB_FUNCTION_GET_STATUS_FROM_ENDPOINT, _T("GET_STATUS_FROM_ENDPOINT"), IS_DIR_IN);
        URBFCN(URB_FUNCTION_GET_STATUS_FROM_OTHER, _T("GET_STATUS_FROM_OTHER"), IS_DIR_IN);
        URBFCN(URB_FUNCTION_RESERVED0, _T("RESERVED0"), 0);
        URBFCN(URB_FUNCTION_VENDOR_DEVICE, _T("VENDOR_DEVICE"), 0);
        URBFCN(URB_FUNCTION_VENDOR_INTERFACE, _T("VENDOR_INTERFACE"), 0);
        URBFCN(URB_FUNCTION_VENDOR_ENDPOINT, _T("VENDOR_ENDPOINT"), 0);
        URBFCN(URB_FUNCTION_VENDOR_OTHER, _T("VENDOR_OTHER"), 0);
        URBFCN(URB_FUNCTION_CLASS_DEVICE, _T("CLASS_DEVICE"), 0);
        URBFCN(URB_FUNCTION_CLASS_INTERFACE, _T("CLASS_INTERFACE"), 0);
        URBFCN(URB_FUNCTION_CLASS_ENDPOINT, _T("CLASS_ENDPOINT"), 0);
        URBFCN(URB_FUNCTION_CLASS_OTHER, _T("CLASS_OTHER"), 0);
        URBFCN(URB_FUNCTION_RESERVED, _T("RESERVED"), 0);
        URBFCN(URB_FUNCTION_GET_CONFIGURATION, _T("GET_CONFIGURATION"), IS_DIR_IN);
        URBFCN(URB_FUNCTION_GET_INTERFACE, _T("GET_INTERFACE"), IS_DIR_IN);
    }

    LPCTSTR sString[URB_FUNCTION_LAST];
	BOOL bIsSelect[URB_FUNCTION_LAST];
    BOOL bIsControl[URB_FUNCTION_LAST];
    BOOL bIsBulk[URB_FUNCTION_LAST];
    BOOL bIsDirIn[URB_FUNCTION_LAST];
    BOOL bIsDirOut[URB_FUNCTION_LAST];

} g_URBFCN;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CURB::CURB(void)
{
    m_pCA = NULL;

	m_dwSequence = 0;
    m_cEndpoint = 0;
    m_dwPipeHandle = 0;
    m_dwFlags = 0;
    m_dwTime = 0;
    m_nFunction = 0;
    m_lStatus = 0;
    m_dwLink = 0;

    m_bExpanded = FALSE;
    m_nExpandedSize = -1;
}

CURB::~CURB()
{
    // we don't own the chunk allocator...
    m_pCA = NULL;
}

IMPLEMENT_SERIAL(CURB, CObject, 1)

void CURB::Serialize(CArchive &ar)
{
    CObject::Serialize(ar);

    if(ar.IsStoring())
    {
        ar << m_dwSequence;
        ar << m_nFunction;
        ar << m_dwTime;
        ar.WriteCount(m_cEndpoint);
        ar << m_dwPipeHandle;
        ar << m_dwFlags;
        ar << m_lStatus;
        ar << m_dwLink;
        ar.Write(&m_PH, sizeof(m_PH));
        ar << m_bExpanded;
    }
    else
    {
        ar >> m_dwSequence;
        ar >> m_nFunction;
        ar >> m_dwTime;
        m_cEndpoint = (UCHAR) ar.ReadCount();
        ar >> m_dwPipeHandle;
        ar >> m_dwFlags;
        ar >> m_lStatus;
        ar >> m_dwLink;
        ar.Read(&m_PH, sizeof(m_PH));
        ar >> m_bExpanded;

        // this will retrigger the rendering...
        m_nExpandedSize = -1;
    }
}

//////////////////////////////////////////////////////////////////////////
// access methods
//////////////////////////////////////////////////////////////////////////

void CURB::GetSequenceNo(LPTSTR sBuffer)
{
    _stprintf(sBuffer, _T("%d"), m_dwSequence);
}

DWORD CURB::GetSequenceNr(void)
{
    return m_dwSequence;
}

int CURB::GetDirection(void)
{
    int nResult = m_dwFlags;
    nResult &= ~(URBFLAGS_DIRECTION_IN | URBFLAGS_DIRECTION_OUT | URBFLAGS_COMING_UP);
    return nResult;
}

void CURB::GetDirectionStr(LPTSTR sBuffer)
{
    if(IsDirectionIn() && IsDirectionOut())
    {
        _tcscpy(sBuffer, _T("inout"));
    }
    else if(IsDirectionIn())
    {
        _tcscpy(sBuffer, _T("in"));
    }
    else if(IsDirectionOut())
    {
        _tcscpy(sBuffer, _T("out"));
    }
    else
    {
        _tcscpy(sBuffer, _T("???"));
    }

    if(IsComingUp())
    {
        _tcscat(sBuffer, _T(" up"));
    }
    else
    {
        _tcscat(sBuffer, _T(" down"));
    }
}

int CURB::GetEndpoint(void)
{
    return (URBFLAGS_ENDPOINTKNOWN & m_dwFlags) ? m_cEndpoint : -1;
}

void CURB::GetEndpointStr(LPTSTR sBuffer)
{
    if(URBFLAGS_ENDPOINTKNOWN & m_dwFlags)
    {
        _stprintf(sBuffer, _T("0x%02x"), m_cEndpoint);
    }
    else
    {
        _tcscpy(sBuffer, _T("n/a"));
    }
}

DWORD CURB::GetTime(CArrayURB *parURB)
{
    ASSERT(NULL != parURB);
    if(NULL != parURB)
    {
        if(parURB->IsTimeFormatRelative())
        {
            return m_dwTime - parURB->GetTimeStampZero();
        }
    }
    return m_dwTime;
}

USHORT CURB::GetFunction(void)
{
    return m_nFunction;
}

LPCTSTR CURB::GetFunctionStr(void)
{
    return g_URBFCN.sString[m_nFunction];
}

LONG CURB::GetStatus(void)
{
    return m_lStatus;
}

DWORD CURB::GetLinkNo(void)
{
    return m_dwLink;
}

void CURB::GetDataDumpStr(LPTSTR sBuffer)
{
    _tcscpy(sBuffer, _T(""));
}

int CURB::GetPacketCount(void)
{
    return 1;
}

int CURB::GetPayloadCount(int nPacket)
{
    ASSERT((-1 == nPacket) || (0 == nPacket));
    return 0;
}

PUCHAR CURB::GetPayload(int nPacket)
{
    ASSERT((-1 == nPacket) || (0 == nPacket));
    return NULL;
}

LPCTSTR CURB::GetPayloadXML(int nPacket, LPTSTR sBuffer)
{
    ASSERT((-1 == nPacket) || (0 == nPacket));
    _tcscpy(sBuffer, _T(""));
    return sBuffer;
}

void CURB::SetChunkAllocator(CChunkAllocator *pChunkAllocator)
{
    m_pCA = pChunkAllocator;
}

BOOL CURB::IsExpanded(void)
{
    return m_bExpanded;
}

void CURB::SetExpanded(BOOL bExpanded)
{
    m_bExpanded = bExpanded;
}

LPCTSTR CURB::GetExpandedLine(int nLine)
{
    // call this to trigger the cache generation...
    GetExpandSize();

    ASSERT(nLine < GetExpandSize());
    
    // return m_arRendered.GetAt(nLine);
    return (LPCTSTR) m_arRenderedLines[nLine];
}

// Actually renders the expanded lines as well as returning the size ??
int CURB::GetExpandSize(void)
{
    if(-1 == m_nExpandedSize)
    {
        // prepare the expanded view
        m_arRenderedLines.RemoveAll();
        ASSERT(NULL != m_pCA);
        RenderProperties();
	    AddPropLine(_T("  "));
        m_nExpandedSize = m_arRenderedLines.GetSize();
    }

    ASSERT(0 <= m_nExpandedSize);
    return m_nExpandedSize;
}

void CURB::AddPropLine(LPCTSTR sFormat, ...)
{
    if(MAX_LINES_PER_URB == m_arRenderedLines.GetSize())
    {
        m_arRenderedLines.Add(m_pCA->AllocLine(_T("..."), 3));
    }
    if(MAX_LINES_PER_URB < m_arRenderedLines.GetSize())
    {
        return;
    }
    static TCHAR sBuffer[2048];
    ASSERT(NULL != m_pCA);
    va_list params;
    va_start(params, sFormat);
    _vstprintf(sBuffer, sFormat, params);
    va_end(params);
    m_arRenderedLines.Add(m_pCA->AllocLine(sBuffer, _tcslen(sBuffer)));
}

void CURB::RenderProperties(void)
{
    AddPropLine(_T("  URB Header (length: %d)"), m_PH.UrbHeader.Length);
    AddPropLine(_T("  SequenceNumber: %d"), m_dwSequence);
    AddPropLine(_T("  Function: %04x (%s)"), m_nFunction, GetFunctionStr());
}

//////////////////////////////////////////////////////////////////////////
// filter methods
//////////////////////////////////////////////////////////////////////////

BOOL CURB::IsSelectURB(void)
{
	return g_URBFCN.bIsSelect[m_nFunction];
}

BOOL CURB::IsControlURB(void)
{
    return g_URBFCN.bIsControl[m_nFunction];
}

BOOL CURB::IsBulkURB(void)
{
    return g_URBFCN.bIsBulk[m_nFunction];
}

BOOL CURB::HasNoPayload(void)
{
    return FALSE;
}

BOOL CURB::IsDirectionIn(void)
{
    return (0 != (URBFLAGS_DIRECTION_IN & m_dwFlags));
}

BOOL CURB::IsDirectionOut(void)
{
    return (0 != (URBFLAGS_DIRECTION_OUT & m_dwFlags));
}

BOOL CURB::IsDirectionKnown(void)
{
    return (0 != ((URBFLAGS_DIRECTION_IN | URBFLAGS_DIRECTION_OUT) & m_dwFlags));
}

void CURB::SetDirection(BOOL bIsIn, BOOL bIsOut)
{
    m_dwFlags &= ~(URBFLAGS_DIRECTION_IN | URBFLAGS_DIRECTION_OUT);
    if(bIsIn)
    {
        m_dwFlags |= URBFLAGS_DIRECTION_IN;
    }
    if(bIsOut)
    {
        m_dwFlags |= URBFLAGS_DIRECTION_OUT;
    }
}

BOOL CURB::IsComingUp(void)
{
    return (DIRECTION_FROM_DEVICE == m_PH.bDirection);
}

BOOL CURB::IsPipeHandleAvailable(void)
{
    return (0 != (URBFLAGS_PIPEHANDLE_PRESENT & m_dwFlags));
}

void PokeHexNumber(LPTSTR &sBuffer, ULONG uNumber, int nDigits)
{
    static TCHAR s_HexDigit[] = _T("0123456789abcdef");

    while(0 < nDigits)
    {
        int nNibble = (uNumber >> (4 * (nDigits - 1))) & 0x0f;
        *sBuffer++ = s_HexDigit[nNibble];
        nDigits--;
    }
}

void CURB::GenerateHexDumpLine(LPTSTR pOutput, PVOID pBuffer, int nSize)
{
    ASSERT(NULL != pOutput);

    pOutput[0] = 0x00;
    PUCHAR pData = (PUCHAR) pBuffer;
    if(NULL != pData)
    {
        int nAddress = 0;
        if(nSize > 0)
        {
            while(nSize > 0)
            {
                PokeHexNumber(pOutput, pData[nAddress], 2);
                *pOutput++ = ' ';
                nAddress++;
                nSize--;
            }
            --pOutput;
            *pOutput = 0x00;
        }
    }
}

void CURB::GenerateHexDump(PVOID pBuffer, int nSize)
{
    PUCHAR pData = (PUCHAR) pBuffer;
    if(NULL == pData)
        return;
    
    // each line consists of:
    //  2 bytes for indentation
    //  4 bytes for address
    //  2 bytes for ': '
    // 3n bytes for the hexdump ('%02x ')
    //  1 byte for 0x00 terminator
    // --
    // 7 + 3n bytes per line (with n==8 bytes per line)
    
    const int nBytesPerLine = 16;
    TCHAR sLine[2 + 7 + 3 * nBytesPerLine];
    
    int nAddress = 0;
    while(nSize > 0)
    {
        LPTSTR sBuffer = sLine;
        *sBuffer++ = ' ';
        *sBuffer++ = ' ';
        PokeHexNumber(sBuffer, nAddress, 4);
        *sBuffer++ = ':';
        *sBuffer++ = ' ';
        for(int nByte = 0; nByte < min(nBytesPerLine, max(0, nSize)); ++nByte)
        {
            PokeHexNumber(sBuffer, pData[nAddress + nByte], 2);
            *sBuffer++ = ' ';
        }
        *sBuffer++ = 0x00;
        AddPropLine(sLine);
        
        nAddress += nBytesPerLine;
        nSize -= nBytesPerLine;
    }
}

void CURB::GrabData(PPACKET_HEADER ph)
{
    m_dwSequence = ph->uSequenceNumber;
    m_dwTime = ph->uTimeStamp;
    m_nFunction = ph->UrbHeader.Function;
    m_lStatus = ph->UrbHeader.Status;
    CopyMemory(&m_PH, ph, sizeof(m_PH));
    
    SetDirection(g_URBFCN.bIsDirIn[m_nFunction], g_URBFCN.bIsDirOut[m_nFunction]);
}

void CURB::GrabBytes(PUCHAR &pData, PVOID pBuffer, ULONG uSize)
{
    if(0 != uSize)
    {
        CopyMemory(pBuffer, pData, uSize);
        pData += uSize;
    }
}

DWORD CURB::GetPipeHandle(void)
{
    return m_dwPipeHandle;
}

void CURB::SetEndpoint(UCHAR cEndpoint)
{
    m_cEndpoint = cEndpoint;
    m_dwFlags |= URBFLAGS_ENDPOINTKNOWN;
}

void CURB::SetPipeHandle(PVOID PipeHandle)
{
    m_dwPipeHandle = (DWORD) PipeHandle;
    m_dwFlags |= URBFLAGS_PIPEHANDLE_PRESENT;
}

//////////////////////////////////////////////////////////////////////////
// CURB_TransferBuffer

CURB_TransferBuffer::CURB_TransferBuffer(void)
{
    m_TransferLength = 0;
    m_TransferBuffer = NULL;
}

CURB_TransferBuffer::~CURB_TransferBuffer(void)
{
    if(NULL != m_TransferBuffer)
    {
        delete [] m_TransferBuffer;
        m_TransferBuffer = NULL;
    }
}

IMPLEMENT_SERIAL(CURB_TransferBuffer, CURB, 1);

void CURB_TransferBuffer::Serialize(CArchive &ar)
{
    CURB::Serialize(ar);

    if(ar.IsStoring())
    {
        ar << m_TransferLength;
        if(0 != m_TransferLength)
        {
            ar.WriteCount((NULL != m_TransferBuffer) ? 1 : 0);
            if(NULL != m_TransferBuffer)
            {
                ar.Write(m_TransferBuffer, m_TransferLength);
            }
        }
    }
    else
    {
        ar >> m_TransferLength;
        ASSERT(NULL == m_TransferBuffer);
        if(0 != m_TransferLength)
        {
            if(0 != ar.ReadCount())
            {
                m_TransferBuffer = new BYTE [m_TransferLength];
                ar.Read(m_TransferBuffer, m_TransferLength);
            }
        }
    }
}

int CURB_TransferBuffer::GetPayloadCount(int nPacket)
{
    ASSERT((-1 == nPacket) || (0 == nPacket));
    return m_TransferLength;
}

PUCHAR CURB_TransferBuffer::GetPayload(int nPacket)
{
    ASSERT((-1 == nPacket) || (0 == nPacket));
    return m_TransferBuffer;
}

LPCTSTR CURB_TransferBuffer::GetPayloadXML(int nPacket, LPTSTR sBuffer)
{
    ASSERT((-1 == nPacket) || (0 == nPacket));
    if(NULL == GetPayload(nPacket))
    {
        return CURB::GetPayloadXML(nPacket, sBuffer);
    }
    
    for(int nIndex = 0; nIndex < m_TransferLength; ++nIndex)
    {
#ifdef UNICODE
		swprintf(&sBuffer[nIndex << 1], _T("%02x"), m_TransferBuffer[nIndex]);
#else
		sprintf(&sBuffer[nIndex << 1], _T("%02x"), m_TransferBuffer[nIndex]);
#endif
    }
    return sBuffer;
}

void CURB_TransferBuffer::GetDataDumpStr(LPTSTR sBuffer)
{
    if(m_TransferBuffer)
    {
        GenerateHexDumpLine(sBuffer, m_TransferBuffer, min(8, m_TransferLength));
    }
    else
    {
        _tcscpy(sBuffer, _T("-"));
    }
}

BOOL CURB_TransferBuffer::HasNoPayload(void)
{
    return (NULL == m_TransferBuffer);
}

void CURB_TransferBuffer::RenderProperties(void)
{
    if(m_TransferBuffer)
    {
        AddPropLine(_T(""));
        AddPropLine(_T("  TransferBuffer: 0x%08x (%d) length"), m_TransferLength, m_TransferLength);
        GenerateHexDump(m_TransferBuffer, m_TransferLength);
    }
    else
    {
        AddPropLine(_T(""));
        AddPropLine(_T("  No TransferBuffer"));
    }
}

void CURB_TransferBuffer::GrabTransferBuffer(PUCHAR pData, int nLength)
{
    m_TransferLength = nLength;
    if(0 != *pData++)
    {
        if(0 != *pData++)
        {
            ULONG uSize = *((PULONG)pData);
            pData += sizeof(uSize);
// ~~99
TRACE("CURB_TransferBuffer::GrabTransferBuffer assert uSize = %d == m_TransferLength = %d\n",uSize,m_TransferLength);
            ASSERT((int) uSize == m_TransferLength);
         
            ASSERT(NULL == m_TransferBuffer);
            m_TransferBuffer = new BYTE [m_TransferLength];
            CopyMemory(m_TransferBuffer, pData, m_TransferLength);
        }
    }
    
    //TRACE("  transfer length: %d (0x%08x)\n", m_TransferLength, m_TransferLength);
}

//////////////////////////////////////////////////////////////////////////
// CURB_SelectConfiguration

CURB_SelectConfiguration::CURB_SelectConfiguration(void)
{
    ZeroMemory(&m_SelectConfiguration, sizeof(m_SelectConfiguration));
	m_UCDPresent = FALSE;
    ZeroMemory(&m_UCD, sizeof(m_UCD));
    m_uNumInterfaces = 0;
    for(int nInterface = 0; nInterface < MAX_INTERFACES; ++nInterface)
    {
        m_pInterfaces[nInterface] = NULL;
    }
}

CURB_SelectConfiguration::~CURB_SelectConfiguration(void)
{
    for(int nInterface = 0; nInterface < MAX_INTERFACES; ++nInterface)
    {
        if(NULL != m_pInterfaces[nInterface])
        {
            delete m_pInterfaces[nInterface];
            m_pInterfaces[nInterface] = NULL;
        }
    }
}

IMPLEMENT_SERIAL(CURB_SelectConfiguration, CURB, 1)

void CURB_SelectConfiguration::Serialize(CArchive &ar)
{
    CURB::Serialize(ar);

    if(ar.IsStoring())
    {
        ar.Write(&m_SelectConfiguration, sizeof(m_SelectConfiguration));
        ar << m_UCDPresent;
        ar.Write(&m_UCD, sizeof(m_UCD));
        ar << m_uNumInterfaces;
        if(0 != m_uNumInterfaces)
        {
            ULONG nInterface = 0;
            while(nInterface < m_uNumInterfaces)
            {
                ar.WriteCount(m_pInterfaces[nInterface]->Length);
                ar.Write(m_pInterfaces[nInterface], m_pInterfaces[nInterface]->Length);
                ++nInterface;
            }
            while(nInterface < MAX_INTERFACES)
            {
                ASSERT(NULL == m_pInterfaces[nInterface]);
                ++nInterface;
            }
        }
    }
    else
    {
        ar.Read(&m_SelectConfiguration, sizeof(m_SelectConfiguration));
        ar >> m_UCDPresent;
        ar.Read(&m_UCD, sizeof(m_UCD));
        ar >> m_uNumInterfaces;
        if(0 != m_uNumInterfaces)
        {
            ULONG nInterface = 0;
            while(nInterface < m_uNumInterfaces)
            {
                ULONG nInterfaceLen = ar.ReadCount();
                ASSERT(NULL == m_pInterfaces[nInterface]);
                m_pInterfaces[nInterface] = (PUSBD_INTERFACE_INFORMATION) new BYTE [nInterfaceLen];
                ar.Read(m_pInterfaces[nInterface], nInterfaceLen);
                ++nInterface;
            }
            while(nInterface < MAX_INTERFACES)
            {
                ASSERT(NULL == m_pInterfaces[nInterface]);
                ++nInterface;
            }
        }
    }
}

void CURB_SelectConfiguration::RenderProperties(void)
{
	CURB::RenderProperties();

	if(m_UCDPresent)
	{
		AddPropLine(_T("  Configuration Descriptor:"));

        CString S;
        AddPropLine(_T("  bLength: %d (0x%02x)"), m_UCD.bLength, m_UCD.bLength);
        AddPropLine(_T("  bDescriptorType: %d (0x%02x)"), m_UCD.bDescriptorType, m_UCD.bDescriptorType);
        if(m_UCD.bDescriptorType != 0x02)
        {
            AddPropLine(_T("    (should be 0x02!)"));
        }
        AddPropLine(_T("  wTotalLength: %d (0x%04x)"), m_UCD.wTotalLength, m_UCD.wTotalLength);
        AddPropLine(_T("  bNumInterfaces: %d (0x%02x)"), m_UCD.bNumInterfaces, m_UCD.bNumInterfaces);
        AddPropLine(_T("  bConfigurationValue: %d (0x%02x)"), m_UCD.bConfigurationValue, m_UCD.bConfigurationValue);
        AddPropLine(_T("  iConfiguration: %d (0x%02x)"), m_UCD.iConfiguration, m_UCD.iConfiguration);
        AddPropLine(_T("  bmAttributes: %d (0x%02x)"), m_UCD.bmAttributes, m_UCD.bmAttributes);
        if(0 != (0x80 & m_UCD.bmAttributes))
        {
            AddPropLine(_T("    0x80: Bus Powered"));
        }
        if(0 != (0x40 & m_UCD.bmAttributes))
        {
            AddPropLine(_T("    0x40: Self Powered"));
        }
        if(0 != (0x20 & m_UCD.bmAttributes))
        {
            AddPropLine(_T("    0x20: Remote Wakeup"));
        }
        AddPropLine(_T("  MaxPower: %d (0x%02x)"), m_UCD.MaxPower, m_UCD.MaxPower);
        AddPropLine(_T("    (in 2 mA units, therefore %d mA power consumption)"), m_UCD.MaxPower * 2);

        AddPropLine(_T("  "));
        AddPropLine(_T("  Number of interfaces: %d"), m_uNumInterfaces);
        
        for(ULONG i = 0; i < m_uNumInterfaces; i++)
        {
            AddPropLine(_T("  Interface[%d]:"), i);
            AddPropLine(_T("    Length: 0x%04x"), m_pInterfaces[i]->Length);
            if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, InterfaceNumber))
            {
                AddPropLine(_T("    InterfaceNumber: 0x%02x"), m_pInterfaces[i]->InterfaceNumber);
                if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, AlternateSetting))
                {
                    AddPropLine(_T("    AlternateSetting: 0x%02x"), m_pInterfaces[i]->AlternateSetting);
                    if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, Class))
                    {
                        AddPropLine(_T("    Class             = 0x%02x"), m_pInterfaces[i]->Class);
                        if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, SubClass))
                        {
                            AddPropLine(_T("    SubClass          = 0x%02x"), m_pInterfaces[i]->SubClass);
                            if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, Protocol))
                            {
                                AddPropLine(_T("    Protocol          = 0x%02x"), m_pInterfaces[i]->Protocol);
                                if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, InterfaceHandle))
                                {
                                    AddPropLine(_T("    InterfaceHandle   = 0x%08x"), m_pInterfaces[i]->InterfaceHandle);
                                    if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, NumberOfPipes))
                                    {
                                        AddPropLine(_T("    NumberOfPipes     = 0x%08x"), m_pInterfaces[i]->NumberOfPipes);
                                        ULONG uNumPipes = m_pInterfaces[i]->NumberOfPipes;
                                        if(uNumPipes > 0x1f)
                                        {
                                            AddPropLine(_T("  ERROR: uNumPipes is too large (%d), resetting to 1"), uNumPipes);
                                            uNumPipes = 1;
                                        }
                                        
                                        for(ULONG p = 0; p < uNumPipes; p++)
                                        {
                                            if(m_pInterfaces[i]->Length >= (FIELD_OFFSET(USBD_INTERFACE_INFORMATION, Pipes) + (p + 1) * sizeof(USBD_PIPE_INFORMATION)))
                                            {
                                                PUSBD_PIPE_INFORMATION pPipe = &m_pInterfaces[i]->Pipes[p];
                                                AddPropLine(_T("    Pipe[%d]:"), p);
                                                AddPropLine(_T("      MaximumPacketSize = 0x%04x"), pPipe->MaximumPacketSize);
                                                AddPropLine(_T("      EndpointAddress   = 0x%02x"), pPipe->EndpointAddress);
                                                AddPropLine(_T("      Interval          = 0x%02x"), pPipe->Interval);
                                                AddPropLine(_T("      PipeType          = 0x%02x"), pPipe->PipeType);
                                                if(pPipe->PipeType == UsbdPipeTypeControl)
                                                    AddPropLine(_T("        UsbdPipeTypeControl"));
                                                else if(pPipe->PipeType == UsbdPipeTypeIsochronous)
                                                    AddPropLine(_T("        UsbdPipeTypeIsochronous"));
                                                else if(pPipe->PipeType == UsbdPipeTypeBulk)
                                                    AddPropLine(_T("        UsbdPipeTypeBulk"));
                                                else if(pPipe->PipeType == UsbdPipeTypeInterrupt)
                                                    AddPropLine(_T("        UsbdPipeTypeInterrupt"));
                                                else
                                                    AddPropLine(_T("        !!! INVALID !!!"));
                                                AddPropLine(_T("      PipeHandle        = 0x%08x"), pPipe->PipeHandle);
                                                AddPropLine(_T("      MaxTransferSize   = 0x%08x"), pPipe->MaximumTransferSize);
                                                AddPropLine(_T("      PipeFlags         = 0x%02x"), pPipe->PipeFlags);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        AddPropLine(_T("  no Configuration Descriptor present => unconfigure device!"));
    }
}

#define FIELD_DIFF(low, high) ((UCHAR *)(&(high)) - (UCHAR *)(&(low)))

#ifndef NEVER
void CURB_SelectConfiguration::GrabData(PPACKET_HEADER ph)
{
	CURB::GrabData(ph);

	PUCHAR pData = (PUCHAR)(&ph->UrbHeader);
    _URB_SELECT_CONFIGURATION *pSC = (_URB_SELECT_CONFIGURATION *)(&ph->UrbHeader);
	if (g_bIsWow64) {
TRACE("~1 SelectConfig: Hdr.Length before = %d\n",pSC->Hdr.Length);
		// Yuk! We're getting a 64 bit struct, and we expect a 32 bit one!
		// Copy it section by section between pointers/handles.
		_URB_SELECT_CONFIGURATION *mpSC = pSC;

		// First part of header including bottom 32 bits of Device Handle
	    CopyMemory(&m_SelectConfiguration,
		            mpSC,
		            FIELD_OFFSET(_URB_SELECT_CONFIGURATION, Hdr.UsbdFlags));
		mpSC = (_URB_SELECT_CONFIGURATION *) ((UCHAR *)mpSC + 4);	// Skip top 32 bits of handle
		m_SelectConfiguration.Hdr.Length -= 4;
TRACE("~1 SelectConfig: Hdr.Length after Dev Handle = %d\n",m_SelectConfiguration.Hdr.Length);

		// Balance of header, UsbdFlags
		m_SelectConfiguration.Hdr.UsbdFlags = mpSC->Hdr.UsbdFlags;

		// Align to 64 bit boundary
		mpSC = (_URB_SELECT_CONFIGURATION *) ((UCHAR *)mpSC + 4);
		m_SelectConfiguration.Hdr.Length -= 4;
TRACE("~1 SelectConfig: Hdr.Length after Align = %d\n",m_SelectConfiguration.Hdr.Length);

		// bottom 32 bits of ConfigurationDescriptor pointer
		m_SelectConfiguration.ConfigurationDescriptor = mpSC->ConfigurationDescriptor;
		mpSC = (_URB_SELECT_CONFIGURATION *) ((UCHAR *)mpSC + 4);	// Top 32 bits of pointer
		PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor2 = mpSC->ConfigurationDescriptor;
		m_SelectConfiguration.Hdr.Length -= 4;
TRACE("~1 SelectConfig: Hdr.Length after ConfigDesc pointer = %d\n",m_SelectConfiguration.Hdr.Length);

	    if(m_SelectConfiguration.Hdr.Length >= FIELD_OFFSET(_URB_SELECT_CONFIGURATION, ConfigurationHandle)) {
TRACE("~1 got interface info\n");
			// Grab the number of interfaces
            m_uNumInterfaces = 0;
	        m_UCDPresent = (   NULL != m_SelectConfiguration.ConfigurationDescriptor
	                        || NULL != ConfigurationDescriptor2);
	        if(m_UCDPresent) {
	            CopyMemory(&m_UCD, pData + pSC->Hdr.Length, sizeof(m_UCD));
				m_uNumInterfaces = m_UCD.bNumInterfaces;
TRACE("~1 UCDPresent, NumInterfaces %d\n",m_uNumInterfaces);
			}

			// ConfigurationHandle
			m_SelectConfiguration.ConfigurationHandle = mpSC->ConfigurationHandle;
			mpSC = (_URB_SELECT_CONFIGURATION *) ((UCHAR *)mpSC + 4);	// Skip top 32 bits handle
			m_SelectConfiguration.Hdr.Length -= 4;
TRACE("~1 SelectConfig: Hdr.Length after ConfigHandle = %d\n",m_SelectConfiguration.Hdr.Length);

            PUSBD_INTERFACE_INFORMATION pSCIF = &mpSC->Interface;	// Initial source

            ULONG nInterface = 0;
            while(nInterface < m_uNumInterfaces) {
	            PUSBD_INTERFACE_INFORMATION mpSCIF = pSCIF;	// Movable version of source

				// (pSCIF->Length will be longer than needed)
                m_pInterfaces[nInterface] = (PUSBD_INTERFACE_INFORMATION) new BYTE [pSCIF->Length];
	            PUSBD_INTERFACE_INFORMATION pInterface = m_pInterfaces[nInterface];	// Dest
TRACE("~1 Interface %d, initial length %d\n",nInterface,mpSCIF->Length);

				// From Length including bottom 32 bits of InterfaceHandle
			    CopyMemory(&pInterface->Length,
				           &mpSCIF->Length,
				           FIELD_DIFF(pInterface->Length,
				                      pInterface->NumberOfPipes));
				mpSCIF = (USBD_INTERFACE_INFORMATION *) ((UCHAR *)mpSCIF + 4);	// Skip top 32 
				m_SelectConfiguration.Hdr.Length -= 4;
				pInterface->Length -= 4;
TRACE("~1 SelectConfig: Hdr.Length after InterfaceHandle = %d\n",m_SelectConfiguration.Hdr.Length);

				// The number of pipes
				pInterface->NumberOfPipes = mpSCIF->NumberOfPipes;
TRACE("~1 Interface %d, number of pipes %d\n",nInterface,pInterface->NumberOfPipes);

				// Align to 64 bit boundary, because USBD_PIPE_INFORMATION contains pointer
				mpSCIF = (USBD_INTERFACE_INFORMATION *) ((UCHAR *)mpSCIF + 4);
				m_SelectConfiguration.Hdr.Length -= 4;
				pInterface->Length -= 4;
TRACE("~1 SelectConfig: Hdr.Length after Align = %d\n",m_SelectConfiguration.Hdr.Length);

				// Copy each of the pipe structures */
				ULONG pix;
				for (pix = 0; pix < pInterface->NumberOfPipes; pix++) {

					// MaximumPacketSize including lower 32 bits of PipeHandle
				    CopyMemory(&pInterface->Pipes[pix].MaximumPacketSize,
					           &mpSCIF->Pipes[pix].MaximumPacketSize,
					           FIELD_DIFF(pInterface->Pipes[pix].MaximumPacketSize,
					                      pInterface->Pipes[pix].MaximumTransferSize));
					mpSCIF = (USBD_INTERFACE_INFORMATION *) ((UCHAR *)mpSCIF + 4);	// Skip top 32
					pInterface->Length -= 4;
					m_SelectConfiguration.Hdr.Length -= 4;
TRACE("~1 SelectConfig: Hdr.Length after InterfaceHandle = %d\n",m_SelectConfiguration.Hdr.Length);
		
					// MaximumTransferSize and PipeFlags
				    CopyMemory(&pInterface->Pipes[pix].MaximumTransferSize,
					           &mpSCIF->Pipes[pix].MaximumTransferSize,
					           FIELD_DIFF(pInterface->Pipes[pix].MaximumTransferSize,
					                      pInterface->Pipes[pix+1]));
TRACE("~1 Interface %d, after pipe %d, length %d\n",nInterface,pix,pInterface->Length);
				}
TRACE("~1 Interface %d, initial length %d\n",nInterface,pInterface->Length);
                pSCIF = (PUSBD_INTERFACE_INFORMATION)(((PUCHAR)pSCIF) + pSCIF->Length);
				nInterface++;
			}
		}
TRACE("~1 SelectConfig: Hdr.Length after = %d\n",m_SelectConfiguration.Hdr.Length);
	} else {
TRACE("~1 SelectConfig: Hdr.Length = %d\n",pSC->Hdr.Length);
	    CopyMemory(&m_SelectConfiguration, pSC, sizeof(m_SelectConfiguration));

	    if(m_SelectConfiguration.Hdr.Length >= FIELD_OFFSET(_URB_SELECT_CONFIGURATION, ConfigurationHandle))
	    {
TRACE("~1 got interface info\n");
	        m_UCDPresent = (NULL != m_SelectConfiguration.ConfigurationDescriptor);
	        if(m_UCDPresent)	// USBD_INTERFACE_INFORMATION is after _URB_SELECT_CONFIGURATION
	        {
	            CopyMemory(&m_UCD, pData + m_SelectConfiguration.Hdr.Length, sizeof(m_UCD));
	            m_uNumInterfaces = m_UCD.bNumInterfaces;
	            ULONG nInterface = 0;
	            PUSBD_INTERFACE_INFORMATION pInterface = &pSC->Interface;
TRACE("~1 UCDPresent, NumInterfaces %d\n",m_uNumInterfaces);
	            while(nInterface < m_uNumInterfaces)
	            {
	                m_pInterfaces[nInterface] = (PUSBD_INTERFACE_INFORMATION) new BYTE [pInterface->Length];
	                CopyMemory(m_pInterfaces[nInterface], pInterface, pInterface->Length);
	                pInterface = (PUSBD_INTERFACE_INFORMATION)(((PUCHAR)pInterface) + pInterface->Length);
	                ++nInterface;
	            }
	        }
	    }
	}
}
#else
// Old code
void CURB_SelectConfiguration::GrabData(PPACKET_HEADER ph)
{
	CURB::GrabData(ph);

	PUCHAR pData = (PUCHAR)(&ph->UrbHeader);
    _URB_SELECT_CONFIGURATION *pSC = (_URB_SELECT_CONFIGURATION *)(&ph->UrbHeader);
    CopyMemory(&m_SelectConfiguration, pSC, sizeof(m_SelectConfiguration));
    if(m_SelectConfiguration.Hdr.Length >= FIELD_OFFSET(_URB_SELECT_CONFIGURATION, ConfigurationHandle))
    {
        m_UCDPresent = (NULL != m_SelectConfiguration.ConfigurationDescriptor);
        if(m_UCDPresent)
        {
            CopyMemory(&m_UCD, pData + m_SelectConfiguration.Hdr.Length, sizeof(m_UCD));
            m_uNumInterfaces = m_UCD.bNumInterfaces;
            ULONG nInterface = 0;
            PUSBD_INTERFACE_INFORMATION pInterface = &pSC->Interface;
            while(nInterface < m_uNumInterfaces)
            {
                m_pInterfaces[nInterface] = (PUSBD_INTERFACE_INFORMATION) new BYTE [pInterface->Length];
                CopyMemory(m_pInterfaces[nInterface], pInterface, pInterface->Length);
                pInterface = (PUSBD_INTERFACE_INFORMATION)(((PUCHAR)pInterface) + pInterface->Length);
                ++nInterface;
            }
        }
    }
}
#endif

DWORD CURB_SelectConfiguration::GetInterfaceCountFromConfiguration(void)
{
    ASSERT(IsComingUp());
    return m_uNumInterfaces;
}

DWORD CURB_SelectConfiguration::GetPipeCountFromConfiguration(DWORD dwInterface)
{
    ASSERT(dwInterface < GetInterfaceCountFromConfiguration());
    ASSERT(NULL != m_pInterfaces[dwInterface]);
    return m_pInterfaces[dwInterface]->NumberOfPipes;
}

DWORD CURB_SelectConfiguration::GetPipeHandleFromConfiguration(DWORD dwInterface, DWORD dwPipe)
{
    ASSERT(dwInterface < GetInterfaceCountFromConfiguration());
    ASSERT(dwPipe < GetPipeCountFromConfiguration(dwInterface));
    return (DWORD)(m_pInterfaces[dwInterface]->Pipes[dwPipe].PipeHandle);
}

UCHAR CURB_SelectConfiguration::GetEndpointFromConfiguration(DWORD dwInterface, DWORD dwPipe)
{
    ASSERT(dwInterface < GetInterfaceCountFromConfiguration());
    ASSERT(dwPipe < GetPipeCountFromConfiguration(dwInterface));
    return m_pInterfaces[dwInterface]->Pipes[dwPipe].EndpointAddress;
}

//////////////////////////////////////////////////////////////////////////
// CURB_SelectInterface

CURB_SelectInterface::CURB_SelectInterface(void)
{
    m_pSelectInterface = NULL;
    m_dwSelectInterfaceSize = 0;
}

CURB_SelectInterface::~CURB_SelectInterface(void)
{
    m_dwSelectInterfaceSize = 0;
    m_pSelectInterface = NULL;
}

IMPLEMENT_SERIAL(CURB_SelectInterface, CURB, 1);

void CURB_SelectInterface::Serialize(CArchive &ar)
{
    CURB::Serialize(ar);

    if(ar.IsStoring())
    {
        ar.WriteCount(m_dwSelectInterfaceSize);
        ar.Write(m_pSelectInterface, m_dwSelectInterfaceSize);
    }
    else
    {
        m_dwSelectInterfaceSize = ar.ReadCount();
        m_pSelectInterface = (_URB_SELECT_INTERFACE *) m_pCA->AllocBlock(m_dwSelectInterfaceSize);
        ar.Read(m_pSelectInterface, m_dwSelectInterfaceSize);
    }
}

void CURB_SelectInterface::RenderProperties(void)
{
    CURB::RenderProperties();

    AddPropLine(_T("  ConfigurationHandle: 0x%08x (%d)"), 
        m_pSelectInterface->ConfigurationHandle,
        m_pSelectInterface->ConfigurationHandle);

    AddPropLine(_T(""));
    AddPropLine(_T("  Interface:"));
    PUSBD_INTERFACE_INFORMATION pInterface = &m_pSelectInterface->Interface;
    AddPropLine(_T("    Length: 0x%04x (%d)"), 
        pInterface->Length,
        pInterface->Length);
    AddPropLine(_T("    InterfaceNumber: 0x%02x (%d)"),
        pInterface->InterfaceNumber,
        pInterface->InterfaceNumber);
    AddPropLine(_T("    AlternateSetting: 0x%02x (%d)"),
        pInterface->AlternateSetting,
        pInterface->AlternateSetting);

    if(!IsComingUp())
    {
        return;
    }

    AddPropLine(_T("  "));
    AddPropLine(_T("  Output"));
    AddPropLine(_T("    Class: 0x%02x (%d)"),
        pInterface->Class,
        pInterface->Class);
    AddPropLine(_T("    SubClass: 0x%02x (%d)"),
        pInterface->SubClass,
        pInterface->SubClass);
    AddPropLine(_T("    Protocol: 0x%02x (%d)"),
        pInterface->Protocol,
        pInterface->Protocol);
    AddPropLine(_T("    Reserved: 0x%02x"),
        pInterface->Reserved);

    AddPropLine(_T("    InterfaceHandle: 0x%08x"),
        pInterface->InterfaceHandle);
    AddPropLine(_T("    NumberOfPipes: 0x%08x (%d)"),
        pInterface->NumberOfPipes,
        pInterface->NumberOfPipes);

    PUSBD_PIPE_INFORMATION pPipe = pInterface->Pipes;
    int nBufferLeft = m_dwSelectInterfaceSize - FIELD_OFFSET(_URB_SELECT_INTERFACE, Interface.Pipes[0]);
    ULONG nPipe = 0;
    while((nPipe < pInterface->NumberOfPipes) && (0 < nBufferLeft))
    {
        AddPropLine(_T("    Pipe[%d]:"), nPipe);
        pPipe = &pInterface->Pipes[nPipe];

        // OUTPUT from USBD
        AddPropLine(_T("      MaximumPacketSize: 0x%04x (%d)"),
            pPipe->MaximumPacketSize,
            pPipe->MaximumPacketSize);
        AddPropLine(_T("      EndpointAddress: 0x%02x (%d)"),
            pPipe->EndpointAddress,
            pPipe->EndpointAddress);
        AddPropLine(_T("      Interval: %d ms"),
            pPipe->Interval);
        AddPropLine(_T("      PipeType: 0x%02x"),
            pPipe->PipeType);
        switch(pPipe->PipeType)
        {
        case UsbdPipeTypeControl:
            AddPropLine(_T("        UsbdPipeTypeControl"));
            break;
        case UsbdPipeTypeIsochronous:
            AddPropLine(_T("        UsbdPipeTypeIsochronous"));
            break;
        case UsbdPipeTypeBulk:
            AddPropLine(_T("        UsbdPipeTypeBulk"));
            break;
        case UsbdPipeTypeInterrupt:
            AddPropLine(_T("        UsbdPipeTypeInterrupt"));
            break;
        default:
            AddPropLine(_T("        invalid!"));
        }
        AddPropLine(_T("      PipeHandle: 0x%08x"),
            pPipe->PipeHandle);

        // INPUT
        // These fields are filled in by the client driver
        AddPropLine(_T("  "));
        AddPropLine(_T("      MaximumTransferSize: 0x%08x (%d) bytes"),
            pPipe->MaximumTransferSize,
            pPipe->MaximumTransferSize);
        AddPropLine(_T("      PipeFlags: 0x%08x (%d)"),
            pPipe->PipeFlags,
            pPipe->PipeFlags);

        ++nPipe;
        nBufferLeft -= sizeof(USBD_PIPE_INFORMATION);
    }
}

void CURB_SelectInterface::GrabData(PPACKET_HEADER ph)
{
    CURB::GrabData(ph);

    PUCHAR pData = (PUCHAR)(&ph->UrbHeader);
    m_dwSelectInterfaceSize = max(ph->UrbHeader.Length, sizeof(_URB_SELECT_INTERFACE));
    m_pSelectInterface = (_URB_SELECT_INTERFACE*) m_pCA->AllocBlock(m_dwSelectInterfaceSize);
	if (g_bIsWow64) {
		// Yuk! We're getting a 64 bit struct, and we expect a 32 bit one!
		// Copy it section by section between pointers/handles.
		_URB_SELECT_INTERFACE *mpSI = (_URB_SELECT_INTERFACE *)&ph->UrbHeader;
TRACE("~1 SelectInterf: Hdr.Length before = %d\n",mpSI->Hdr.Length);

		// First part of header including Device Handle
	    CopyMemory(m_pSelectInterface,
		            mpSI,
		            FIELD_OFFSET(_URB_SELECT_INTERFACE, Hdr.UsbdFlags));
		mpSI = (_URB_SELECT_INTERFACE *) ((UCHAR *)mpSI + 4);	// Skip top 32 bits of pointer
		m_pSelectInterface->Hdr.Length -= 4;

		// Balance of header, UsbdFlags
		m_pSelectInterface->Hdr.UsbdFlags = mpSI->Hdr.UsbdFlags;

		// Align to 64 bit boundary
		mpSI = (_URB_SELECT_INTERFACE *) ((UCHAR *)mpSI + 4);
		m_pSelectInterface->Hdr.Length -= 4;

		// bottom 32 bits of ConfigurationHandle pointer
		m_pSelectInterface->ConfigurationHandle = mpSI->ConfigurationHandle;
		mpSI = (_URB_SELECT_INTERFACE *) ((UCHAR *)mpSI + 4);	// Skip top 32 bits of pointer
		m_pSelectInterface->Hdr.Length -= 4;

		// From Interface to Interface.InterfaceHandle
	    CopyMemory(&m_pSelectInterface->Interface,
		           &mpSI->Interface,
		           FIELD_DIFF(m_pSelectInterface->Interface,
		                      m_pSelectInterface->Interface.NumberOfPipes));
		mpSI = (_URB_SELECT_INTERFACE *) ((UCHAR *)mpSI + 4);	// Skip top 32 bits of pointer
		m_pSelectInterface->Hdr.Length -= 4;
		m_pSelectInterface->Interface.Length -= 4;

		// The number of pipes
		m_pSelectInterface->Interface.NumberOfPipes = mpSI->Interface.NumberOfPipes;

		// Align to 64 bit boundary, because USBD_PIPE_INFORMATION contains pointer
		mpSI = (_URB_SELECT_INTERFACE *) ((UCHAR *)mpSI + 4);
		m_pSelectInterface->Hdr.Length -= 4;
		m_pSelectInterface->Interface.Length -= 4;

		// Copy each of the pipe structures */
		ULONG pix;
		for (pix = 0; pix < mpSI->Interface.NumberOfPipes; pix++) {

			// From start of Pipe including bottom 32 bits of PipeHandle
		    CopyMemory(&m_pSelectInterface->Interface.Pipes[pix],
			           &mpSI->Interface.Pipes[pix],
			           FIELD_DIFF(m_pSelectInterface->Interface.Pipes[pix],
			                      m_pSelectInterface->Interface.Pipes[pix].MaximumTransferSize));
			mpSI = (_URB_SELECT_INTERFACE *) ((UCHAR *)mpSI + 4);	// Skip top 32 bits of pointer
			m_pSelectInterface->Hdr.Length -= 4;
			m_pSelectInterface->Interface.Length -= 4;

			// From MaximumTransferSize to end of Pipe
		    CopyMemory(&m_pSelectInterface->Interface.Pipes[pix].MaximumTransferSize,
			           &mpSI->Interface.Pipes[pix].MaximumTransferSize,
			           FIELD_DIFF(m_pSelectInterface->Interface.Pipes[pix].MaximumTransferSize,
			                      m_pSelectInterface->Interface.Pipes[pix+1]));
		}
TRACE("~1 SelectInterf: Hdr.Length after = %d\n",m_pSelectInterface->Hdr.Length);
	} else {
	    CopyMemory(m_pSelectInterface, pData, m_dwSelectInterfaceSize);
	}
}

DWORD CURB_SelectInterface::GetPipeCountFromInterface(void)
{
    ASSERT(IsComingUp());
    ASSERT(NULL != m_pSelectInterface);
    return m_pSelectInterface->Interface.NumberOfPipes;
}

DWORD CURB_SelectInterface::GetPipeHandleFromInterface(DWORD dwPipe)
{
    ASSERT(dwPipe < GetPipeCountFromInterface());
    return (DWORD) (m_pSelectInterface->Interface.Pipes[dwPipe].PipeHandle);
}

UCHAR CURB_SelectInterface::GetEndpointFromInterface(DWORD dwPipe)
{
    ASSERT(dwPipe < GetPipeCountFromInterface());
    return m_pSelectInterface->Interface.Pipes[dwPipe].EndpointAddress;
}


//////////////////////////////////////////////////////////////////////////
// CURB_ControlTransfer

CURB_ControlTransfer::CURB_ControlTransfer(void)
{
    ZeroMemory(&m_ControlTransfer, sizeof(m_ControlTransfer));
}

IMPLEMENT_SERIAL(CURB_ControlTransfer, CURB_TransferBuffer, 1)

void CURB_ControlTransfer::Serialize(CArchive &ar)
{
    CURB_TransferBuffer::Serialize(ar);

    if(ar.IsStoring())
    {
        ar.Write(&m_ControlTransfer, sizeof(m_ControlTransfer));
    }
    else
    {
        ar.Read(&m_ControlTransfer, sizeof(m_ControlTransfer));
    }
}

void CURB_ControlTransfer::RenderProperties(void)
{
    CURB::RenderProperties();

    AddPropLine(_T("  PipeHandle: %08x"), m_ControlTransfer.PipeHandle);
    
    AddPropLine(_T("  "));
    AddPropLine(_T("  SetupPacket:"));
    GenerateHexDump(m_ControlTransfer.SetupPacket, sizeof(m_ControlTransfer.SetupPacket));

    UCHAR bmRequestType = m_ControlTransfer.SetupPacket[0];
    UCHAR bRequest = m_ControlTransfer.SetupPacket[1];
    USHORT wValue = (USHORT)(m_ControlTransfer.SetupPacket[3] + 256 * m_ControlTransfer.SetupPacket[2]);
    //USHORT wIndex = (USHORT)(m_ControlTransfer.SetupPacket[5] + 256 * m_ControlTransfer.SetupPacket[4]);
    //USHORT wLength = (USHORT)(m_ControlTransfer.SetupPacket[7] + 256 * m_ControlTransfer.SetupPacket[6]);

    AddPropLine(_T("  bmRequestType: %02x"), bmRequestType);
    AddPropLine(_T("    DIR: %s"), USB_ENDPOINT_DIRECTION_IN(bmRequestType) ?
        _T("Device-To-Host") : _T("Host-To-Device"));

    BOOL bIsTypeStandard = FALSE;
    switch((bmRequestType & 0x60) >> 5)
    {
    case 0:
        AddPropLine(_T("    TYPE: Standard"));
        bIsTypeStandard = TRUE;
        break;
    case 1:
        AddPropLine(_T("    TYPE: Class"));
        break;
    case 2:
        AddPropLine(_T("    TYPE: Vendor"));
        break;
    case 3:
        AddPropLine(_T("    TYPE: Reserved"));
        break;
    default:
        ASSERT(FALSE);
        break;
    }

    BOOL bIsRecipientDevice = FALSE;
    switch(bmRequestType & 0x1f)
    {
    case 0:
        AddPropLine(_T("    RECIPIENT: Device"));
        bIsRecipientDevice = TRUE;
        break;
    case 1:
        AddPropLine(_T("    RECIPIENT: Interface"));
        break;
    case 2:
        AddPropLine(_T("    RECIPIENT: Endpoint"));
        break;
    case 3:
        AddPropLine(_T("    RECIPIENT: Other"));
        break;
    default:
        AddPropLine(_T("    RECIPIENT: %d => 4..31 == Reserved"), bmRequestType & 0x1f);
        break;
    }

    AddPropLine(_T("  bRequest: %02x  "), bRequest);
    
    BOOL bIsReqGetDescriptor = FALSE;
    if(bIsTypeStandard)
    {
        switch(bRequest)
        {
        case USB_REQUEST_GET_STATUS:
            AddPropLine(_T("    GET_STATUS"));
            break;
        case USB_REQUEST_CLEAR_FEATURE:
            AddPropLine(_T("    CLEAR_FEATURE"));
            break;
        case 2:
        case 4:
            AddPropLine(_T("    reserved for future use!!"));
            break;
        case USB_REQUEST_SET_FEATURE:
            AddPropLine(_T("    SET_FEATURE"));
            break;
        case USB_REQUEST_SET_ADDRESS:
            AddPropLine(_T("    SET_ADDRESS"));
            break;
        case USB_REQUEST_GET_DESCRIPTOR:
            AddPropLine(_T("    GET_DESCRIPTOR"));
            bIsReqGetDescriptor = TRUE;
            break;
        case USB_REQUEST_SET_DESCRIPTOR:
            AddPropLine(_T("    SET_DESCRIPTOR"));
            break;
        case USB_REQUEST_GET_CONFIGURATION:
            AddPropLine(_T("    GET_CONFIGURATION"));
            break;
        case USB_REQUEST_SET_CONFIGURATION:
            AddPropLine(_T("    SET_CONFIGURATION"));
            break;
        case USB_REQUEST_GET_INTERFACE:
            AddPropLine(_T("    GET_INTERFACE"));
            break;
        case USB_REQUEST_SET_INTERFACE:
            AddPropLine(_T("    SET_INTERFACE"));
            break;
        case USB_REQUEST_SYNC_FRAME:
            AddPropLine(_T("    SYNCH_FRAME"));
            break;
        default:
            AddPropLine(_T("    unknown!"));
            break;
        }
    }
    
    BOOL bIsDeviceDescriptorType = FALSE;
    BOOL bIsConfigurationDescriptorType = FALSE;
    if(bIsReqGetDescriptor)
    {
        AddPropLine(_T("  Descriptor Type: 0x%04x"), wValue);
        switch(wValue)
        {
        case USB_DEVICE_DESCRIPTOR_TYPE:
            AddPropLine(_T("    DEVICE"));
            bIsDeviceDescriptorType = TRUE;
            break;
            
        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
            AddPropLine(_T("    CONFIGURATION"));
            bIsConfigurationDescriptorType = TRUE;
            break;
            
        case USB_STRING_DESCRIPTOR_TYPE:
            AddPropLine(_T("    STRING"));
            break;
            
        case USB_INTERFACE_DESCRIPTOR_TYPE:
            AddPropLine(_T("    INTERFACE"));
            break;
            
        case USB_ENDPOINT_DESCRIPTOR_TYPE:
            AddPropLine(_T("    ENDPOINT"));
            break;
            
        default:
            AddPropLine(_T("    unknown"));
            break;
        }
    }
    
    CURB_TransferBuffer::RenderProperties();

    if(m_TransferBuffer)
    {
        if(bIsDeviceDescriptorType)
        {
            USB_DEVICE_DESCRIPTOR *pDesc = (USB_DEVICE_DESCRIPTOR*) m_TransferBuffer;
            AddPropLine(_T("      bLength            : 0x%02x (%d)"), pDesc->bLength, pDesc->bLength);
            AddPropLine(_T("      bDescriptorType    : 0x%02x (%d)"), pDesc->bDescriptorType, pDesc->bDescriptorType);
            AddPropLine(_T("      bcdUSB             : 0x%04x (%d)"), pDesc->bcdUSB, pDesc->bcdUSB);
            AddPropLine(_T("      bDeviceClass       : 0x%02x (%d)"), pDesc->bDeviceClass, pDesc->bDeviceClass);
            AddPropLine(_T("      bDeviceSubClass    : 0x%02x (%d)"), pDesc->bDeviceSubClass, pDesc->bDeviceSubClass);
            AddPropLine(_T("      bDeviceProtocol    : 0x%02x (%d)"), pDesc->bDeviceProtocol, pDesc->bDeviceProtocol);
            AddPropLine(_T("      bMaxPacketSize0    : 0x%02x (%d)"), pDesc->bMaxPacketSize0, pDesc->bMaxPacketSize0);
            AddPropLine(_T("      idVendor           : 0x%04x (%d)"), pDesc->idVendor, pDesc->idVendor);
            AddPropLine(_T("      idProduct          : 0x%04x (%d)"), pDesc->idProduct, pDesc->idProduct);
            AddPropLine(_T("      bcdDevice          : 0x%04x (%d)"), pDesc->bcdDevice, pDesc->bcdDevice);
            AddPropLine(_T("      iManufacturer      : 0x%02x (%d)"), pDesc->iManufacturer, pDesc->iManufacturer);
            AddPropLine(_T("      iProduct           : 0x%02x (%d)"), pDesc->iProduct, pDesc->iProduct);
            AddPropLine(_T("      iSerialNumber      : 0x%02x (%d)"), pDesc->iSerialNumber, pDesc->iSerialNumber);
            AddPropLine(_T("      bNumConfigurations : 0x%02x (%d)"), pDesc->bNumConfigurations, pDesc->bNumConfigurations);
        }
        
        if(bIsConfigurationDescriptorType)
        {
            USB_CONFIGURATION_DESCRIPTOR *pDesc = (USB_CONFIGURATION_DESCRIPTOR*) m_TransferBuffer;
            AddPropLine(_T("      bLength            : 0x%02x (%d)"), pDesc->bLength, pDesc->bLength);
            AddPropLine(_T("      bDescriptorType    : 0x%02x (%d)"), pDesc->bDescriptorType, pDesc->bDescriptorType);
            AddPropLine(_T("      wTotalLength       : 0x%04x (%d)"), pDesc->wTotalLength, pDesc->wTotalLength);
            AddPropLine(_T("      bNumInterfaces     : 0x%02x (%d)"), pDesc->bNumInterfaces, pDesc->bNumInterfaces);
            AddPropLine(_T("      bConfigurationValue: 0x%02x (%d)"), pDesc->bConfigurationValue, pDesc->bConfigurationValue);
            AddPropLine(_T("      iConfiguration     : 0x%02x (%d)"), pDesc->iConfiguration, pDesc->iConfiguration);
            AddPropLine(_T("      bmAttributes       : 0x%02x (%d)"), pDesc->bmAttributes, pDesc->bmAttributes);
            AddPropLine(_T("      MaxPower           : 0x%02x (%d)"), pDesc->MaxPower, pDesc->MaxPower);
        }
    }
}

void CURB_ControlTransfer::GrabData(PPACKET_HEADER ph)
{
    CURB::GrabData(ph);
    
	PUCHAR pData = (PUCHAR)(&ph->UrbHeader);
	_URB_CONTROL_TRANSFER *pCT = (_URB_CONTROL_TRANSFER *)&ph->UrbHeader;
	if (g_bIsWow64) {
		// Yuk! We're getting a 64 bit struct, and we expect a 32 bit one!
		// Copy it section by section between pointers/handles.
		_URB_CONTROL_TRANSFER *mpCT = pCT;
TRACE("~1 ControlTransf: Hdr.Length before = %d\n",mpCT->Hdr.Length);

		// First part of header including Device Handle
	    CopyMemory(&m_ControlTransfer,
		            mpCT,
		            FIELD_OFFSET(_URB_CONTROL_TRANSFER, Hdr.UsbdFlags));
		mpCT = (_URB_CONTROL_TRANSFER *) ((UCHAR *)mpCT + 4);	// Skip top 32 bits of handle
		m_ControlTransfer.Hdr.Length -= 4;

		// Balance of header, UsbdFlags
		m_ControlTransfer.Hdr.UsbdFlags = mpCT->Hdr.UsbdFlags;

		// Align to 64 bit boundary
		mpCT = (_URB_CONTROL_TRANSFER *) ((UCHAR *)mpCT + 4);	// Skip top 32 bits of handle
		m_ControlTransfer.Hdr.Length -= 4;

		// Lower 32 bits of PipeHandle
		m_ControlTransfer.PipeHandle = mpCT->PipeHandle;
		mpCT = (_URB_CONTROL_TRANSFER *) ((UCHAR *)mpCT + 4);	// Skip top 32 bits of handle
		m_ControlTransfer.Hdr.Length -= 4;

		// From TransferFlags including bottom 32 bits of TransferBuffer pointer
	    CopyMemory(&m_ControlTransfer.TransferFlags,
		           &mpCT->TransferFlags,
		           FIELD_DIFF(m_ControlTransfer.TransferFlags,
		                      m_ControlTransfer.TransferBufferMDL));
		mpCT = (_URB_CONTROL_TRANSFER *) ((UCHAR *)mpCT + 4);	// Skip top 32 bits of pointer
		m_ControlTransfer.Hdr.Length -= 4;

		// TransferBufferMDL pointer
		m_ControlTransfer.TransferBufferMDL = mpCT->TransferBufferMDL;
		mpCT = (_URB_CONTROL_TRANSFER *) ((UCHAR *)mpCT + 4);	// Skip top 32 bits of pointer
		m_ControlTransfer.Hdr.Length -= 4;

		// UrbLink pointer
		m_ControlTransfer.UrbLink = mpCT->UrbLink;
		mpCT = (_URB_CONTROL_TRANSFER *) ((UCHAR *)mpCT + 4);	// Skip top 32 bits of pointer
		m_ControlTransfer.Hdr.Length -= 4;
		
		// _URB_HCD_AREA is 8 pointers, and it's not used. Skip values
		mpCT = (_URB_CONTROL_TRANSFER *) ((UCHAR *)mpCT + sizeof(struct _URB_HCD_AREA));
		m_ControlTransfer.Hdr.Length -= sizeof(struct _URB_HCD_AREA);
		
		// The SetupPacket
	    CopyMemory(&m_ControlTransfer.SetupPacket,
		           &mpCT->SetupPacket,
		           8);
TRACE("~1 ControlTransf: Hdr.Length after = %d\n",m_ControlTransfer.Hdr.Length);
	} else {
	    CopyMemory(&m_ControlTransfer, pData, sizeof(m_ControlTransfer));
	}
	pData += pCT->Hdr.Length;
    GrabTransferBuffer(pData, m_ControlTransfer.TransferBufferLength);
TRACE("~1 ControlTransf: TransferBufferLength = %d\n",m_ControlTransfer.TransferBufferLength);
    
    SetPipeHandle(m_ControlTransfer.PipeHandle);

    if(!IsDirectionKnown())
    {
        switch(m_nFunction)
        {
        case URB_FUNCTION_VENDOR_DEVICE:
        case URB_FUNCTION_VENDOR_ENDPOINT:
        case URB_FUNCTION_VENDOR_INTERFACE:
        case URB_FUNCTION_VENDOR_OTHER:
            SetDirection(
                0 != (m_ControlTransfer.TransferFlags & USBD_TRANSFER_DIRECTION_IN),
                0 == (m_ControlTransfer.TransferFlags & USBD_TRANSFER_DIRECTION_IN));
            break;

        default:
            {
                UCHAR bmRequestType = m_ControlTransfer.SetupPacket[0];
                SetDirection(USB_ENDPOINT_DIRECTION_IN(bmRequestType), USB_ENDPOINT_DIRECTION_OUT(bmRequestType));
            }
            break;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
// CURB_BulkOrInterruptTransfer

CURB_BulkOrInterruptTransfer::CURB_BulkOrInterruptTransfer(void)
{
    ZeroMemory(&m_BulkOrInterruptTransfer, sizeof(m_BulkOrInterruptTransfer));
}

IMPLEMENT_SERIAL(CURB_BulkOrInterruptTransfer, CURB_TransferBuffer, 1)

void CURB_BulkOrInterruptTransfer::Serialize(CArchive &ar)
{
    CURB_TransferBuffer::Serialize(ar);

    if(ar.IsStoring())
    {
        ar.Write(&m_BulkOrInterruptTransfer, sizeof(m_BulkOrInterruptTransfer));
    }
    else
    {
        ar.Read(&m_BulkOrInterruptTransfer, sizeof(m_BulkOrInterruptTransfer));
    }
}

void CURB_BulkOrInterruptTransfer::RenderProperties(void)
{
    CURB::RenderProperties();
    
    AddPropLine(_T("  TransferFlags: 0x%08x"), m_BulkOrInterruptTransfer.TransferFlags);

    CURB_TransferBuffer::RenderProperties();
}

void CURB_BulkOrInterruptTransfer::GrabData(PPACKET_HEADER ph)
{
    CURB::GrabData(ph);
    
    PUCHAR pData = (PUCHAR)(&ph->UrbHeader);
	_URB_BULK_OR_INTERRUPT_TRANSFER *pBIT = (_URB_BULK_OR_INTERRUPT_TRANSFER *)&ph->UrbHeader;
	if (g_bIsWow64) {
		// Yuk! We're getting a 64 bit struct, and we expect a 32 bit one!
		// Copy it section by section between pointers/handles.
		_URB_BULK_OR_INTERRUPT_TRANSFER *mpBIT = pBIT;
TRACE("~1 BulkOrInterTrans: Hdr.Length before = %d\n",mpBIT->Hdr.Length);

		// First part of header including bottom 32 bit of Device Handle
	    CopyMemory(&m_BulkOrInterruptTransfer,
		            mpBIT,
		            FIELD_OFFSET(_URB_BULK_OR_INTERRUPT_TRANSFER, Hdr.UsbdFlags));
		mpBIT = (_URB_BULK_OR_INTERRUPT_TRANSFER *) ((UCHAR *)mpBIT + 4);	// Skip top 32 handle
		m_BulkOrInterruptTransfer.Hdr.Length -= 4;

		// Balance of header, UsbdFlags
		m_BulkOrInterruptTransfer.Hdr.UsbdFlags = mpBIT->Hdr.UsbdFlags;

		// Align to 64 bit boundary
		mpBIT = (_URB_BULK_OR_INTERRUPT_TRANSFER *) ((UCHAR *)mpBIT + 4);	// Skip top 32 handle
		m_BulkOrInterruptTransfer.Hdr.Length -= 4;

		// Lower 32 bits of PipeHandle
		m_BulkOrInterruptTransfer.PipeHandle = mpBIT->PipeHandle;
		mpBIT = (_URB_BULK_OR_INTERRUPT_TRANSFER *) ((UCHAR *)mpBIT + 4);	// Skip top 32 handle
		m_BulkOrInterruptTransfer.Hdr.Length -= 4;

		// From TransferFlags including TransferBuffer pointer
	    CopyMemory(&m_BulkOrInterruptTransfer.TransferFlags,
		           &mpBIT->TransferFlags,
		           FIELD_DIFF(m_BulkOrInterruptTransfer.TransferFlags,
		                      m_BulkOrInterruptTransfer.TransferBufferMDL));
		mpBIT = (_URB_BULK_OR_INTERRUPT_TRANSFER *) ((UCHAR *)mpBIT + 4);	// Skip top 32 pointer
		m_BulkOrInterruptTransfer.Hdr.Length -= 4;

		// TransferBufferMDL pointer
		m_BulkOrInterruptTransfer.TransferBufferMDL = mpBIT->TransferBufferMDL;
		mpBIT = (_URB_BULK_OR_INTERRUPT_TRANSFER *) ((UCHAR *)mpBIT + 4);	// Skip top 32 pointer
		m_BulkOrInterruptTransfer.Hdr.Length -= 4;

		// UrbLink pointer
		m_BulkOrInterruptTransfer.UrbLink = mpBIT->UrbLink;
		mpBIT = (_URB_BULK_OR_INTERRUPT_TRANSFER *) ((UCHAR *)mpBIT + 4);	// Skip top 32 pointer
		m_BulkOrInterruptTransfer.Hdr.Length -= 4;
		
		// _URB_HCD_AREA is 100% pointers, and it's not used. Skip it
		mpBIT = (_URB_BULK_OR_INTERRUPT_TRANSFER *) ((UCHAR *)mpBIT + sizeof(struct _URB_HCD_AREA));
		m_BulkOrInterruptTransfer.Hdr.Length -= sizeof(struct _URB_HCD_AREA);

TRACE("~1 BulkOrInterTrans: Hdr.Length after = %d\n",m_BulkOrInterruptTransfer.Hdr.Length);
	} else {
	    CopyMemory(&m_BulkOrInterruptTransfer, pData, sizeof(m_BulkOrInterruptTransfer));
	}
    pData += pBIT->Hdr.Length;
    GrabTransferBuffer(pData, m_BulkOrInterruptTransfer.TransferBufferLength);

    SetPipeHandle(m_BulkOrInterruptTransfer.PipeHandle);
}

//////////////////////////////////////////////////////////////////////////
// CURB_IsochTransfer

CURB_IsochTransfer::CURB_IsochTransfer(void)
{
    m_IsochTransferLen = 0;
    m_pIsochTransfer = NULL;
    m_nTransferBufferCnt = 0;
    for(int nPacket = 0; nPacket < ISOCH_MAXPACKETS; ++nPacket)
    {
        m_nTransferBufferLength[nPacket] = 0;
        m_pTransferBuffer[nPacket] = NULL;
    }
}

CURB_IsochTransfer::~CURB_IsochTransfer(void)
{
    m_IsochTransferLen = 0;
    m_pIsochTransfer = NULL;
    for(int nPacket = 0; nPacket < ISOCH_MAXPACKETS; ++nPacket)
    {
        m_nTransferBufferLength[nPacket] = 0;
        m_pTransferBuffer[nPacket] = NULL;
    }
}

IMPLEMENT_SERIAL(CURB_IsochTransfer, CURB, 1)

void CURB_IsochTransfer::Serialize(CArchive &ar)
{
    CURB::Serialize(ar);
    
    if(ar.IsStoring())
    {
        ar.WriteCount(m_IsochTransferLen);
        ar.Write(m_pIsochTransfer, m_IsochTransferLen);
        ar.WriteCount(m_nTransferBufferCnt);
        for(ULONG nPacket = 0; nPacket < m_nTransferBufferCnt; ++nPacket)
        {
            ar.WriteCount(m_nTransferBufferLength[nPacket]);
            ar.Write(m_pTransferBuffer[nPacket], m_nTransferBufferLength[nPacket]);
        }
        
    }
    else
    {
        m_IsochTransferLen = ar.ReadCount();
        m_pIsochTransfer = (_URB_ISOCH_TRANSFER*) m_pCA->AllocBlock(m_IsochTransferLen);
        ar.Read(m_pIsochTransfer, m_IsochTransferLen);
        m_nTransferBufferCnt = ar.ReadCount();
        for(ULONG nPacket = 0; nPacket < m_nTransferBufferCnt; ++nPacket)
        {
            m_nTransferBufferLength[nPacket] = ar.ReadCount();
            m_pTransferBuffer[nPacket] = (PUCHAR) m_pCA->AllocBlock(m_nTransferBufferLength[nPacket]);
            ar.Read(m_pTransferBuffer[nPacket], m_nTransferBufferLength[nPacket]);
        }        
    }
}

int CURB_IsochTransfer::GetPacketCount(void)
{
    return m_nTransferBufferCnt;
}

int CURB_IsochTransfer::GetPayloadCount(int nPacket)
{
    if(0 <= nPacket)
    {
        return m_nTransferBufferLength[nPacket];
    }

    int nTotalLength = 0;
    for(DWORD nIndex = 0; nIndex < m_nTransferBufferCnt; ++nIndex)
    {
        nTotalLength += m_nTransferBufferLength[nIndex];
    }
    return nTotalLength;
}

PUCHAR CURB_IsochTransfer::GetPayload(int nPacket)
{
    ASSERT(0 <= nPacket);
    ASSERT(nPacket < (int) m_nTransferBufferCnt);
    return m_pTransferBuffer[nPacket];
}

LPCTSTR CURB_IsochTransfer::GetPayloadXML(int nPacket, LPTSTR sBuffer)
{
    if(NULL == GetPayload(nPacket))
    {
        return CURB::GetPayloadXML(nPacket, sBuffer);
    }
    
    ASSERT(0 <= nPacket);
    ASSERT(nPacket < (int) m_nTransferBufferCnt);
    PUCHAR pTransferBuffer = m_pTransferBuffer[nPacket];
    for(DWORD nIndex = 0; nIndex < m_nTransferBufferLength[nPacket]; ++nIndex)
    {
#ifdef UNICODE
		swprintf(&sBuffer[nIndex << 1], _T("%02x"), pTransferBuffer[nIndex]);
#else
		sprintf(&sBuffer[nIndex << 1], _T("%02x"), pTransferBuffer[nIndex]);
#endif
	}
    return sBuffer;
}

void CURB_IsochTransfer::GetDataDumpStr(LPTSTR sBuffer)
{
    if(0 != m_nTransferBufferCnt)
    {
        GenerateHexDumpLine(sBuffer, m_pTransferBuffer[0], min(8, m_nTransferBufferLength[0]));
    }
    else
    {
        _tcscpy(sBuffer, _T("-"));
    }
}

void CURB_IsochTransfer::RenderProperties(void)
{
    CURB::RenderProperties();

    AddPropLine(_T("  NumberOfPackets: 0x%08x (%d)"), 
        m_pIsochTransfer->NumberOfPackets, 
        m_pIsochTransfer->NumberOfPackets);

    for(ULONG nPacket = 0; nPacket < m_pIsochTransfer->NumberOfPackets; ++nPacket)
    {
        PUSBD_ISO_PACKET_DESCRIPTOR pPacket = &m_pIsochTransfer->IsoPacket[nPacket];
        AddPropLine(_T("  Packet %2d: Ofs: 0x%08x  Len: 0x%08x  Status: 0x%08x"),
            nPacket, pPacket->Offset, pPacket->Length, pPacket->Status);
    }

    for(ULONG nPacket = 0; nPacket < m_nTransferBufferCnt; ++nPacket)
    {
        AddPropLine(_T("  "));
        AddPropLine(_T("  Buffer: %2d: Len: 0x%08x (%d)"), nPacket,
            m_nTransferBufferLength[nPacket], m_nTransferBufferLength[nPacket]);
        GenerateHexDump(m_pTransferBuffer[nPacket], m_nTransferBufferLength[nPacket]);
    }
}

void CURB_IsochTransfer::GrabData(PPACKET_HEADER ph)
{
    CURB::GrabData(ph);
    
    PUCHAR pData = (PUCHAR)(&ph->UrbHeader);
    m_pIsochTransfer = (_URB_ISOCH_TRANSFER*) m_pCA->AllocBlock(m_IsochTransferLen);
	if (g_bIsWow64) {
		// Yuk! We're getting a 64 bit struct, and we expect a 32 bit one!
		// Copy it section by section between pointers/handles.
		_URB_ISOCH_TRANSFER *mpIST = (_URB_ISOCH_TRANSFER *)&ph->UrbHeader;
TRACE("~1 IsochTransf: Hdr.Length before = %d\n",mpIST->Hdr.Length);

		// First part of header including Device Handle
	    CopyMemory(m_pIsochTransfer,
		            mpIST,
		            FIELD_OFFSET(_URB_ISOCH_TRANSFER, Hdr.UsbdFlags));
		mpIST = (_URB_ISOCH_TRANSFER *) ((UCHAR *)mpIST + 4);	// Skip top 32 bits of handle
		m_pIsochTransfer->Hdr.Length -= 4;

		// Balance of header, UsbdFlags
		m_pIsochTransfer->Hdr.UsbdFlags = mpIST->Hdr.UsbdFlags;

		// Align to 64 bit boundary
		mpIST = (_URB_ISOCH_TRANSFER *) ((UCHAR *)mpIST + 4);	// Skip top 32 bits of handle
		m_pIsochTransfer->Hdr.Length -= 4;

		// Lower 32 bits of PipeHandle
		m_pIsochTransfer->PipeHandle = mpIST->PipeHandle;
		mpIST = (_URB_ISOCH_TRANSFER *) ((UCHAR *)mpIST + 4);	// Skip top 32 bits of handle
		m_pIsochTransfer->Hdr.Length -= 4;

		// From TransferFlags including TransferBuffer pointer
	    CopyMemory(&m_pIsochTransfer->TransferFlags,
		           &mpIST->TransferFlags,
		           FIELD_DIFF(m_pIsochTransfer->TransferFlags,
		                      m_pIsochTransfer->TransferBufferMDL));
		mpIST = (_URB_ISOCH_TRANSFER *) ((UCHAR *)mpIST + 4);	// Skip top 32 bits of pointer
		m_pIsochTransfer->Hdr.Length -= 4;

		// TransferBufferMDL pointer
		m_pIsochTransfer->TransferBufferMDL = mpIST->TransferBufferMDL;
		mpIST = (_URB_ISOCH_TRANSFER *) ((UCHAR *)mpIST + 4);	// Skip top 32 bits of pointer
		m_pIsochTransfer->Hdr.Length -= 4;

		// UrbLink pointer
		m_pIsochTransfer->UrbLink = mpIST->UrbLink;
		mpIST = (_URB_ISOCH_TRANSFER *) ((UCHAR *)mpIST + 4);	// Skip top 32 bits of pointer
		m_pIsochTransfer->Hdr.Length -= 4;
		
		// _URB_HCD_AREA is 100% pointers, and it's not used. Skip it
		mpIST = (_URB_ISOCH_TRANSFER *) ((UCHAR *)mpIST + sizeof(struct _URB_HCD_AREA));
		m_pIsochTransfer->Hdr.Length -= sizeof(struct _URB_HCD_AREA);

		// From StartFrame to IsoPacket[m_pIsochTransfer.NumberOfPackets]
	    CopyMemory(&m_pIsochTransfer->StartFrame,
		           &mpIST->StartFrame,
		           FIELD_DIFF(m_pIsochTransfer->StartFrame,
		                      m_pIsochTransfer->IsoPacket[mpIST->NumberOfPackets]));

	    m_IsochTransferLen = FIELD_OFFSET(_URB_ISOCH_TRANSFER, IsoPacket[0])
		      + mpIST->NumberOfPackets * sizeof(USBD_ISO_PACKET_DESCRIPTOR);
TRACE("~1 IsochTransf: Hdr.Length after = %d\n",m_pIsochTransfer->Hdr.Length);
	} else {

	    CopyMemory(m_pIsochTransfer, pData, m_IsochTransferLen);
	    m_IsochTransferLen = ph->UrbHeader.Length;
	}
    pData += ph->UrbHeader.Length;;
    
    // special grabbing for isoch transfer data...
    GrabBytes(pData, &m_nTransferBufferCnt, sizeof(m_nTransferBufferCnt));
    for(ULONG nPacket = 0; nPacket < m_nTransferBufferCnt; ++nPacket)
    {
        GrabBytes(pData, &m_nTransferBufferLength[nPacket], sizeof(ULONG));
        m_pTransferBuffer[nPacket] = (PUCHAR) m_pCA->AllocBlock(m_nTransferBufferLength[nPacket]);
        GrabBytes(pData, m_pTransferBuffer[nPacket], m_nTransferBufferLength[nPacket]);
    }

    SetPipeHandle(m_pIsochTransfer->PipeHandle);
}

//////////////////////////////////////////////////////////////////////////
// CArrayURB

CArrayURB::CArrayURB(void)
{
    m_bTimeIsRelative = TRUE;
}

CArrayURB::~CArrayURB(void)
{
}

void CArrayURB::Serialize(CArchive &ar)
{
    CArray<CURB*, CURB*>::Serialize(ar);
    
    if(ar.IsStoring())
    {
        ar.WriteCount(m_dwPlugTimeStamp);
        ar.WriteCount(m_bTimeIsRelative ? 1 : 0);
        for(int nURB = 0; nURB < GetSize(); nURB++)
        {
            CURB *pURB = GetAt(nURB);
            ar.WriteClass(pURB->GetRuntimeClass());
            pURB->Serialize(ar);
        }
    }
    else
    {
        m_dwPlugTimeStamp = ar.ReadCount();
        m_bTimeIsRelative = (1 == ar.ReadCount());
        for(int nURB = 0; nURB < GetSize(); nURB++)
        {
            CRuntimeClass *pClass = ar.ReadClass();
            if(!pClass->IsDerivedFrom(RUNTIME_CLASS(CURB)))
            {
                TRACE("unknown runtime class!\n");
                AfxThrowArchiveException(CArchiveException::badClass);
            }

            CURB *pURB = (CURB*) pClass->CreateObject();
            ASSERT(NULL != pURB);
            pURB->SetChunkAllocator(&m_ChunkAllocator);
            pURB->Serialize(ar);
            SetAt(nURB, pURB);
        }
    }
}

BOOL CArrayURB::UnserializeURB(PPACKET_HEADER ph)
{
	CURB *pURB = NULL;
	TRACE("UnserializeURB(): function %d (0x%08x)\n", ph->UrbHeader.Function, ph->UrbHeader.Function);
	switch(ph->UrbHeader.Function)
	{
	case URB_FUNCTION_SELECT_CONFIGURATION:
		pURB = new CURB_SelectConfiguration();
		break;

	case URB_FUNCTION_SELECT_INTERFACE:
        pURB = new CURB_SelectInterface();
        break;

	case URB_FUNCTION_ABORT_PIPE:
	case URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL:
	case URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL:
	case URB_FUNCTION_GET_FRAME_LENGTH:
	case URB_FUNCTION_SET_FRAME_LENGTH:
	case URB_FUNCTION_GET_CURRENT_FRAME_NUMBER:
	case URB_FUNCTION_RESET_PIPE:
	case URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE:
	case URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT:
	case URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE:
	case URB_FUNCTION_SET_FEATURE_TO_DEVICE:
	case URB_FUNCTION_SET_FEATURE_TO_INTERFACE:
	case URB_FUNCTION_SET_FEATURE_TO_ENDPOINT:
	case URB_FUNCTION_SET_FEATURE_TO_OTHER:
	case URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE:
	case URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE:
	case URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT:
	case URB_FUNCTION_CLEAR_FEATURE_TO_OTHER:
	case URB_FUNCTION_GET_CONFIGURATION:
	case URB_FUNCTION_GET_INTERFACE:
		// no user data apart from the stuff in the URB itself
		pURB = new CURB();
		break;

	case URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE:
	case URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT:
	case URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE:
	case URB_FUNCTION_GET_STATUS_FROM_DEVICE:
	case URB_FUNCTION_GET_STATUS_FROM_INTERFACE:
	case URB_FUNCTION_GET_STATUS_FROM_ENDPOINT:
	case URB_FUNCTION_GET_STATUS_FROM_OTHER:
	case URB_FUNCTION_VENDOR_DEVICE:
	case URB_FUNCTION_VENDOR_INTERFACE:
	case URB_FUNCTION_VENDOR_ENDPOINT:
	case URB_FUNCTION_VENDOR_OTHER:
	case URB_FUNCTION_CLASS_DEVICE:
	case URB_FUNCTION_CLASS_INTERFACE:
	case URB_FUNCTION_CLASS_ENDPOINT:
	case URB_FUNCTION_CLASS_OTHER:
	case URB_FUNCTION_CONTROL_TRANSFER:
        pURB = new CURB_ControlTransfer();
        break;

	case URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER:
        pURB = new CURB_BulkOrInterruptTransfer();
        break;

    case URB_FUNCTION_ISOCH_TRANSFER:
        pURB = new CURB_IsochTransfer();
		break;

	default:
		TRACE("UnserializeURB(): unknown URB function %d (0x%08x)...\n", ph->UrbHeader.Function, ph->UrbHeader.Function);
		break;
	}

	if(NULL != pURB)
	{
        pURB->SetChunkAllocator(&m_ChunkAllocator);
		pURB->GrabData(ph);
        Add(pURB);
        return TRUE;
	}

	return FALSE;
}

void CArrayURB::SetTimeFormat(BOOL bRelative)
{
    m_bTimeIsRelative = bRelative;
}

BOOL CArrayURB::IsTimeFormatRelative(void)
{
    return m_bTimeIsRelative;
}

void CArrayURB::SetTimeStampZero(ULONG uTimeStampZero)
{
    m_dwPlugTimeStamp = uTimeStampZero;
}

ULONG CArrayURB::GetTimeStampZero(void)
{
    return m_dwPlugTimeStamp;
}

int CArrayURB::FindBySequenceAndDirection(DWORD dwSequenceNr, BOOL bIsComingUp)
{
    int nURB = 0;
    while(nURB < GetSize())
    {
        CURB *pURB = GetAt(nURB);
        if((dwSequenceNr == pURB->GetSequenceNr()) && (bIsComingUp == pURB->IsComingUp()))
        {
            return nURB;
        }
        ++nURB;
    }
    return -1;
}


//////////////////////////////////////////////////////////////////////////
// CMyDWORDArray

void CMyDWORDArray::RemoveDuplicates(void)
{
    int nIndex = 0;
    while(nIndex < GetSize() - 1)
    {
        if(ElementAt(nIndex) == ElementAt(nIndex + 1))
        {
            RemoveAt(nIndex);
        }
        else
        {
            nIndex++;
        }
    }
}


//** end of URB.cpp ******************************************************
/*************************************************************************

  $Log: /CD/Entertainment/Tools/Snoopy/SnoopyPro/URB.cpp $
 * 
 * 6     10/07/02 2:52p Rbosa
 * 
 * 5     10/07/02 12:49p Rbosa
  Revision 1.2  2002/10/05 01:10:43  rbosa
  Added the basic framework for exporting a log into an XML file. The
  output written is fairly poor. This checkin is mainly to get the
  framework in place and get feedback on it.

  Revision 1.1  2002/08/14 23:03:34  rbosa
  the application to capture urbs and display them...

 * 
 * 4     2/22/02 6:13p Rbosa
 * - fixed Isochronous IN in terms of transferbuffer displayed
 * - added some flags for direction
 * - some cosmetics
 * - more debug output on descriptors
 * 
 * 3     2/05/02 9:02p Rbosa
 * - added relative/absolute time functionality
 * - added decoding of URB_SELECT_INTERFACE
 * - put a couple of URB which are like a control transfer to the control
 * transfer family
 * 
 * 2     1/25/02 2:48p Rbosa
 * This is a good state to check in...not everything is implemented, but
 * one can use the Snooper in this state...
 * 
 * 1     8/21/01 7:07p Rbosa

*************************************************************************/

