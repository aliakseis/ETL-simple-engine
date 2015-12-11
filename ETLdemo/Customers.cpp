// Customers.cpp: implementation of the CCustomers class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ETLdemo.h"
#include "Customers.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LPCWSTR CCustomers::g_szTableName = L"Customers";

CCustomers::CCustomers(CTableHolder* pConn)
: CDBTable(pConn)
{
	m_pstrTableName = g_szTableName;
	static AtomDesc g_desc[] = 
	{
		{ fltPrimaryKey, L"CustomerID" },

		{ fltUniqueIndex, L"CustomerID" },

		//{ fltUniqueIndex, L"City" },
		//{ fltUniqueIndex, L"CompanyName" },
		//{ fltUniqueIndex, L"PostalCode" },
	};
	m_pAtomDesc = g_desc;
	m_nAtomCount = sizeof(g_desc) / sizeof(g_desc[0]);
}

CCustomers::~CCustomers()
{

}
