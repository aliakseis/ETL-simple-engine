
#include "stdafx.h"
#include "DlgCopyDesign.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CDlgCopyDesign dialog


CDlgCopyDesign::CDlgCopyDesign(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCopyDesign::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCopyDesign)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgCopyDesign::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCopyDesign)
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressBar);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCopyDesign, CDialog)
	//{{AFX_MSG_MAP(CDlgCopyDesign)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCopyDesign message handlers



