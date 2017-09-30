// ETLdemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ETLdemo.h"
#include "ETLdemoDlg.h"


#include "DemoCopyHelper.h"

#include "DlgCopyDesign.h"

#include "DataSourceLocator.h"

#include <sstream>

#include "radix40.h"
#include ".\etldemodlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum { MODE_CUSTOMERS_ALL, MODE_CUSTOMERS_SELECT, MODE_CUSTOMERS_QUERY };
enum { MODE_EMPLOYEES_ALL, MODE_EMPLOYEES_SELECT, MODE_EMPLOYEES_QUERY };

bool GetConnectString(CString& strConnectString, CWnd* pParent)
{
	CDataSourceLocator dlg;
	if(dlg.CreateDispatch(_T("Datalinks")))
	{
		dlg.SetHWnd((long) pParent->GetSafeHwnd());
		LPDISPATCH lpDispatch = dlg.PromptNew();
		if(lpDispatch != NULL)
		{
			_ConnectionPtr ptrConnection = lpDispatch;
			BSTR bstrConnectString = ptrConnection->ConnectionString;
			USES_CONVERSION;
			strConnectString = W2CT(bstrConnectString);
			TRACE("Connect String: %s\n", strConnectString);
			return true;
		}
	}
	return false;
}

const TCHAR g_szConnectStrings[] = _T("Connnect Strings");
const TCHAR g_szSource[] = _T("Source");
const TCHAR g_szDestination[] = _T("Destination");

const TCHAR g_szQueries[] = _T("Queries");
const TCHAR g_szCustomers[] = _T("Customers");
const TCHAR g_szEmployees[] = _T("Employees");

class CProgressBarAdapter : public IProgress
{
public:
    explicit CProgressBarAdapter(CProgressCtrl& rProgress)
		: m_progress(rProgress) {}

	void SetRange(short nLower, short nUpper) { m_progress.SetRange(nLower, nUpper); }

	int SetStep(int nStep)	{ return m_progress.SetStep(nStep); }

	int SetPos(int nPos)	{ return m_progress.SetPos(nPos); }

	int OffsetPos(int nPos)	{ return m_progress.OffsetPos(nPos); }

private:
	CProgressCtrl& m_progress;
};


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CETLdemoDlg dialog

CETLdemoDlg::CETLdemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CETLdemoDlg::IDD, pParent)
	, m_bFastLoad(FALSE)
{
	//{{AFX_DATA_INIT(CETLdemoDlg)
	m_connectStringFrom = _T("");
	m_connectStringTo = _T("");
	m_bCustomersOrEmployeesOrders = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_nCustomersMode = 0;
	m_nEmployeesMode = 0;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CETLdemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CETLdemoDlg)
	DDX_Text(pDX, IDC_CONNECT_STRING_FROM, m_connectStringFrom);
	DDX_Text(pDX, IDC_CONNECT_STRING_TO, m_connectStringTo);
	DDX_Radio(pDX, IDC_ALL_CUSTOMERS, m_nCustomersMode);
	DDX_Radio(pDX, IDC_ALL_EMPLOYEES, m_nEmployeesMode);
	DDX_Check(pDX, IDC_CUSTOMERS_OR_EMPLOYEES_ORDERS, m_bCustomersOrEmployeesOrders);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_LIST_CUSTOMERS, m_listCustomers);
	DDX_Control(pDX, IDC_LIST_EMPLOYEES, m_listEmployees);
	DDX_Check(pDX, IDC_ENABLE_FAST_LOAD, m_bFastLoad);
}

BEGIN_MESSAGE_MAP(CETLdemoDlg, CDialog)
	//{{AFX_MSG_MAP(CETLdemoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_TRANSFORM, OnTransform)
	ON_BN_CLICKED(IDC_BTN_SELECT_CONNECT_STRING_FROM, OnSelectConnectStringFrom)
	ON_BN_CLICKED(IDC_BTN_SELECT_CONNECT_STRING_TO, OnSelectConnectStringTo)
	ON_BN_CLICKED(IDC_SELECT_CUSTOMERS, OnSelectCustomers)
	ON_BN_CLICKED(IDC_CLEAR_TARGET, OnClearTarget)
	ON_BN_CLICKED(IDC_QUERY_CUSTOMERS, OnQueryCustomers)
	ON_BN_CLICKED(IDC_ALL_CUSTOMERS, OnAllCustomers)
	ON_BN_CLICKED(IDC_ALL_EMPLOYEES, OnAllEmployees)
	ON_BN_CLICKED(IDC_SELECT_EMPLOYEES, OnSelectEmployees)
	ON_BN_CLICKED(IDC_QUERY_EMPLOYEES, OnQueryEmployees)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_TEST, OnBnClickedTest)
