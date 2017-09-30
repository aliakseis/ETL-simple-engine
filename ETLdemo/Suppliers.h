// Suppliers.h: interface for the CSuppliers class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SUPPLIERS_H__CE2A864F_57A6_11D7_9CE6_D43439106E1D__INCLUDED_)
#define AFX_SUPPLIERS_H__CE2A864F_57A6_11D7_9CE6_D43439106E1D__INCLUDED_

#pragma once

#include "../ETLLib/DBTable.h"

class CSuppliers : public CDBTable  
{
public:
	static LPCWSTR g_szTableName;
	
    explicit CSuppliers(CTableHolder* pConn);
	virtual ~CSuppliers();

};

#endif // !defined(AFX_SUPPLIERS_H__CE2A864F_57A6_11D7_9CE6_D43439106E1D__INCLUDED_)
