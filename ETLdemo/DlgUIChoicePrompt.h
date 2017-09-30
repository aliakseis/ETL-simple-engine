#if !defined(AFX_DLGUICHOICEPROMPT_H__B179FEF5_6719_4928_A59E_E384A7CDDBDE__INCLUDED_)
#define AFX_DLGUICHOICEPROMPT_H__B179FEF5_6719_4928_A59E_E384A7CDDBDE__INCLUDED_

#pragma once
// DlgUIChoicePrompt.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgUIChoicePrompt dialog

class CDlgUIChoicePrompt : public CDialog
{
// Construction
public:
	explicit CDlgUIChoicePrompt(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgUIChoicePrompt)
	enum { IDD = IDD_UI_CHOICE_PROMPT };
	BOOL	m_bApplyToAll;
	CString	m_strStaticText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgUIChoicePrompt)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void EndDialog( UINT nResult );
	void OnOK();
	void OnCancel();

	// Generated message map functions
	//{{AFX_MSG(CDlgUIChoicePrompt)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGUICHOICEPROMPT_H__B179FEF5_6719_4928_A59E_E384A7CDDBDE__INCLUDED_)
