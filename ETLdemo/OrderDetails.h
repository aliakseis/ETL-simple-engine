// OrderDetails.h: interface for the COrderDetails class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ORDERDETAILS_H__DA7AAFA1_681E_11D7_9CE6_A0B121E94B1E__INCLUDED_)
#define AFX_ORDERDETAILS_H__DA7AAFA1_681E_11D7_9CE6_A0B121E94B1E__INCLUDED_

#pragma once

#include "../ETLLib/DBTable.h"

class COrderDetails : public CDBTable  
{
public:
	enum 
	{
		fltOrderID = fltRelation,
		fltProductID = fltRelation << 1,
	};

	static LPCWSTR g_szTableName;

    explicit COrderDetails(CTableHolder* pConn);
	virtual ~COrderDetails();

};

#endif // !defined(AFX_ORDERDETAILS_H__DA7AAFA1_681E_11D7_9CE6_A0B121E94B1E__INCLUDED_)
