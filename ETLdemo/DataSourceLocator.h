// Machine generated IDispatch wrapper class(es) created with ClassWizard
/////////////////////////////////////////////////////////////////////////////
// CDataSourceLocator wrapper class

class CDataSourceLocator : public COleDispatchDriver
{
public:
	CDataSourceLocator() {}		// Calls COleDispatchDriver default constructor
	explicit CDataSourceLocator(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CDataSourceLocator(const CDataSourceLocator& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	long GetHWnd();
	void SetHWnd(long nNewValue);
	LPDISPATCH PromptNew();
	BOOL PromptEdit(LPDISPATCH* ppADOConnection);
};
