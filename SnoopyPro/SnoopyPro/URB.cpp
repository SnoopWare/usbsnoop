//************************************************************************
//
// URB.cpp
//
// implementation of the CURB class.
//
//************************************************************************

#include "StdAfx.h"
#include "URB.h"

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
        URBFCN(URB_FUNCTION_SELECT_CONFIGURATION, "SELECT_CONFIGURATION", IS_SELECT | IS_DIR_IN | IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SELECT_INTERFACE, "SELECT_INTERFACE", IS_SELECT | IS_DIR_IN | IS_DIR_OUT);
        URBFCN(URB_FUNCTION_ABORT_PIPE, "ABORT_PIPE", 0);
        URBFCN(URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL, "TAKE_FRAME_LENGTH_CONTROL", 0);
        URBFCN(URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL, "RELEASE_FRAME_LENGTH_CONTROL", 0);
        URBFCN(URB_FUNCTION_GET_FRAME_LENGTH, "GET_FRAME_LENGTH", IS_DIR_IN);
        URBFCN(URB_FUNCTION_SET_FRAME_LENGTH, "SET_FRAME_LENGTH", IS_DIR_OUT);
        URBFCN(URB_FUNCTION_GET_CURRENT_FRAME_NUMBER, "GET_CURRENT_FRAME_NUMBER", 0);
        URBFCN(URB_FUNCTION_CONTROL_TRANSFER, "CONTROL_TRANSFER", IS_CONTROL);
        URBFCN(URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER, "BULK_OR_INTERRUPT_TRANSFER", IS_BULK);
        URBFCN(URB_FUNCTION_ISOCH_TRANSFER, "ISOCH_TRANSFER", 0);
        URBFCN(URB_FUNCTION_RESET_PIPE, "RESET_PIPE", 0);
        URBFCN(URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE, "GET_DESCRIPTOR_FROM_DEVICE", IS_DIR_IN);
        URBFCN(URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT, "GET_DESCRIPTOR_FROM_ENDPOINT", IS_DIR_IN);
        URBFCN(URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE, "GET_DESCRIPTOR_FROM_INTERFACE", IS_DIR_IN);
        URBFCN(URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE, "SET_DESCRIPTOR_TO_DEVICE", IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT, "SET_DESCRIPTOR_TO_ENDPOINT", IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE, "SET_DESCRIPTOR_TO_INTERFACE", IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SET_FEATURE_TO_DEVICE, "SET_FEATURE_TO_DEVICE", IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SET_FEATURE_TO_INTERFACE, "SET_FEATURE_TO_INTERFACE", IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SET_FEATURE_TO_ENDPOINT, "SET_FEATURE_TO_ENDPOINT", IS_DIR_OUT);
        URBFCN(URB_FUNCTION_SET_FEATURE_TO_OTHER, "SET_FEATURE_TO_OTHER", IS_DIR_OUT);
        URBFCN(URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE, "CLEAR_FEATURE_TO_DEVICE", IS_DIR_OUT);
        URBFCN(URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE, "CLEAR_FEATURE_TO_INTERFACE", IS_DIR_OUT);
        URBFCN(URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT, "CLEAR_FEATURE_TO_ENDPOINT", IS_DIR_OUT);
        URBFCN(URB_FUNCTION_CLEAR_FEATURE_TO_OTHER, "CLEAR_FEATURE_TO_OTHER", IS_DIR_OUT);
        URBFCN(URB_FUNCTION_GET_STATUS_FROM_DEVICE, "GET_STATUS_FROM_DEVICE", IS_DIR_IN);
        URBFCN(URB_FUNCTION_GET_STATUS_FROM_INTERFACE, "GET_STATUS_FROM_INTERFACE", IS_DIR_IN);
        URBFCN(URB_FUNCTION_GET_STATUS_FROM_ENDPOINT, "GET_STATUS_FROM_ENDPOINT", IS_DIR_IN);
        URBFCN(URB_FUNCTION_GET_STATUS_FROM_OTHER, "GET_STATUS_FROM_OTHER", IS_DIR_IN);
        URBFCN(URB_FUNCTION_RESERVED0, "RESERVED0", 0);
        URBFCN(URB_FUNCTION_VENDOR_DEVICE, "VENDOR_DEVICE", 0);
        URBFCN(URB_FUNCTION_VENDOR_INTERFACE, "VENDOR_INTERFACE", 0);
        URBFCN(URB_FUNCTION_VENDOR_ENDPOINT, "VENDOR_ENDPOINT", 0);
        URBFCN(URB_FUNCTION_VENDOR_OTHER, "VENDOR_OTHER", 0);
        URBFCN(URB_FUNCTION_CLASS_DEVICE, "CLASS_DEVICE", 0);
        URBFCN(URB_FUNCTION_CLASS_INTERFACE, "CLASS_INTERFACE", 0);
        URBFCN(URB_FUNCTION_CLASS_ENDPOINT, "CLASS_ENDPOINT", 0);
        URBFCN(URB_FUNCTION_CLASS_OTHER, "CLASS_OTHER", 0);
        URBFCN(URB_FUNCTION_RESERVED, "RESERVED", 0);
        URBFCN(URB_FUNCTION_GET_CONFIGURATION, "GET_CONFIGURATION", IS_DIR_IN);
        URBFCN(URB_FUNCTION_GET_INTERFACE, "GET_INTERFACE", IS_DIR_IN);
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
    _stprintf(sBuffer, "%d", m_dwSequence);
}

