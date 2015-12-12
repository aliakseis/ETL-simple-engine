// Products.h: interface for the CProducts class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PRODUCTS_H__0C3F1842_675D_11D7_9CE6_ED44D460271E__INCLUDED_)
#define AFX_PRODUCTS_H__0C3F1842_675D_11D7_9CE6_ED44D460271E__INCLUDED_

#pragma once

#include "../ETLLib/DBTable.h"

class CProducts : public CDBTable  
{
public:
	enum 
	{
		fltSupplierID = fltRelation,
		fltCategoryID = fltRelation << 1,
	};

	static LPCWSTR g_szTableName;

	CProducts(CTableHolder* pConn);
	virtual ~CProducts();

};

#endif // !defined(AFX_PRODUCTS_H__0C3F1842_675D_11D7_9CE6_ED44D460271E__INCLUDED_)
