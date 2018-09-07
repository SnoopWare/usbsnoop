// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__72E60D9F_34A1_4A21_AC6A_0C22E6775557__INCLUDED_)
#define AFX_STDAFX_H__72E60D9F_34A1_4A21_AC6A_0C22E6775557__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define WINVER      0x0500

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxtempl.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

// the SetupDiXXX api (from the DDK)
#pragma warning(disable:4200)

#include <dbt.h>

#define INITGUID
#include <initguid.h>

#include <setupapi.h>
#include <usbioctl.h>
#include <usbdi.h>

typedef ULONG PDEVICE_OBJECT;

#include "Interface.h"

#pragma warning(default:4200)

#include "malloc.h"


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__72E60D9F_34A1_4A21_AC6A_0C22E6775557__INCLUDED_)