END_MESSAGE_MAP()

inline LPCTSTR Null2None(LPCTSTR psz)
{
	return psz? psz : _T("<none>");
}

void CETLdemoDlg::ReportComError(_com_error& e)
{
	std::basic_ostringstream<TCHAR> s;
	s //<< _T("Code meaning = ") << e.ErrorMessage() 
	<< _T("Source = ") << Null2None(e.Source())
	<< _T("\nDescription = ") << Null2None(e.Description());
	
	MessageBox(s.str().c_str(), e.ErrorMessage(), MB_OK | MB_ICONEXCLAMATION);
}



/////////////////////////////////////////////////////////////////////////////
// CETLdemoDlg message handlers

BOOL CETLdemoDlg::OnInitDialog()
{
	m_connectStringFrom = AfxGetApp()->GetProfileString(g_szConnectStrings, g_szSource);
	m_connectStringTo = AfxGetApp()->GetProfileString(g_szConnectStrings, g_szDestination);
	
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_EDIT_CUSTOMERS, AfxGetApp()->GetProfileString(g_szQueries, g_szCustomers));
	SetDlgItemText(IDC_EDIT_EMPLOYEES, AfxGetApp()->GetProfileString(g_szQueries, g_szEmployees));

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		if (strAboutMenu.LoadString(IDS_ABOUTBOX) && !strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CETLdemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CETLdemoDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CETLdemoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CETLdemoDlg::OnTransform() 
{
    clock_t start = clock();

	if (DoTransform())
	{
		CString strBuf;
		strBuf.Format(_T("Stuff was copied successfully in %g seconds.")
			, (double)(clock() - start) / CLOCKS_PER_SEC);

		MessageBox(strBuf);
	}
}

void CETLdemoDlg::OnSelectConnectStringFrom() 
{
	CString connectStringFrom = m_connectStringFrom;
	if (GetConnectString(m_connectStringFrom, this)
	&& connectStringFrom != m_connectStringFrom)
	{
		m_nCustomersMode = 0;
		m_listCustomers.ResetContent();
		m_nEmployeesMode = 0;
		m_listEmployees.ResetContent();
		UpdateData(false);
		UpdateCustomersView();
		UpdateEmployeesView();
	}
}

void CETLdemoDlg::OnSelectConnectStringTo() 
{
	if (GetConnectString(m_connectStringTo, this))
		UpdateData(false);
}

void CETLdemoDlg::OnSelectCustomers() 
{
	UpdateCustomersView();

	if (m_listCustomers.GetCount() == 0)
	{
		CWaitCursor waitCursor;
		try 
		{
			_RecordsetPtr pRSet;
			if(FAILED(pRSet.CreateInstance(__uuidof(Recordset))))
			{
				ASSERT(0);
				return;
			}
			pRSet->Open(L"SELECT CustomerID, CompanyName, ContactName FROM CUSTOMERS ORDER BY CustomerID, CompanyName", 
				(LPCTSTR) m_connectStringFrom, 
				adOpenForwardOnly, adLockReadOnly, adCmdText);
			
			for (; !pRSet->adoEOF; pRSet->MoveNext())
			{
				CString item((LPCTSTR) (_bstr_t) pRSet->Fields->Item[1L]->Value);
				item += _T(" / ");
				item += (LPCTSTR) (_bstr_t) pRSet->Fields->Item[2L]->Value;

				DWORD id = 0;
				VERIFY(S_OKAY == ascii_to_radix40((Radix40*)&id, pRSet->Fields->Item[0L]->Value.bstrVal, 6));
				int idx = m_listCustomers.AddString(item);
				m_listCustomers.SetItemData(idx, id);
			}

		}
		catch (_com_error& e)
		{
			ReportComError(e);
		}
	}
}


static LPCWSTR g_arrTables[] =
{
	L"Order Details",
	L"Orders",
	L"Products",
	L"Categories",
	L"Customers",
	L"Employees",
	L"Shippers",
	L"Suppliers",
};

void CETLdemoDlg::OnClearTarget() 
{
	if (!m_connectStringTo.CompareNoCase(m_connectStringFrom))
	{
		MessageBox(_T("Source and destination are the same"), NULL, MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	
	CWaitCursor waitCursor;
	try 
	{
		_ConnectionPtr connectionTo;
		VERIFY(SUCCEEDED(connectionTo.CreateInstance(__uuidof(Connection))));
		connectionTo->Open((LPCTSTR) m_connectStringTo
			, _bstr_t(), _bstr_t(), adConnectUnspecified);

		for (int i = 0; i < sizeof(g_arrTables)/sizeof(g_arrTables[0]); i++)
		{
			wstring query(L"DELETE FROM [");
			query += g_arrTables[i]; 
			query += L']';
			connectionTo->Execute(query.c_str(), NULL, adExecuteNoRecords);
		}

		connectionTo->Close();
	}
	catch (_com_error& e)
	{
		ReportComError(e);
	}
}

void CETLdemoDlg::OnQueryCustomers() 
{
	UpdateCustomersView();
}

void CETLdemoDlg::OnAllCustomers() 
{
	UpdateCustomersView();
}

void CETLdemoDlg::UpdateCustomersView()
{
	UpdateData();
	GetDlgItem(IDC_LIST_CUSTOMERS)->ShowWindow(
		(1 == m_nCustomersMode)? SW_SHOWNA : SW_HIDE);
	GetDlgItem(IDC_STATIC_CUSTOMERS)->ShowWindow(
		(2 == m_nCustomersMode)? SW_SHOWNA : SW_HIDE);
	GetDlgItem(IDC_EDIT_CUSTOMERS)->ShowWindow(
		(2 == m_nCustomersMode)? SW_SHOWNA : SW_HIDE);
}

void CETLdemoDlg::UpdateEmployeesView()
{
	UpdateData();
	GetDlgItem(IDC_LIST_EMPLOYEES)->ShowWindow(
		(1 == m_nEmployeesMode)? SW_SHOWNA : SW_HIDE);
	GetDlgItem(IDC_STATIC_EMPLOYEES)->ShowWindow(
		(2 == m_nEmployeesMode)? SW_SHOWNA : SW_HIDE);
	GetDlgItem(IDC_EDIT_EMPLOYEES)->ShowWindow(
		(2 == m_nEmployeesMode)? SW_SHOWNA : SW_HIDE);
}

void CETLdemoDlg::OnAllEmployees() 
{
	UpdateEmployeesView();
}

void CETLdemoDlg::OnSelectEmployees() 
{
	UpdateEmployeesView();

	if (m_listEmployees.GetCount() == 0)
	{
		CWaitCursor waitCursor;
		try 
		{
			_RecordsetPtr pRSet;
			if(FAILED(pRSet.CreateInstance(__uuidof(Recordset))))
			{
				ASSERT(0);
				return;
			}
			pRSet->Open(L"SELECT EmployeeID, LastName, FirstName FROM EMPLOYEES ORDER BY LastName, FirstName", 
				(LPCTSTR) m_connectStringFrom, 
				adOpenForwardOnly, adLockReadOnly, adCmdText);
			
			for (; !pRSet->adoEOF; pRSet->MoveNext())
			{
				CString item((LPCTSTR) (_bstr_t) pRSet->Fields->Item[1L]->Value);
				item += _T(", ");
				item += (LPCTSTR) (_bstr_t) pRSet->Fields->Item[2L]->Value;

				int idx = m_listEmployees.AddString(item);
				m_listEmployees.SetItemData(idx, (long) pRSet->Fields->Item[0L]->Value);
			}

		}
		catch (_com_error& e)
		{
			ReportComError(e);
		}
	}
}

void CETLdemoDlg::OnQueryEmployees() 
{
	UpdateEmployeesView();
}

void CETLdemoDlg::OnBnClickedTest()
{
    clock_t start = clock();

	for (int i = 0; i < 10; ++i)
	{
		OnClearTarget();
		if (!DoTransform())
			return;
	}
	CString strBuf;
	strBuf.Format(_T("Stuff was tested successfully in %g seconds.")
		, (double)(clock() - start) / CLOCKS_PER_SEC);

	MessageBox(strBuf);
}

bool CETLdemoDlg::DoTransform()
{
	UpdateData();

	m_connectStringFrom.TrimRight(); m_connectStringFrom.TrimLeft();
	m_connectStringTo.TrimRight(); m_connectStringTo.TrimLeft();

	if (m_connectStringFrom.IsEmpty())
	{
		MessageBox(_T("Source connection string is empty"), NULL, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	if (m_connectStringTo.IsEmpty())
	{
		MessageBox(_T("Destination connection string is empty"), NULL, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	if (!m_connectStringTo.CompareNoCase(m_connectStringFrom))
	{
		MessageBox(_T("Source and destination are the same"), NULL, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	bool bOK = false;

	try 
	{
		_ConnectionPtr connectionFrom;
		VERIFY(SUCCEEDED(connectionFrom.CreateInstance(__uuidof(Connection))));
		connectionFrom->Open((LPCTSTR) m_connectStringFrom
			, _bstr_t(), _bstr_t(), adConnectUnspecified);

		_ConnectionPtr connectionTo;
		VERIFY(SUCCEEDED(connectionTo.CreateInstance(__uuidof(Connection))));
		connectionTo->Open((LPCTSTR) m_connectStringTo
			, _bstr_t(), _bstr_t(), adConnectUnspecified);

		AfxGetApp()->WriteProfileString(g_szConnectStrings, g_szSource, m_connectStringFrom);
		AfxGetApp()->WriteProfileString(g_szConnectStrings, g_szDestination, m_connectStringTo);

		auto DBProTableHolderFrom = std::make_shared<CTableHolder>();
		DBProTableHolderFrom->SetDBManager(connectionFrom);
        auto DBProTableHolderTo = std::make_shared<CTableHolder>();
		DBProTableHolderTo->SetDBManager(connectionTo);

		CDemoCopyHelper CopyHelper;
		CopyHelper.SetDataSources(DBProTableHolderTo, DBProTableHolderFrom);

		DWORD fltCustomers = fltNoFilter;
		CCopyIterator iterCustomers;
		std::deque<Identity> arrCustomers;
		CString queryCustomers;
		GetDlgItemText(IDC_EDIT_CUSTOMERS, queryCustomers);

		switch (m_nCustomersMode)
		{
		case MODE_CUSTOMERS_SELECT:
			fltCustomers = fltPrimaryKey;

			for (int i = m_listCustomers.GetCount(); i--; )
				if (m_listCustomers.GetCheck(i))
				{
					DWORD id = m_listCustomers.GetItemData(i);
					WCHAR buf[7];
					radix40_to_ascii(buf, (Radix40*)&id, 6);
					arrCustomers.push_back(DBProTableHolderFrom->GetIdentity(buf));
				}

			iterCustomers.SetData(&arrCustomers);
			break;
		case MODE_CUSTOMERS_QUERY:
			fltCustomers = (DWORD) (LPCTSTR) queryCustomers;
			break;
		}

		DWORD fltEmployees = fltNoFilter;
		CCopyIterator iterEmployees;
		std::deque<Identity> arrEmployees;
		CString queryEmployees;
		GetDlgItemText(IDC_EDIT_EMPLOYEES, queryEmployees);

		switch (m_nEmployeesMode)
		{
		case MODE_EMPLOYEES_SELECT:
			fltEmployees = fltPrimaryKey;

			for (int i = m_listEmployees.GetCount(); i--; )
				if (m_listEmployees.GetCheck(i))
				{
					arrEmployees.push_back(Identity(m_listEmployees.GetItemData(i)));
				}

			iterEmployees.SetData(&arrEmployees);
			break;
		case MODE_EMPLOYEES_QUERY:
			fltEmployees = (DWORD) (LPCTSTR) queryEmployees;
			break;
		}

		CopyHelper.Init(iterCustomers, fltCustomers,
			iterEmployees, fltEmployees, !!m_bCustomersOrEmployeesOrders);

		CopyHelper.SetFastLoad(!!m_bFastLoad);

		CDlgCopyDesign	dlg;
		dlg.Create(IDD_CopyDesign, this);
		dlg.CenterWindow();
		dlg.ShowWindow(SW_SHOW);

		CProgressBarAdapter progressBarAdapter(dlg.m_ProgressBar);

		CWaitCursor waitCursor;

		if(CopyHelper.CopyTables(&progressBarAdapter))
		{
			bOK = true;

			AfxGetApp()->WriteProfileString(g_szQueries, g_szCustomers, queryCustomers);
			AfxGetApp()->WriteProfileString(g_szQueries, g_szEmployees, queryEmployees);		
		}

		connectionFrom->Close();
		connectionTo->Close();

	}
	catch (_com_error& e)
	{
		ReportComError(e);
		return false;
	}
	return bOK;
}
