//************************************************************************
//
// RingBuffer.h
//
//************************************************************************

#ifndef _RINGBUFFER_H_INCLUDED_ 
#define _RINGBUFFER_H_INCLUDED_ 

class CRingBuffer
{
public:
    void Initialize(void *pBase, long nBufferSize);
    bool WriteBytes(void *pData, long nBytes);
    bool ReadBytes(void *pData, long nBytes);
    bool PeekBytes(void *pData, long nBytes);
    bool FlushBuffer(void); 

    inline bool GetOverflowOccured(void) { return (0 != m_nOverflowOccurred); }
    inline long GetUsedSize(void) { return m_nBytes; };
    inline long GetFreeSize(void) { return m_nTotalBytes - m_nBytes; };
    inline long GetTotalSize(void) { return m_nTotalBytes; }
    inline void *GetBase(void) { return m_pBase; }

    inline void SetOverflowOccurred(bool bOccurred) { m_nOverflowOccurred = bOccurred ? 1 : 0; }

private:
    unsigned char *m_pBase;
#ifndef _WIN64
    ULONG pack2;
#endif
    long m_nBytes;
    long m_nTotalBytes;
    long m_nInPtr;
    long m_nOutPtr;
    long m_nOverflowOccurred;
};

#endif // !_RINGBUFFER_H_INCLUDED_ 

//** end of RingBuffer.h *************************************************
/*************************************************************************

  $Log: RingBuffer.h,v $
  Revision 1.1  2002/08/14 23:00:58  rbosa
  shared code between the application and driver and drivers themselves...

 * 
 * 1     1/25/02 2:44p Rbosa

*************************************************************************/
