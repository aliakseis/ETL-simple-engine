// Machine generated IDispatch wrapper class(es) created with ClassWizard

#include "stdafx.h"
#include "DataSourceLocator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataSourceLocator properties

/////////////////////////////////////////////////////////////////////////////
// CDataSourceLocator operations

long CDataSourceLocator::GetHWnd()
{
	long result;
	InvokeHelper(0x60020000, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CDataSourceLocator::SetHWnd(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x60020000, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

LPDISPATCH CDataSourceLocator::PromptNew()
{
	LPDISPATCH result;
	InvokeHelper(0x60020002, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

BOOL CDataSourceLocator::PromptEdit(LPDISPATCH* ppADOConnection)
{
	BOOL result;
	static BYTE parms[] =
		VTS_PDISPATCH;
	InvokeHelper(0x60020003, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		ppADOConnection);
	return result;
}
