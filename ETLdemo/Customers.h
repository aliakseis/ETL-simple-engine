// Customers.h: interface for the CCustomers class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CUSTOMERS_H__D7A124E2_6791_11D7_9CE6_D5706ECC631E__INCLUDED_)
#define AFX_CUSTOMERS_H__D7A124E2_6791_11D7_9CE6_D5706ECC631E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../ETLLib/DBTable.h"

class CCustomers : public CDBTable  
{
public:
	static LPCWSTR g_szTableName;

	CCustomers(CTableHolder* pConn);
	virtual ~CCustomers();

};

#endif // !defined(AFX_CUSTOMERS_H__D7A124E2_6791_11D7_9CE6_D5706ECC631E__INCLUDED_)
