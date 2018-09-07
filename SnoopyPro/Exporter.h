//************************************************************************
//
// Exporter.h
//
//************************************************************************

#ifndef _EXPORTER_H_INCLUDED_ 
#define _EXPORTER_H_INCLUDED_ 

#include "URB.h"

// the following exporters are defined:
// * plain text (top line only)
// * plain text (expanded view)
// * custom (whith the checkboxes deciding)
// * xml output

typedef enum EXPORTER_TYPE
{
    EXPTYPE_FIRST,

    EXPTYPE_PLAIN_CONDENSED = EXPTYPE_FIRST,
    EXPTYPE_PLAIN_FULL,
    EXPTYPE_CUSTOM,
    EXPTYPE_XML,

    EXPTYPE_LAST = EXPTYPE_XML
} EXPORTER_TYPE;

// to export the number of the urb
#define EXP_URB_NO                      (0x00000001)
#define EXP_URB_DIR                     (0x00000002)
#define EXP_URB_TIMESTAMP               (0x00000004)

class CExporter
{
public:
    CExporter(void);
    virtual ~CExporter(void);

    // create a specific exporter
    static CExporter* Factory(EXPORTER_TYPE ExporterType);

    // determine to which file the export is written to
    void SetOutputFilename(LPCTSTR sFilename);
    void SetArrayURB(CArrayURB *pAR);
    void Export(void);

    // helpers for exporting
    virtual void OutputLine(LPCTSTR sLine);
    void OutputFormat(LPCTSTR sFormat, ...);

    // to be implemented by the different exporters
    virtual void BeginExport(void);
    virtual void ExportURB(CURB *pURB);
    virtual void EndExport(void);

protected:
    CString m_sFilename;
    CFile m_File;
    CArrayURB *m_pAR;
};


#endif // !_EXPORTER_H_INCLUDED_ 

//** end of Exporter.h ***************************************************
/*************************************************************************

  $Log: Exporter.h,v $
  Revision 1.1  2002/10/05 01:10:43  rbosa
  Added the basic framework for exporting a log into an XML file. The
  output written is fairly poor. This checkin is mainly to get the
  framework in place and get feedback on it.


*************************************************************************/
