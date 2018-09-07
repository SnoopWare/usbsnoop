//************************************************************************
//
// MyMemFile.h
//
//************************************************************************

#ifndef _MYMEMFILE_H_INCLUDED_ 
#define _MYMEMFILE_H_INCLUDED_ 

class CMyMemFile : public CFile
{
public:
    CMyMemFile(void);
    CMyMemFile(PVOID pData);
    
    virtual UINT Read(void* lpBuf, UINT nCount);
    virtual void Write(const void* lpBuf, UINT nCount);
    virtual LONG Seek(LONG lOff, UINT nFrom);
    virtual DWORD GetLength() const;
    
protected:
    PBYTE m_pData;
    DWORD m_dwLength;

    // all other functions are not supported anymore...
public:
    virtual DWORD GetPosition() const;
    virtual void SetFilePath(LPCTSTR);
    
    // Operations
    virtual BOOL Open(LPCTSTR, UINT, CFileException* pError = NULL);
    
    // Overridables
    virtual CFile* Duplicate() const;
    virtual void LockRange(DWORD, DWORD);
    virtual void UnlockRange(DWORD, DWORD);
    virtual void Abort();
    virtual void Flush();
    virtual void Close();
    virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0, 
        void** ppBufStart = NULL, void** ppBufMax = NULL);
    virtual void SetLength(DWORD dwNewLen);
};

#endif // !_MYMEMFILE_H_INCLUDED_ 

//** end of MyMemFile.h **************************************************
/*************************************************************************

  $Log: MyMemFile.h,v $
  Revision 1.1  2002/08/14 23:03:35  rbosa
  the application to capture urbs and display them...

 * 
 * 1     2/05/02 8:52p Rbosa

*************************************************************************/
