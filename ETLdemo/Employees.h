// Employees.h: interface for the CEmployees class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMPLOYEES_H__D7A124E3_6791_11D7_9CE6_D5706ECC631E__INCLUDED_)
#define AFX_EMPLOYEES_H__D7A124E3_6791_11D7_9CE6_D5706ECC631E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../ETLLib/DBTable.h"

class CEmployees : public CDBTable  
{
public:
	enum 
	{
		fltReportsTo = fltRelation,
	};

	static LPCWSTR g_szTableName;

	CEmployees(CTableHolder* pConn);
	virtual ~CEmployees();

};

#endif // !defined(AFX_EMPLOYEES_H__D7A124E3_6791_11D7_9CE6_D5706ECC631E__INCLUDED_)
