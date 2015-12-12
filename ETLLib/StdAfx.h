// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__FFDDDC83_4AE0_11D9_9CE9_444553540000__INCLUDED_)
#define AFX_STDAFX_H__FFDDDC83_4AE0_11D9_9CE9_444553540000__INCLUDED_

#pragma once


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#pragma warning(disable:4275)  // deriving exported class from non-exported
#pragma warning(disable:4251)  // using non-exported as public in exported
#pragma warning(disable:4786)

#include <windows.h>

// TODO: reference additional headers your program requires here

#include <atlbase.h>

#define ASSERT		ATLASSERT
#define TRACE		ATLTRACE

#ifdef _DEBUG

#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define VERIFY(f)	ATLASSERT(f)

#else   // _DEBUG

#define VERIFY(f)   ((void)(f))

#endif // !_DEBUG


#import <C:\Program Files\Common Files\System\ado\msado15.dll> no_namespace rename( "EOF", "adoEOF" ) 

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__FFDDDC83_4AE0_11D9_9CE9_444553540000__INCLUDED_)
