// Categories.h: interface for the CCategories class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CATEGORIES_H__0C3F1841_675D_11D7_9CE6_ED44D460271E__INCLUDED_)
#define AFX_CATEGORIES_H__0C3F1841_675D_11D7_9CE6_ED44D460271E__INCLUDED_

#pragma once

#include "../ETLLib/DBTable.h"

class CCategories : public CDBTable  
{
public:
	static LPCWSTR g_szTableName;

	explicit CCategories(CTableHolder* pConn);
	virtual ~CCategories();

};

#endif // !defined(AFX_CATEGORIES_H__0C3F1841_675D_11D7_9CE6_ED44D460271E__INCLUDED_)
