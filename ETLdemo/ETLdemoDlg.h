// ETLdemoDlg.h : header file
//

#if !defined(AFX_ETLDEMODLG_H__CE2A8647_57A6_11D7_9CE6_D43439106E1D__INCLUDED_)
#define AFX_ETLDEMODLG_H__CE2A8647_57A6_11D7_9CE6_D43439106E1D__INCLUDED_

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CETLdemoDlg dialog

class CETLdemoDlg : public CDialog
{
// Construction
public:
	CETLdemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CETLdemoDlg)
	enum { IDD = IDD_ETLDEMO_DIALOG };
	CString	m_connectStringFrom;
	CString	m_connectStringTo;
	int		m_nCustomersMode;
	int		m_nEmployeesMode;
	BOOL	m_bCustomersOrEmployeesOrders;
	BOOL m_bFastLoad;
	//}}AFX_DATA
	CCheckListBox	m_listCustomers;
	CCheckListBox	m_listEmployees;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CETLdemoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CETLdemoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTransform();
	afx_msg void OnSelectConnectStringFrom();
	afx_msg void OnSelectConnectStringTo();
	afx_msg void OnSelectCustomers();
	afx_msg void OnClearTarget();
	afx_msg void OnQueryCustomers();
	afx_msg void OnAllCustomers();
	afx_msg void OnAllEmployees();
	afx_msg void OnSelectEmployees();
	afx_msg void OnQueryEmployees();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void ReportComError(_com_error& e);
private:
	void UpdateCustomersView();
	void UpdateEmployeesView();
public:
	afx_msg void OnBnClickedTest();
	bool DoTransform(void);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ETLDEMODLG_H__CE2A8647_57A6_11D7_9CE6_D43439106E1D__INCLUDED_)
