// DemoCopyHelper.cpp: implementation of the CDemoCopyHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ETLdemo.h"
#include "DemoCopyHelper.h"

#include "Products.h"
#include "Suppliers.h"
#include "Categories.h"
#include "Orders.h"
#include "Customers.h"
#include "Employees.h"
#include "Shippers.h"
#include "OrderDetails.h"

#include "DlgUIChoicePrompt.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


template<> class CTypedOrderVarDesc<CProducts> : public CTypedOrderVariant<CProducts>
{
public:
	CTypedOrderVarDesc() { SetUIChoiceKind(uiAskChoice); }
};

template<> class CTypedOrderVarDesc<CCustomers> : public CTypedOrderVariant<CCustomers>
{
public:
	CTypedOrderVarDesc() 
	{ 
		SetUIChoiceKind(uiAskChoice); 
		SetAbandonDependants();
	}

	void FirstSubType()
	{
		if (!static_cast<CDemoCopyHelper*>(GetTblCopyHelper())
				->IsCustomersOrEmployeesOrders())
			SetConvertOnly();
	}

	bool SetUniqueName()
	{
		CTableHolder tempHolder;
		tempHolder.SetDBManager(GetTblCopyHelper()->GetHolderTo()->GetDBManager());
		CDBTable* pTblTemp = GetCopyTableId().GetDBTable(&tempHolder);

		WCHAR id[6] = {0};
		pTblTemp->CopyDataFromTable(GetTblCopyTo());
		while(pTblTemp->FindFirst(fltUniqueIndex))
		{
			pTblTemp->CopyDataFromTable(GetTblCopyTo());
			for (int i = 0; i < 5; i++)
				id[i] = WCHAR('A' + rand() % ('Z' - 'A' + 1));
			pTblTemp->SetIdentityValue(fltUniqueIndex, 
				tempHolder.GetIdentity(id));
		}
		if(id[0] != 0)
			GetTblCopyTo()->SetIdentityValue(fltUniqueIndex, 
				GetTblCopyHelper()->GetHolderTo()->GetIdentity(id));
		return true;
	}
};

template<> class CTypedOrderVarDesc<CEmployees> : public CTypedOrderVariant<CEmployees>
{
public:
	CTypedOrderVarDesc() 
	{ 
		SetAbandonDependants();
		SetUpdateDestination();
	}
	void FirstSubType()
	{
		if (!static_cast<CDemoCopyHelper*>(GetTblCopyHelper())
				->IsCustomersOrEmployeesOrders())
			SetConvertOnly();
	}
};

template<> class CTypedOrderVarDesc<COrders> : public CTypedOrderVariant<COrders>
{
public:
	bool IsObsoleteByRefs() 
	{
		return ID_NULL == GetTblCopyTo()->GetIdentityValue(COrders::fltCustomerID)
			|| ID_NULL == GetTblCopyTo()->GetIdentityValue(COrders::fltEmployeeID);
	}
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDemoCopyHelper::CDemoCopyHelper()
{
	m_bCustomersOrEmployeesOrders = FALSE;
}

CDemoCopyHelper::~CDemoCopyHelper()
{
}

void CDemoCopyHelper::Init(const CCopyIterator& iterCustomers, DWORD fltCustomers,
		const CCopyIterator& iterEmployees, DWORD fltEmployees, BOOL bCustomersOrEmployeesOrders)
{
	m_bCustomersOrEmployeesOrders = bCustomersOrEmployeesOrders;

	COPY_RLINK(Suppliers, Products, CProducts::fltSupplierID);
	COPY_RLINK(Categories, Products, CProducts::fltCategoryID);

	if (fltCustomers == (fltCustomers & (fltAutoNumber | fltPrimaryKey)))
		COPY_ENTRY_PK(iterCustomers, Customers, fltPrimaryKey);
	else
		COPY_ENTRY(ID_NOT_DEF, ID_NOT_DEF, Customers, 0, fltCustomers);

	COPY_LINK(Customers, Orders, COrders::fltCustomerID, COrders::fltCustomerID);
	COPY_XLINK(Customers, Orders, COrders::fltCustomerID);


	if (fltEmployees == (fltEmployees & (fltAutoNumber | fltPrimaryKey)))
		COPY_ENTRY_PK(iterEmployees, Employees, fltPrimaryKey);
	else
		COPY_ENTRY(ID_NOT_DEF, ID_NOT_DEF, Employees, 0, fltEmployees);

	COPY_LINK(Employees, Orders, COrders::fltEmployeeID, COrders::fltEmployeeID);
	COPY_XLINK(Employees, Orders, COrders::fltEmployeeID);

	COPY_RLINK(Employees, Employees, CEmployees::fltReportsTo);
	COPY_RLINK(Shippers, Orders, COrders::fltShipVia);

	COPY_LINK(Orders, OrderDetails, COrderDetails::fltOrderID, COrderDetails::fltOrderID);
	COPY_RLINK(Products, OrderDetails, COrderDetails::fltProductID);
}

UIChoiceKind CDemoCopyHelper::AskChoice(COrderVariant* pLink)
{
	CDBTable* pTblTo = pLink->GetTblCopyTo();

	CDlgUIChoicePrompt dlg;

	dlg.m_strStaticText 
		= _T("Matching record is already present in the destination table \"") 
		+ CString(pTblTo->GetTableName()) 
		+ _T("\".\nChoose an appropriate action:");

	UIChoiceKind choice = uiUseExisting;
	switch(dlg.DoModal())
	{
	case IDC_USE_EXISTING:	
		choice = uiUseExisting; 
		break;
	case IDC_CREATE_NEW:		
		choice = uiCreateNew; 
		break;
	case IDC_OVERWRITE:		
		choice = uiOverwrite; 
		break;
	default: ASSERT(0);
	}
	AfxGetApp()->RestoreWaitCursor();
	if(dlg.m_bApplyToAll)
		pLink->SetUIChoiceKind(choice);
	return choice;
}