
#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDlgCopyDesign dialog

class CDlgCopyDesign : public CDialog
{
// Construction
public:
	explicit CDlgCopyDesign(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCopyDesign)
	enum { IDD = IDD_CopyDesign };
	CProgressCtrl	m_ProgressBar;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCopyDesign)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCopyDesign)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
