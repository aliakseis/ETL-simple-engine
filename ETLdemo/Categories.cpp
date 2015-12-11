// Categories.cpp: implementation of the CCategories class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ETLdemo.h"
#include "Categories.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LPCWSTR CCategories::g_szTableName = L"Categories";

CCategories::CCategories(CTableHolder* pConn)
: CDBTable(pConn)
{
	m_pstrTableName = g_szTableName;
	static AtomDesc g_desc[] = 
	{
		{ fltAutoNumber, L"CategoryID" },

		{ fltUniqueIndex, L"CategoryName" },
	};
	m_pAtomDesc = g_desc;
	m_nAtomCount = sizeof(g_desc) / sizeof(g_desc[0]);
}

CCategories::~CCategories()
{

}
