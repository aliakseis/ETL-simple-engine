// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__CE2A8649_57A6_11D7_9CE6_D43439106E1D__INCLUDED_)
#define AFX_STDAFX_H__CE2A8649_57A6_11D7_9CE6_D43439106E1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef STRICT
#define STRICT
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0501		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#pragma warning(disable:4786)

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#import <C:\Program Files\Common Files\System\ado\msado15.dll> no_namespace rename( "EOF", "adoEOF" ) 

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#include <afxtempl.h>			// MFC support for Windows Common Controls

#include <ATLCONV.h>


#endif // !defined(AFX_STDAFX_H__CE2A8649_57A6_11D7_9CE6_D43439106E1D__INCLUDED_)
