// OrderDetails.cpp: implementation of the COrderDetails class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ETLdemo.h"
#include "OrderDetails.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LPCWSTR COrderDetails::g_szTableName = L"[Order Details]";

COrderDetails::COrderDetails(CTableHolder* pConn)
: CDBTable(pConn)
{
	m_pstrTableName = g_szTableName;
	static AtomDesc g_desc[] = 
	{
		{ fltOrderID, L"OrderID" },
		{ fltProductID, L"ProductID" },
	};
	m_pAtomDesc = g_desc;
	m_nAtomCount = sizeof(g_desc) / sizeof(g_desc[0]);
}

COrderDetails::~COrderDetails()
{

}
