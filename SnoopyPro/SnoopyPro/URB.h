// URB.h: interface for the CURB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_URB_H__4555B9EA_0EE6_11D4_A755_00A0C9971EFC__INCLUDED_)
#define AFX_URB_H__4555B9EA_0EE6_11D4_A755_00A0C9971EFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//////////////////////////////////////////////////////////////////////////
// CChunkAllocator

class CChunkAllocator
{
public:
    CChunkAllocator(void);
    virtual ~CChunkAllocator(void);

    // allocates from the pool managed below, usually this memory
    // is not freed unless the whole array goes out of scope...
    LPCTSTR AllocLine(LPCTSTR sLine, int nLen);

    // allocates from the pool managed below... freed when array
    // goes out of scope...
    PVOID AllocBlock(int nLen);


    static const int LINEMEMORY_CHUNKSIZE;
    LPTSTR m_CurrentChunk;
    int m_nCurrentOffset;
    CList<LPTSTR, LPTSTR> m_LineMemory;
};

//////////////////////////////////////////////////////////////////////////
// CURB

class CArrayURB;

#define URBFLAGS_ENDPOINTKNOWN                  (0x00000001)
#define URBFLAGS_PIPEHANDLE_PRESENT             (0x00000002)
#define URBFLAGS_DIRECTION_IN                   (0x00000004)
#define URBFLAGS_DIRECTION_OUT                  (0x00000008)
#define URBFLAGS_COMING_UP                      (0x00000010)

#define MAX_LINES_PER_URB_LOG2                  (10)
#define MAX_LINES_PER_URB                       ((1 << MAX_LINES_PER_URB_LOG2) - 2)

#define MAX_USB_TRANSFERBUFFER_SIZE             (1024)

class CURB : public CObject
{
public:
	CURB(void);
	virtual ~CURB();

    // load/store
    DECLARE_SERIAL(CURB);
    virtual void Serialize(CArchive &ar);

    // access methods
    void GetSequenceNo(LPTSTR sBuffer);
    DWORD GetSequenceNr(void);
    int GetDirection(void);
    void GetDirectionStr(LPTSTR sBuffer);
    int GetEndpoint(void);
    void GetEndpointStr(LPTSTR sBuffer);
    DWORD GetTime(CArrayURB *parURB);
    USHORT GetFunction(void);
    LPCTSTR GetFunctionStr(void);
    LONG GetStatus(void);
    void GetStatusStr(LPTSTR sBuffer);
    DWORD GetLinkNo(void);
    virtual int GetPayloadCount(void);
    virtual PUCHAR GetPayload(void);
    virtual LPCTSTR GetPayloadXML(LPTSTR sBuffer);
    virtual void GetDataDumpStr(LPTSTR sBuffer);

    // for detailed output
    void SetChunkAllocator(CChunkAllocator *pChunkAllocator);
    BOOL IsExpanded(void);
    void SetExpanded(BOOL bExpanded);
    int GetExpandSize(void);
    LPCTSTR GetExpandedLine(int nLine);

    // general filter methods
    BOOL IsSelectURB(void);
    BOOL IsControlURB(void);
    BOOL IsBulkURB(void);
    virtual BOOL HasNoPayload(void);
    BOOL IsDirectionIn(void);
    BOOL IsDirectionOut(void);
    BOOL IsDirectionKnown(void);
    void SetDirection(BOOL bIsIn, BOOL bIsOut);
    BOOL IsComingUp(void);
    BOOL IsPipeHandleAvailable(void);

	// inspection
    void AddPropLine(LPCTSTR sFormat, ...);
	virtual void RenderProperties(void);

    void GenerateHexDumpLine(LPTSTR pOutput, PVOID pBuffer, int nSize);
    void GenerateHexDump(PVOID pBuffer, int nSize);

    // data acquiring
    virtual void GrabData(PPACKET_HEADER ph);
    void GrabBytes(PUCHAR &pData, PVOID pBuffer, ULONG uSize);

    // post-processing
    DWORD GetPipeHandle(void);
    void SetPipeHandle(PVOID PipeHandle);
    void SetEndpoint(UCHAR cEndpoint);

protected:
    CChunkAllocator *m_pCA;

	PACKET_HEADER m_PH;

    DWORD m_dwSequence;
    DWORD m_dwTime;
    DWORD m_dwPipeHandle;
    UCHAR m_cEndpoint;
    USHORT m_nFunction;
    LONG m_lStatus;
    DWORD m_dwLink;

    DWORD m_dwFlags;

    // Expanded caching mechanism
    BOOL m_bExpanded;
    int m_nExpandedSize;
    CArray<LPCTSTR, LPCTSTR> m_arRenderedLines;
};


//////////////////////////////////////////////////////////////////////////
// CURB_TransferBuffer

class CURB_TransferBuffer : public CURB
{
public:
    CURB_TransferBuffer(void);
    virtual ~CURB_TransferBuffer(void);

    // load/store
    DECLARE_SERIAL(CURB_TransferBuffer);
    virtual void Serialize(CArchive &ar);
    
    // access member
    virtual int GetPayloadCount(void);
    virtual PUCHAR GetPayload(void);
    virtual LPCTSTR GetPayloadXML(LPTSTR sBuffer);
    virtual void GetDataDumpStr(LPTSTR sBuffer);
    
    // filter methods
    virtual BOOL HasNoPayload(void);
    
    // inspection
    virtual void RenderProperties(void);
    
