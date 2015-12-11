// Shippers.h: interface for the CShippers class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHIPPERS_H__D7A124E4_6791_11D7_9CE6_D5706ECC631E__INCLUDED_)
#define AFX_SHIPPERS_H__D7A124E4_6791_11D7_9CE6_D5706ECC631E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../ETLLib/DBTable.h"

class CShippers : public CDBTable  
{
public:
	static LPCWSTR g_szTableName;

	CShippers(CTableHolder* pConn);
	virtual ~CShippers();

};

#endif // !defined(AFX_SHIPPERS_H__D7A124E4_6791_11D7_9CE6_D5706ECC631E__INCLUDED_)
