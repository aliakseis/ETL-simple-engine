// ETLdemo.h : main header file for the ETLDEMO application
//

#if !defined(AFX_ETLDEMO_H__CE2A8645_57A6_11D7_9CE6_D43439106E1D__INCLUDED_)
#define AFX_ETLDEMO_H__CE2A8645_57A6_11D7_9CE6_D43439106E1D__INCLUDED_

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CETLdemoApp:
// See ETLdemo.cpp for the implementation of this class
//

class CETLdemoApp : public CWinApp
{
public:
	CETLdemoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CETLdemoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CETLdemoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ETLDEMO_H__CE2A8645_57A6_11D7_9CE6_D43439106E1D__INCLUDED_)
