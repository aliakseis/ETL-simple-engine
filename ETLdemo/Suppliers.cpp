// Suppliers.cpp: implementation of the CSuppliers class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ETLdemo.h"
#include "Suppliers.h"

#include "../ETLLib/OrderVariant.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LPCWSTR CSuppliers::g_szTableName = L"Suppliers";

CSuppliers::CSuppliers(CTableHolder* pConn)
: CDBTable(pConn)
{
	m_pstrTableName = g_szTableName;
	static AtomDesc g_desc[] = 
	{
		{ fltAutoNumber, L"SupplierID" },

		{ fltUniqueIndex, L"CompanyName" },
		{ fltUniqueIndex, L"PostalCode" },
	};
	m_pAtomDesc = g_desc;
	m_nAtomCount = sizeof(g_desc) / sizeof(g_desc[0]);
}

CSuppliers::~CSuppliers()
{

}