DWORD CURB::GetSequenceNr(void)
{
    return m_dwSequence;
}

void CURB::GetDirection(LPTSTR sBuffer)
{
    if(IsDirectionIn() && IsDirectionOut())
    {
        _tcscpy(sBuffer, "inout");
    }
    else if(IsDirectionIn())
    {
        _tcscpy(sBuffer, "in");
    }
    else if(IsDirectionOut())
    {
        _tcscpy(sBuffer, "out");
    }
    else
    {
        _tcscpy(sBuffer, "???");
    }

    if(IsComingUp())
    {
        _tcscat(sBuffer, " up");
    }
    else
    {
        _tcscat(sBuffer, " down");
    }
}

void CURB::GetEndpoint(LPTSTR sBuffer)
{
    if(URBFLAGS_ENDPOINTKNOWN & m_dwFlags)
    {
        _stprintf(sBuffer, "0x%02x", m_cEndpoint);
    }
    else
    {
        _tcscpy(sBuffer, "n/a");
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

void CURB::GetDataDump(LPTSTR sBuffer)
{
    _tcscpy(sBuffer, "");
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

int CURB::GetExpandSize(void)
{
    if(-1 == m_nExpandedSize)
    {
        // prepare the expanded view
        m_arRenderedLines.RemoveAll();
        ASSERT(NULL != m_pCA);
        RenderProperties();
        m_nExpandedSize = m_arRenderedLines.GetSize();
    }

    ASSERT(0 <= m_nExpandedSize);
    return m_nExpandedSize;
}

void CURB::AddPropLine(LPCTSTR sFormat, ...)
{
    if(MAX_LINES_PER_URB == m_arRenderedLines.GetSize())
    {
        m_arRenderedLines.Add(m_pCA->AllocLine("...", 3));
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
    AddPropLine("URB Header (length: %d)", m_PH.UrbHeader.Length);
    AddPropLine("SequenceNumber: %d", m_dwSequence);
    AddPropLine("Function: %04x (%s)", m_nFunction, GetFunctionStr());
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
    static TCHAR s_HexDigit[] = "0123456789abcdef";

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
    //  4 bytes for address
    //  2 bytes for ': '
    // 3n bytes for the hexdump ('%02x ')
    //  1 byte for 0x00 terminator
    // --
    // 7 + 3n bytes per line (with n==8 bytes per line)
    
    const int nBytesPerLine = 16;
    TCHAR sLine[7 + 3 * nBytesPerLine];
    
    int nAddress = 0;
    while(nSize > 0)
    {
        LPTSTR sBuffer = sLine;
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

void CURB_TransferBuffer::GetDataDump(LPTSTR sBuffer)
{
    if(m_TransferBuffer)
    {
        GenerateHexDumpLine(sBuffer, m_TransferBuffer, min(8, m_TransferLength));
    }
    else
    {
        _tcscpy(sBuffer, "-");
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
        AddPropLine("");
        AddPropLine("TransferBuffer: 0x%08x (%d) length", m_TransferLength, m_TransferLength);
        GenerateHexDump(m_TransferBuffer, m_TransferLength);
    }
    else
    {
        AddPropLine("");
        AddPropLine("No TransferBuffer");
        AddPropLine("");
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
		AddPropLine("Configuration Descriptor:");

        CString S;
        AddPropLine("bLength: %d (0x%02x)", m_UCD.bLength, m_UCD.bLength);
        AddPropLine("bDescriptorType: %d (0x%02x)", m_UCD.bDescriptorType, m_UCD.bDescriptorType);
        if(m_UCD.bDescriptorType != 0x02)
        {
            AddPropLine("  (should be 0x02!)");
        }
        AddPropLine("wTotalLength: %d (0x%04x)", m_UCD.wTotalLength, m_UCD.wTotalLength);
        AddPropLine("bNumInterfaces: %d (0x%02x)", m_UCD.bNumInterfaces, m_UCD.bNumInterfaces);
        AddPropLine("bConfigurationValue: %d (0x%02x)", m_UCD.bConfigurationValue, m_UCD.bConfigurationValue);
        AddPropLine("iConfiguration: %d (0x%02x)", m_UCD.iConfiguration, m_UCD.iConfiguration);
        AddPropLine("bmAttributes: %d (0x%02x)", m_UCD.bmAttributes, m_UCD.bmAttributes);
        if(0 != (0x80 & m_UCD.bmAttributes))
        {
            AddPropLine("  0x80: Bus Powered");
        }
        if(0 != (0x40 & m_UCD.bmAttributes))
        {
            AddPropLine("  0x40: Self Powered");
        }
        if(0 != (0x20 & m_UCD.bmAttributes))
        {
            AddPropLine("  0x20: Remote Wakeup");
        }
        AddPropLine("MaxPower: %d (0x%02x)", m_UCD.MaxPower, m_UCD.MaxPower);
        AddPropLine("  (in 2 mA units, therefore %d mA power consumption)", m_UCD.MaxPower * 2);

        AddPropLine("");
        AddPropLine("Number of interfaces: %d", m_uNumInterfaces);
        
        for(ULONG i = 0; i < m_uNumInterfaces; i++)
        {
            AddPropLine("Interface[%d]:", i);
            AddPropLine("  Length: 0x%04x", m_pInterfaces[i]->Length);
            if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, InterfaceNumber))
            {
                AddPropLine("  InterfaceNumber: 0x%02x", m_pInterfaces[i]->InterfaceNumber);
                if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, AlternateSetting))
                {
                    AddPropLine("  AlternateSetting: 0x%02x", m_pInterfaces[i]->AlternateSetting);
                    if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, Class))
                    {
                        AddPropLine("  Class             = 0x%02x", m_pInterfaces[i]->Class);
                        if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, SubClass))
                        {
                            AddPropLine("  SubClass          = 0x%02x", m_pInterfaces[i]->SubClass);
                            if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, Protocol))
                            {
                                AddPropLine("  Protocol          = 0x%02x", m_pInterfaces[i]->Protocol);
                                if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, InterfaceHandle))
                                {
                                    AddPropLine("  InterfaceHandle   = 0x%08x", m_pInterfaces[i]->InterfaceHandle);
                                    if(m_pInterfaces[i]->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, NumberOfPipes))
                                    {
                                        AddPropLine("  NumberOfPipes     = 0x%08x", m_pInterfaces[i]->NumberOfPipes);
                                        ULONG uNumPipes = m_pInterfaces[i]->NumberOfPipes;
                                        if(uNumPipes > 0x1f)
                                        {
                                            AddPropLine("ERROR: uNumPipes is too large (%d), resetting to 1", uNumPipes);
                                            uNumPipes = 1;
                                        }
                                        
                                        for(ULONG p = 0; p < uNumPipes; p++)
                                        {
                                            if(m_pInterfaces[i]->Length >= (FIELD_OFFSET(USBD_INTERFACE_INFORMATION, Pipes) + (p + 1) * sizeof(USBD_PIPE_INFORMATION)))
                                            {
                                                PUSBD_PIPE_INFORMATION pPipe = &m_pInterfaces[i]->Pipes[p];
                                                AddPropLine("  Pipe[%d]:", p);
                                                AddPropLine("    MaximumPacketSize = 0x%04x", pPipe->MaximumPacketSize);
                                                AddPropLine("    EndpointAddress   = 0x%02x", pPipe->EndpointAddress);
                                                AddPropLine("    Interval          = 0x%02x", pPipe->Interval);
                                                AddPropLine("    PipeType          = 0x%02x", pPipe->PipeType);
                                                if(pPipe->PipeType == UsbdPipeTypeControl)
                                                    AddPropLine("      UsbdPipeTypeControl");
                                                else if(pPipe->PipeType == UsbdPipeTypeIsochronous)
                                                    AddPropLine("      UsbdPipeTypeIsochronous");
                                                else if(pPipe->PipeType == UsbdPipeTypeBulk)
                                                    AddPropLine("      UsbdPipeTypeBulk");
                                                else if(pPipe->PipeType == UsbdPipeTypeInterrupt)
                                                    AddPropLine("      UsbdPipeTypeInterrupt");
                                                else
                                                    AddPropLine("      !!! INVALID !!!");
                                                AddPropLine("    PipeHandle        = 0x%08x", pPipe->PipeHandle);
                                                AddPropLine("    MaxTransferSize   = 0x%08x", pPipe->MaximumTransferSize);
                                                AddPropLine("    PipeFlags         = 0x%02x", pPipe->PipeFlags);
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
        AddPropLine("no Configuration Descriptor present => unconfigure device!");
    }
}

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

    AddPropLine("ConfigurationHandle: 0x%08x (%d)", 
        m_pSelectInterface->ConfigurationHandle,
        m_pSelectInterface->ConfigurationHandle);

    AddPropLine("");
    AddPropLine("Interface:");
    PUSBD_INTERFACE_INFORMATION pInterface = &m_pSelectInterface->Interface;
    AddPropLine("  Length: 0x%04x (%d)", 
        pInterface->Length,
        pInterface->Length);
    AddPropLine("  InterfaceNumber: 0x%02x (%d)",
        pInterface->InterfaceNumber,
        pInterface->InterfaceNumber);
    AddPropLine("  AlternateSetting: 0x%02x (%d)",
        pInterface->AlternateSetting,
        pInterface->AlternateSetting);

    if(!IsComingUp())
    {
        return;
    }

    AddPropLine("");
    AddPropLine("Output");
    AddPropLine("  Class: 0x%02x (%d)",
        pInterface->Class,
        pInterface->Class);
    AddPropLine("  SubClass: 0x%02x (%d)",
        pInterface->SubClass,
        pInterface->SubClass);
    AddPropLine("  Protocol: 0x%02x (%d)",
        pInterface->Protocol,
        pInterface->Protocol);
    AddPropLine("  Reserved: 0x%02x",
        pInterface->Reserved);

    AddPropLine("  InterfaceHandle: 0x%08x",
        pInterface->InterfaceHandle);
    AddPropLine("  NumberOfPipes: 0x%08x (%d)",
        pInterface->NumberOfPipes,
        pInterface->NumberOfPipes);

    PUSBD_PIPE_INFORMATION pPipe = pInterface->Pipes;
    int nBufferLeft = m_dwSelectInterfaceSize - FIELD_OFFSET(_URB_SELECT_INTERFACE, Interface.Pipes[0]);
    ULONG nPipe = 0;
    while((nPipe < pInterface->NumberOfPipes) && (0 < nBufferLeft))
    {
        AddPropLine("  Pipe[%d]:", nPipe);
        pPipe = &pInterface->Pipes[nPipe];

        // OUTPUT from USBD
        AddPropLine("    MaximumPacketSize: 0x%04x (%d)",
            pPipe->MaximumPacketSize,
            pPipe->MaximumPacketSize);
        AddPropLine("    EndpointAddress: 0x%02x (%d)",
            pPipe->EndpointAddress,
            pPipe->EndpointAddress);
        AddPropLine("    Interval: %d ms",
            pPipe->Interval);
        AddPropLine("    PipeType: 0x%02x",
            pPipe->PipeType);
        switch(pPipe->PipeType)
        {
        case UsbdPipeTypeControl:
            AddPropLine("      UsbdPipeTypeControl");
            break;
        case UsbdPipeTypeIsochronous:
            AddPropLine("      UsbdPipeTypeIsochronous");
            break;
        case UsbdPipeTypeBulk:
            AddPropLine("      UsbdPipeTypeBulk");
            break;
        case UsbdPipeTypeInterrupt:
            AddPropLine("      UsbdPipeTypeInterrupt");
            break;
        default:
            AddPropLine("      invalid!");
        }
        AddPropLine("    PipeHandle: 0x%08x",
            pPipe->PipeHandle);

        // INPUT
        // These fields are filled in by the client driver
        AddPropLine("");
        AddPropLine("    MaximumTransferSize: 0x%08x (%d) bytes",
            pPipe->MaximumTransferSize,
            pPipe->MaximumTransferSize);
        AddPropLine("    PipeFlags: 0x%08x (%d)",
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
    CopyMemory(m_pSelectInterface, pData, m_dwSelectInterfaceSize);
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

    AddPropLine("PipeHandle: %08x", m_ControlTransfer.PipeHandle);
    
    AddPropLine("");
    AddPropLine("SetupPacket:");
    GenerateHexDump(m_ControlTransfer.SetupPacket, sizeof(m_ControlTransfer.SetupPacket));

    UCHAR bmRequestType = m_ControlTransfer.SetupPacket[0];
    UCHAR bRequest = m_ControlTransfer.SetupPacket[1];
    USHORT wValue = (USHORT)(m_ControlTransfer.SetupPacket[3] + 256 * m_ControlTransfer.SetupPacket[2]);
    //USHORT wIndex = (USHORT)(m_ControlTransfer.SetupPacket[5] + 256 * m_ControlTransfer.SetupPacket[4]);
    //USHORT wLength = (USHORT)(m_ControlTransfer.SetupPacket[7] + 256 * m_ControlTransfer.SetupPacket[6]);

    AddPropLine("bmRequestType: %02x", bmRequestType);
    AddPropLine("  DIR: %s", USB_ENDPOINT_DIRECTION_IN(bmRequestType) ?
        "Device-To-Host" : "Host-To-Device");

    BOOL bIsTypeStandard = FALSE;
    switch((bmRequestType & 0x60) >> 5)
    {
    case 0:
        AddPropLine("  TYPE: Standard");
        bIsTypeStandard = TRUE;
        break;
    case 1:
        AddPropLine("  TYPE: Class");
        break;
    case 2:
        AddPropLine("  TYPE: Vendor");
        break;
    case 3:
        AddPropLine("  TYPE: Reserved");
        break;
    default:
        ASSERT(FALSE);
        break;
    }

    BOOL bIsRecipientDevice = FALSE;
    switch(bmRequestType & 0x1f)
    {
    case 0:
        AddPropLine("  RECIPIENT: Device");
        bIsRecipientDevice = TRUE;
        break;
    case 1:
        AddPropLine("  RECIPIENT: Interface");
        break;
    case 2:
        AddPropLine("  RECIPIENT: Endpoint");
        break;
    case 3:
        AddPropLine("  RECIPIENT: Other");
        break;
    default:
        AddPropLine("  RECIPIENT: %d => 4..31 == Reserved", bmRequestType & 0x1f);
        break;
    }

    AddPropLine("bRequest: %02x  ", bRequest);
    
    BOOL bIsReqGetDescriptor = FALSE;
    if(bIsTypeStandard)
    {
        switch(bRequest)
        {
        case USB_REQUEST_GET_STATUS:
            AddPropLine("  GET_STATUS");
            break;
        case USB_REQUEST_CLEAR_FEATURE:
            AddPropLine("  CLEAR_FEATURE");
            break;
        case 2:
        case 4:
            AddPropLine("  reserved for future use!!");
            break;
        case USB_REQUEST_SET_FEATURE:
            AddPropLine("  SET_FEATURE");
            break;
        case USB_REQUEST_SET_ADDRESS:
            AddPropLine("  SET_ADDRESS");
            break;
        case USB_REQUEST_GET_DESCRIPTOR:
            AddPropLine("  GET_DESCRIPTOR");
            bIsReqGetDescriptor = TRUE;
            break;
        case USB_REQUEST_SET_DESCRIPTOR:
            AddPropLine("  SET_DESCRIPTOR");
            break;
        case USB_REQUEST_GET_CONFIGURATION:
            AddPropLine("  GET_CONFIGURATION");
            break;
        case USB_REQUEST_SET_CONFIGURATION:
            AddPropLine("  SET_CONFIGURATION");
            break;
        case USB_REQUEST_GET_INTERFACE:
            AddPropLine("  GET_INTERFACE");
            break;
        case USB_REQUEST_SET_INTERFACE:
            AddPropLine("  SET_INTERFACE");
            break;
        case USB_REQUEST_SYNC_FRAME:
            AddPropLine("  SYNCH_FRAME");
            break;
        default:
            AddPropLine("  unknown!");
            break;
        }
    }
    
    BOOL bIsDeviceDescriptorType = FALSE;
    BOOL bIsConfigurationDescriptorType = FALSE;
    if(bIsReqGetDescriptor)
    {
        AddPropLine("Descriptor Type: 0x%04x", wValue);
        switch(wValue)
        {
        case USB_DEVICE_DESCRIPTOR_TYPE:
            AddPropLine("  DEVICE");
            bIsDeviceDescriptorType = TRUE;
            break;
            
        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
            AddPropLine("  CONFIGURATION");
            bIsConfigurationDescriptorType = TRUE;
            break;
            
        case USB_STRING_DESCRIPTOR_TYPE:
            AddPropLine("  STRING");
            break;
            
        case USB_INTERFACE_DESCRIPTOR_TYPE:
            AddPropLine("  INTERFACE");
            break;
            
        case USB_ENDPOINT_DESCRIPTOR_TYPE:
            AddPropLine("  ENDPOINT");
            break;
            
        default:
            AddPropLine("  unknown");
            break;
        }
    }
    AddPropLine("");
    
    CURB_TransferBuffer::RenderProperties();

    if(m_TransferBuffer)
    {
        if(bIsDeviceDescriptorType)
        {
            USB_DEVICE_DESCRIPTOR *pDesc = (USB_DEVICE_DESCRIPTOR*) m_TransferBuffer;
            AddPropLine("    bLength            : 0x%02x (%d)", pDesc->bLength, pDesc->bLength);
            AddPropLine("    bDescriptorType    : 0x%02x (%d)", pDesc->bDescriptorType, pDesc->bDescriptorType);
            AddPropLine("    bcdUSB             : 0x%04x (%d)", pDesc->bcdUSB, pDesc->bcdUSB);
            AddPropLine("    bDeviceClass       : 0x%02x (%d)", pDesc->bDeviceClass, pDesc->bDeviceClass);
            AddPropLine("    bDeviceSubClass    : 0x%02x (%d)", pDesc->bDeviceSubClass, pDesc->bDeviceSubClass);
            AddPropLine("    bDeviceProtocol    : 0x%02x (%d)", pDesc->bDeviceProtocol, pDesc->bDeviceProtocol);
            AddPropLine("    bMaxPacketSize0    : 0x%02x (%d)", pDesc->bMaxPacketSize0, pDesc->bMaxPacketSize0);
            AddPropLine("    idVendor           : 0x%04x (%d)", pDesc->idVendor, pDesc->idVendor);
            AddPropLine("    idProduct          : 0x%04x (%d)", pDesc->idProduct, pDesc->idProduct);
            AddPropLine("    bcdDevice          : 0x%04x (%d)", pDesc->bcdDevice, pDesc->bcdDevice);
            AddPropLine("    iManufacturer      : 0x%02x (%d)", pDesc->iManufacturer, pDesc->iManufacturer);
            AddPropLine("    iProduct           : 0x%02x (%d)", pDesc->iProduct, pDesc->iProduct);
            AddPropLine("    iSerialNumber      : 0x%02x (%d)", pDesc->iSerialNumber, pDesc->iSerialNumber);
            AddPropLine("    bNumConfigurations : 0x%02x (%d)", pDesc->bNumConfigurations, pDesc->bNumConfigurations);
        }
        
        if(bIsConfigurationDescriptorType)
        {
            USB_CONFIGURATION_DESCRIPTOR *pDesc = (USB_CONFIGURATION_DESCRIPTOR*) m_TransferBuffer;
            AddPropLine("    bLength            : 0x%02x (%d)", pDesc->bLength, pDesc->bLength);
            AddPropLine("    bDescriptorType    : 0x%02x (%d)", pDesc->bDescriptorType, pDesc->bDescriptorType);
            AddPropLine("    wTotalLength       : 0x%04x (%d)", pDesc->wTotalLength, pDesc->wTotalLength);
            AddPropLine("    bNumInterfaces     : 0x%02x (%d)", pDesc->bNumInterfaces, pDesc->bNumInterfaces);
            AddPropLine("    bConfigurationValue: 0x%02x (%d)", pDesc->bConfigurationValue, pDesc->bConfigurationValue);
            AddPropLine("    iConfiguration     : 0x%02x (%d)", pDesc->iConfiguration, pDesc->iConfiguration);
            AddPropLine("    bmAttributes       : 0x%02x (%d)", pDesc->bmAttributes, pDesc->bmAttributes);
            AddPropLine("    MaxPower           : 0x%02x (%d)", pDesc->MaxPower, pDesc->MaxPower);
        }
    }
}

void CURB_ControlTransfer::GrabData(PPACKET_HEADER ph)
{
    CURB::GrabData(ph);
    
    PUCHAR pData = (PUCHAR)(&ph->UrbHeader);
    CopyMemory(&m_ControlTransfer, pData, sizeof(m_ControlTransfer));
    pData += m_ControlTransfer.Hdr.Length;
    GrabTransferBuffer(pData, m_ControlTransfer.TransferBufferLength);
    
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
    
    AddPropLine("TransferFlags: 0x%08x", m_BulkOrInterruptTransfer.TransferFlags);

    CURB_TransferBuffer::RenderProperties();
}

void CURB_BulkOrInterruptTransfer::GrabData(PPACKET_HEADER ph)
{
    CURB::GrabData(ph);
    
    PUCHAR pData = (PUCHAR)(&ph->UrbHeader);
    CopyMemory(&m_BulkOrInterruptTransfer, pData, sizeof(m_BulkOrInterruptTransfer));
    pData += m_BulkOrInterruptTransfer.Hdr.Length;
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

void CURB_IsochTransfer::GetDataDump(LPTSTR sBuffer)
{
    if(0 != m_nTransferBufferCnt)
    {
        GenerateHexDumpLine(sBuffer, m_pTransferBuffer[0], min(8, m_nTransferBufferLength[0]));
    }
    else
    {
        _tcscpy(sBuffer, "-");
    }
}

void CURB_IsochTransfer::RenderProperties(void)
{
    CURB::RenderProperties();

    AddPropLine("NumberOfPackets: 0x%08x (%d)", 
        m_pIsochTransfer->NumberOfPackets, 
        m_pIsochTransfer->NumberOfPackets);

    for(ULONG nPacket = 0; nPacket < m_pIsochTransfer->NumberOfPackets; ++nPacket)
    {
        PUSBD_ISO_PACKET_DESCRIPTOR pPacket = &m_pIsochTransfer->IsoPacket[nPacket];
        AddPropLine("Packet %2d: Ofs: 0x%08x  Len: 0x%08x  Status: 0x%08x",
            nPacket, pPacket->Offset, pPacket->Length, pPacket->Status);
    }

    for(nPacket = 0; nPacket < m_nTransferBufferCnt; ++nPacket)
    {
        AddPropLine("");
        AddPropLine("Buffer: %2d: Len: 0x%08x (%d)", nPacket,
            m_nTransferBufferLength[nPacket], m_nTransferBufferLength[nPacket]);
        GenerateHexDump(m_pTransferBuffer[nPacket], m_nTransferBufferLength[nPacket]);
    }
}

void CURB_IsochTransfer::GrabData(PPACKET_HEADER ph)
{
    CURB::GrabData(ph);
    
    PUCHAR pData = (PUCHAR)(&ph->UrbHeader);
    m_IsochTransferLen = ph->UrbHeader.Length;
    m_pIsochTransfer = (_URB_ISOCH_TRANSFER*) m_pCA->AllocBlock(m_IsochTransferLen);
    CopyMemory(m_pIsochTransfer, pData, m_IsochTransferLen);
    pData += m_IsochTransferLen;
    
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

  $Log: not supported by cvs2svn $
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

