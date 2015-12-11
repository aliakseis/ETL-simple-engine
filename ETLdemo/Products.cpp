// Products.cpp: implementation of the CProducts class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ETLdemo.h"
#include "Products.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LPCWSTR CProducts::g_szTableName = L"Products";

CProducts::CProducts(CTableHolder* pConn)
: CDBTable(pConn)
{
	m_pstrTableName = g_szTableName;
	static AtomDesc g_desc[] = 
	{
		{ fltAutoNumber, L"ProductID" },

		{ fltUniqueIndex, L"ProductName" },
		{ fltUniqueIndex, L"SupplierID" },
		{ fltUniqueIndex, L"CategoryID" },

		{ fltSupplierID, L"SupplierID" },

		{ fltCategoryID, L"CategoryID" },
	};
	m_pAtomDesc = g_desc;
	m_nAtomCount = sizeof(g_desc) / sizeof(g_desc[0]);
}

CProducts::~CProducts()
{

}
