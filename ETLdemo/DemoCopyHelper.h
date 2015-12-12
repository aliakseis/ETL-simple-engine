// DemoCopyHelper.h: interface for the CDemoCopyHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEMOCOPYHELPER_H__CE2A8650_57A6_11D7_9CE6_D43439106E1D__INCLUDED_)
#define AFX_DEMOCOPYHELPER_H__CE2A8650_57A6_11D7_9CE6_D43439106E1D__INCLUDED_

#pragma once

#include "../ETLLib/CopyHelper.h"

class CDemoCopyHelper : public CTblCopyHelper  
{
	BOOL m_bCustomersOrEmployeesOrders;

public:
	CDemoCopyHelper();
	virtual ~CDemoCopyHelper();

	void Init(const CCopyIterator& iterCustomers, DWORD fltCustomers,
		const CCopyIterator& iterEmployees, DWORD fltEmployees, BOOL bCustomersOrEmployeesOrders);

	BOOL IsCustomersOrEmployeesOrders() const
	{
		return m_bCustomersOrEmployeesOrders;

	}
protected:
	UIChoiceKind AskChoice(COrderVariant* pLink);
};

#endif // !defined(AFX_DEMOCOPYHELPER_H__CE2A8650_57A6_11D7_9CE6_D43439106E1D__INCLUDED_)
