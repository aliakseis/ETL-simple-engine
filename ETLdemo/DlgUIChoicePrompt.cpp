// DlgUIChoicePrompt.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DlgUIChoicePrompt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgUIChoicePrompt dialog


CDlgUIChoicePrompt::CDlgUIChoicePrompt(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgUIChoicePrompt::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgUIChoicePrompt)
	m_bApplyToAll = FALSE;
	m_strStaticText = _T("");
	//}}AFX_DATA_INIT
}


void CDlgUIChoicePrompt::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgUIChoicePrompt)
	DDX_Check(pDX, IDC_APPLY_TO_ALL, m_bApplyToAll);
	DDX_Text(pDX, IDC_STATIC_TEXT, m_strStaticText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgUIChoicePrompt, CDialog)
	//{{AFX_MSG_MAP(CDlgUIChoicePrompt)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(BN_CLICKED, 
		min(min(IDC_USE_EXISTING, IDC_CREATE_NEW), IDC_OVERWRITE), 
		max(max(IDC_USE_EXISTING, IDC_CREATE_NEW), IDC_OVERWRITE), 
		EndDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgUIChoicePrompt message handlers

void CDlgUIChoicePrompt::EndDialog( UINT nResult )
{
	UpdateData();
	CDialog::EndDialog(nResult);
}

void CDlgUIChoicePrompt::OnOK()
{
}

void CDlgUIChoicePrompt::OnCancel()
{
}