    // data acquiring
    void GrabTransferBuffer(PUCHAR pData, int nLength);
    
protected:
    int m_TransferLength;
    PUCHAR m_TransferBuffer;
};


//////////////////////////////////////////////////////////////////////////
// CURB_SelectConfiguration

class CURB_SelectConfiguration : public CURB
{
public:
    CURB_SelectConfiguration(void);
    virtual ~CURB_SelectConfiguration(void);
    
    // load/store
    DECLARE_SERIAL(CURB_SelectConfiguration);
    virtual void Serialize(CArchive &ar);
    
    virtual void RenderProperties(void);
    
    virtual void GrabData(PPACKET_HEADER ph);
    
    // for analyzing
    DWORD GetInterfaceCountFromConfiguration(void);
    DWORD GetPipeCountFromConfiguration(DWORD dwInterface);
    DWORD GetPipeHandleFromConfiguration(DWORD dwInterface, DWORD dwPipe);
    UCHAR GetEndpointFromConfiguration(DWORD dwInterface, DWORD dwPipe);

protected:
    _URB_SELECT_CONFIGURATION m_SelectConfiguration;
	BOOL m_UCDPresent;
	USB_CONFIGURATION_DESCRIPTOR m_UCD;
    ULONG m_uNumInterfaces;
#define MAX_INTERFACES      (256)
    PUSBD_INTERFACE_INFORMATION m_pInterfaces[MAX_INTERFACES];
};


//////////////////////////////////////////////////////////////////////////
// CURB_SelectInterface

class CURB_SelectInterface : public CURB
{
public:
    CURB_SelectInterface(void);
    virtual ~CURB_SelectInterface(void);

    // load/store
    DECLARE_SERIAL(CURB_SelectInterface);
    virtual void Serialize(CArchive &ar);

    virtual void RenderProperties(void);

    virtual void GrabData(PPACKET_HEADER ph);

    // for analyzing
    DWORD GetPipeCountFromInterface(void);
    DWORD GetPipeHandleFromInterface(DWORD dwPipe);
    UCHAR GetEndpointFromInterface(DWORD dwPipe);

protected:
    ULONG m_dwSelectInterfaceSize;
    _URB_SELECT_INTERFACE *m_pSelectInterface;
};


//////////////////////////////////////////////////////////////////////////
// CURB_ControlTransfer

class CURB_ControlTransfer : public CURB_TransferBuffer
{
public:
    CURB_ControlTransfer(void);

    // load/store
    DECLARE_SERIAL(CURB_ControlTransfer);
    virtual void Serialize(CArchive &ar);

    virtual void RenderProperties(void);

    virtual void GrabData(PPACKET_HEADER ph);

protected:
    _URB_CONTROL_TRANSFER m_ControlTransfer;
};


//////////////////////////////////////////////////////////////////////////
// CURB_BulkOrInterruptTransfer

class CURB_BulkOrInterruptTransfer : public CURB_TransferBuffer
{
public:
    CURB_BulkOrInterruptTransfer(void);

    // load/store
    DECLARE_SERIAL(CURB_BulkOrInterruptTransfer);
    virtual void Serialize(CArchive &ar);

    virtual void RenderProperties(void);

    virtual void GrabData(PPACKET_HEADER ph);

protected:
    _URB_BULK_OR_INTERRUPT_TRANSFER m_BulkOrInterruptTransfer;
};


//////////////////////////////////////////////////////////////////////////
// CURB_IsochTransfer

class CURB_IsochTransfer : public CURB
{
public:
    CURB_IsochTransfer(void);
    virtual ~CURB_IsochTransfer(void);

    // load/store
    DECLARE_SERIAL(CURB_IsochTransfer);
    virtual void Serialize(CArchive &ar);
    
    virtual void GetDataDumpStr(LPTSTR sBuffer);
    virtual void RenderProperties(void);

    virtual void GrabData(PPACKET_HEADER ph);

protected:
    ULONG m_IsochTransferLen;
    _URB_ISOCH_TRANSFER *m_pIsochTransfer;

#define ISOCH_MAXPACKETS        (256)
    ULONG m_nTransferBufferCnt;
    PUCHAR m_pTransferBuffer[ISOCH_MAXPACKETS];
    ULONG m_nTransferBufferLength[ISOCH_MAXPACKETS];
};


//////////////////////////////////////////////////////////////////////////
// CArrayURB
//
// Encapsulates also some rather fast memory allocation for rendering
// the URB's to the screen...

class CArrayURB : public CArray<CURB*, CURB*>
{
public:
    CArrayURB(void);
    virtual ~CArrayURB(void);

    virtual void Serialize(CArchive &ar);

    BOOL UnserializeURB(PPACKET_HEADER ph);

    void SetTimeFormat(BOOL bRelative);
    BOOL IsTimeFormatRelative(void);
    void SetTimeStampZero(ULONG uTimeStampZero);
    ULONG GetTimeStampZero(void);

    int FindBySequenceAndDirection(DWORD dwSequenceNr, BOOL bIsComingUp);

public:
    CChunkAllocator m_ChunkAllocator;
    DWORD m_dwPlugTimeStamp;
    BOOL m_bTimeIsRelative;
};

//////////////////////////////////////////////////////////////////////////
// CMyDWORDArray
//
// Same as CDWORDArray, but allows to remove duplicates entries

class CMyDWORDArray : public CDWordArray
{
public:
    void RemoveDuplicates(void);
};


#endif // !defined(AFX_URB_H__4555B9EA_0EE6_11D4_A755_00A0C9971EFC__INCLUDED_)
