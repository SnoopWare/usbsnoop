//************************************************************************
//
// RingBuffer.cpp
//
//************************************************************************

#include "StdDCls.h"

// This is a nice ringbuffer implementation that Tom gave me yesterday...
// thanks!
#include "RingBuffer.h"

// fast MOD operation, doesn't need divide (but is restricted in its generality)
#define MYMOD(x,n) ((x) >= (n) ? ((x)-(n)) : (x))

void CRingBuffer::Initialize(void *pBase, long nBufferSize)
{
    m_nOverflowOccurred = 0;
    m_pBase = (unsigned char *) pBase;
    m_nTotalBytes = (NULL == m_pBase) ? 0 : nBufferSize;
    m_nBytes = 0;
    m_nInPtr = 0;
    m_nOutPtr = 0;
}

bool CRingBuffer::WriteBytes(void *pData, long nBytes)
{
    long p1len, p2len;
DbgPrint("~1 USBSnpys - WriteBytes, nBytes %d, m_nBytes %d\n",nBytes,m_nBytes);
    if(nBytes > GetFreeSize())
    {
DbgPrint("~1 USBSnpys - FreeSize %d, returning false\n",GetFreeSize());
        return false;
    }
    
    p1len = min(nBytes, m_nTotalBytes - m_nInPtr);
    p2len = nBytes - p1len;
    // copy first bit (up to the end of the buffer, or end of source data, whichever comes first)
    RtlCopyMemory(m_pBase + m_nInPtr, pData, p1len);
    if(p2len)
    {
        // if there was an overlap, copy the rest as well
        RtlCopyMemory(m_pBase, (unsigned char *)pData + p1len, p2len);
    }
    m_nInPtr += nBytes;
    m_nInPtr = MYMOD(m_nInPtr, m_nTotalBytes);
    
    InterlockedExchangeAdd(&m_nBytes, nBytes);
DbgPrint("~1 USBSnpys - m_nBytes now %d\n",m_nBytes);
    
    return true;
}

bool CRingBuffer::ReadBytes(void *pData, long nBytes)
{
    long p1len, p2len;
    if(nBytes > m_nBytes)
    {
        // not enough data in buffer
        return false;
    }
    
    p1len = min(nBytes, m_nTotalBytes - m_nOutPtr);
    p2len = nBytes - p1len;
    RtlCopyMemory(pData, m_pBase + m_nOutPtr, p1len);
    if(p2len)
    {
        RtlCopyMemory((unsigned char*)pData + p1len, m_pBase, p2len);
    }

    m_nOutPtr += nBytes;
    m_nOutPtr = MYMOD(m_nOutPtr, m_nTotalBytes);
    
    InterlockedExchangeAdd(&m_nBytes, -nBytes);

    return true;
}

bool CRingBuffer::PeekBytes(void *pData, long nBytes)
{
    long p1len, p2len;
DbgPrint("~1 USBSnpys - PeekBytes, nBytes %d, m_nBytes %d\n",nBytes,m_nBytes);
    if(nBytes > m_nBytes)
    {
        // not enough data in buffer
        return false;
    }
    
    p1len = min(nBytes, m_nTotalBytes - m_nOutPtr);
    p2len = nBytes - p1len;
    RtlCopyMemory(pData, m_pBase + m_nOutPtr, p1len);
    if(p2len)
    {
        RtlCopyMemory((unsigned char*)pData + p1len, m_pBase, p2len);
    }

    return true;
}

bool CRingBuffer::FlushBuffer(void)
{
    // EnterAccess();
    m_nBytes = 0;
    m_nInPtr = 0;
    m_nOutPtr = 0;
    // LeaveAccess();
    return true;
}

//** end of RingBuffer.cpp ***********************************************
/*************************************************************************

  $Log: RingBuffer.cpp,v $
  Revision 1.1  2002/08/14 23:00:58  rbosa
  shared code between the application and driver and drivers themselves...

 * 
 * 1     1/25/02 2:44p Rbosa

*************************************************************************/
