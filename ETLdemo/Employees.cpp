// Employees.cpp: implementation of the CEmployees class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ETLdemo.h"
#include "Employees.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LPCWSTR CEmployees::g_szTableName = L"Employees";

CEmployees::CEmployees(CTableHolder* pConn)
: CDBTable(pConn)
{
	m_pstrTableName = g_szTableName;
	static AtomDesc g_desc[] = 
	{
		{ fltAutoNumber, L"EmployeeID" },

		{ fltUniqueIndex, L"LastName" },
		{ fltUniqueIndex, L"FirstName" },
		{ fltUniqueIndex, L"PostalCode" },

		{ fltReportsTo, L"ReportsTo" },
	};
	m_pAtomDesc = g_desc;
	m_nAtomCount = sizeof(g_desc) / sizeof(g_desc[0]);
}

CEmployees::~CEmployees()
{

}
