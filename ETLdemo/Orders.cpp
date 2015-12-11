// Orders.cpp: implementation of the COrders class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ETLdemo.h"
#include "Orders.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LPCWSTR COrders::g_szTableName = L"Orders";

COrders::COrders(CTableHolder* pConn)
: CDBTable(pConn)
{
	m_pstrTableName = g_szTableName;
	static AtomDesc g_desc[] = 
	{
		{ fltAutoNumber, L"OrderID" },

		{ fltCustomerID, L"CustomerID" },
		{ fltEmployeeID, L"EmployeeID" },
		{ fltShipVia, L"ShipVia" },
	};
	m_pAtomDesc = g_desc;
	m_nAtomCount = sizeof(g_desc) / sizeof(g_desc[0]);
}

COrders::~COrders()
{

}
