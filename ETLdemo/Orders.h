// Orders.h: interface for the COrders class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ORDERS_H__D7A124E5_6791_11D7_9CE6_D5706ECC631E__INCLUDED_)
#define AFX_ORDERS_H__D7A124E5_6791_11D7_9CE6_D5706ECC631E__INCLUDED_

#pragma once

#include "../ETLLib/DBTable.h"

class COrders : public CDBTable  
{
public:
	enum 
	{
		fltCustomerID = fltRelation,
		fltEmployeeID = fltRelation << 1,
		fltShipVia = fltRelation << 2,
	};

	static LPCWSTR g_szTableName;

	COrders(CTableHolder* pConn);
	virtual ~COrders();

};

#endif // !defined(AFX_ORDERS_H__D7A124E5_6791_11D7_9CE6_D5706ECC631E__INCLUDED_)
