//************************************************************************
//
// MyMemFile.cpp
//
//************************************************************************

#include "StdAfx.h"
#include "MyMemFile.h"

CMyMemFile::CMyMemFile(void) : m_pData(NULL), m_dwLength(0)
{
}

CMyMemFile::CMyMemFile(PVOID pData) : m_pData((PBYTE)pData), m_dwLength(0)
{
}

UINT CMyMemFile::Read(void* lpBuf, UINT nCount)
{
    if(NULL != m_pData)
    {
        memcpy(lpBuf, m_pData, nCount);
        m_pData += nCount;
    }
    m_dwLength += nCount;
    return nCount;
}

void CMyMemFile::Write(const void* lpBuf, UINT nCount)
{
    if(NULL != m_pData)
    {
        memcpy(m_pData, lpBuf, nCount);
        m_pData += nCount;
    }
    m_dwLength += nCount;
}

LONG CMyMemFile::Seek(LONG lOff, UINT nFrom)
{
    switch(nFrom)
    {
    case CFile::begin:
        if(NULL != m_pData)
        {
            m_pData -= m_dwLength;
            m_pData += lOff;
        }
        m_dwLength = lOff;
        break;

    case CFile::current:
        if(NULL != m_pData)
        {
            m_pData += lOff;
        }
        m_dwLength += lOff;
        break;

    case CFile::end:
    default:
        AfxThrowFileException(CFileException::badSeek);
    }
    return m_dwLength;
}

DWORD CMyMemFile::GetLength() const
{
    return m_dwLength;
}

UINT CMyMemFile::GetBufferPtr(UINT nCommand, UINT, void**, void**)
{
    ASSERT(bufferCheck == nCommand);
    UNREFERENCED_PARAMETER(nCommand);
    return 0;
}
    
// all other functions are not supported anymore...

DWORD CMyMemFile::GetPosition() const
{
    AfxThrowNotSupportedException();
    return 0;
}

void CMyMemFile::SetFilePath(LPCTSTR)
{
    AfxThrowNotSupportedException();
}

BOOL CMyMemFile::Open(LPCTSTR, UINT, CFileException*)
{
    AfxThrowNotSupportedException();
    return FALSE;
}

CFile* CMyMemFile::Duplicate() const
{
    AfxThrowNotSupportedException();
    return NULL;
}

void CMyMemFile::LockRange(DWORD, DWORD)
{
    AfxThrowNotSupportedException();
}

void CMyMemFile::UnlockRange(DWORD, DWORD)
{
    AfxThrowNotSupportedException();
}

void CMyMemFile::Abort()
{
    AfxThrowNotSupportedException();
}

void CMyMemFile::Flush()
{
    AfxThrowNotSupportedException();
}

void CMyMemFile::Close()
{
    AfxThrowNotSupportedException();
}

void CMyMemFile::SetLength(DWORD dwNewLen)
{
    UNREFERENCED_PARAMETER(dwNewLen);
    AfxThrowNotSupportedException();
}


//** end of MyMemFile.cpp ************************************************
/*************************************************************************

  $Log: not supported by cvs2svn $
 * 
 * 1     2/05/02 8:53p Rbosa

*************************************************************************/