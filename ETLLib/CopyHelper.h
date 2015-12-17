
#ifndef CopyHelper_h__
#define CopyHelper_h__


#include "OrderVariant.h"


#include <map>
#include <set>
#include <memory>

//////////////////////////////////////////////////////////////////////////

inline bool HasParams(DWORD dwFilterType)
{
	return fltNoFilter != dwFilterType && dwFilterType < 0x00010000;
}

//////////////////////////////////////////////////////////////////////////

#define REFERENCE_PROGRESS_REDUCING_COEFF	4

typedef std::deque<CXLink*> CXLinkPtrArray;


enum EHandleResult { handled, postpone, error };


struct IsPOrderEntryLess
{
    template<typename T1, typename T2>
	bool operator()(const T1& p1, const T2& p2) const
	{
        ASSERT(p1 != nullptr);
        ASSERT(p2 != nullptr);
        return p1->GetCopyTableId() < p2->GetCopyTableId();
	}
    typedef void is_transparent;
};


struct IsPDataHandlerKeyLess
{
    template<typename T1, typename T2>
    bool operator()(const T1& p1, const T2& p2) const
	{
		//CHECK_ADDRESS(p1); CHECK_ADDRESS(p2);
        ASSERT(p1 != nullptr);
        ASSERT(p2 != nullptr);
        return p1->GetCopyTableId() < p2->GetCopyTableId();
	}
    typedef void is_transparent;
};


class IProgress
{
public:
	virtual void SetRange(short nLower, short nUpper) = 0;
	virtual int SetStep(int nStep) = 0;
	virtual int SetPos(int nPos) = 0;
	virtual int OffsetPos(int nPos) = 0;
};

class ETLLIB_EXPORT CTblCopyHelper
{
//friend CDataHandler* COrderVariant::GetOrderVariantBase();
//friend CDataHandler* COrderVariant::GetFollowerOrderVariantBase();
friend class COrderVariant;
friend CDataProvider* CDataHandler::GetDataProvider();

    CTblCopyHelper(const CTblCopyHelper& other) = delete;
    CTblCopyHelper& operator =(const CTblCopyHelper& other) = delete;

protected:
    std::deque<std::unique_ptr<COrderEntry>> m_Entries;
    std::multiset<std::unique_ptr<COrderEntry>, IsPOrderEntryLess>
	    m_WorkFlowEntries,
	    m_NextPassEntries;
	std::deque<std::unique_ptr<COrderLink>> m_Links;
	std::deque<std::unique_ptr<CXLink>>		m_XLinks;
    std::map<CTableId, std::unique_ptr<CMapIdentities>> m_mapTbl2MapId;

	BOOL GoDownstairs(CCopyIterator CopyIterator,
							COrderVariant* pTwinTables);
	BOOL DoCopyLinkedTables(CSubstRecArrayPtr& parrSubstRec,
							COrderVariantKey* pTwinTables, bool bPrimary);
	BOOL HasVirginXLinks(CTableId pTblTo, int nCount);
	COrderLink* GetSelfLink(CTableId pTblTo);
	BOOL GoUpstairs(CDBTable* pTblTo, int nCount);
	virtual void SetDefValues(COrderVariant* ) {}
	void AddEntry(std::unique_ptr<COrderEntry> pEntry);
	void AddLink(std::unique_ptr<COrderLink> pLink);
	void AddXLink(std::unique_ptr<CXLink> pL);


	BOOL CopyReferenceTables(IProgress* pProgress, bool bClear = false);
	void DoClear(const CXLinkPtrArray& ) {}
	void ArrangeOrphanXLinks(bool bInitial);

	virtual BOOL BeforeCopyTables(IProgress* pProgress);
	BOOL DoCopyTables(IProgress* pProgress);
	virtual BOOL AfterCopyTables()			{ return TRUE; }
	int EnumReferenceTables(CXLinkPtrArray* pArray = NULL);
	int GetNumReferenceTables()
		{
			if(-1 == m_nReferenceTables)
				m_nReferenceTables = EnumReferenceTables();
			return m_nReferenceTables;
		}
	EHandleResult HandleEntry(COrderEntry* pEntry);

	IProgress* GetProgressCtrl() { return m_pProgress; }

	bool IsLazyObjectBinding() const		{ return m_bLazyObjectBinding; }
	void SetLazyObjectBinding(bool bSet)	{ m_bLazyObjectBinding = bSet; }

    bool LookupMap(const CTableId& key, CMapIdentities*& rValue) const;
    CMapIdentities* GetMap(const CTableId& id);

	void DoHandleRecord(COrderVariant* pLink, bool& bUpdate, bool& bHandleDependants);
	void ShowProgress();

	void SetDataProvider(const CTableId& id, std::unique_ptr<CDataProvider> pDataProvider);

	virtual UIChoiceKind AskChoice(COrderVariant* pLink) = 0;
	virtual void PumpPaintMsg()	{}

    template<typename C>
    void TransferData(const C& rContext, COrderVariant* pVar, int nCount,
        bool bForceAdd, CMapIdentities* pSubstId, CSubstRecArrayPtr& rIdHandler, bool canBatch);

    struct CDownstairsContext;
    bool DoConvertAndFilter(COrderVariant* pVar, CMapIdentities* pSubstId,
        const CDownstairsContext& rContext);

    struct CUpstairsContext {};
    bool DoConvertAndFilter(COrderVariant*, CMapIdentities*, const CUpstairsContext&)
    {
        return true;
    }

    struct CRefContext {};
    bool DoConvertAndFilter(COrderVariant* pVar, CMapIdentities* pMapId, const CRefContext&);

public:
	CTblCopyHelper();
	~CTblCopyHelper();
	void FreeArrays();
	void SetDataSources(std::shared_ptr<CTableHolder> pHolderTo, 
        std::shared_ptr<CTableHolder> pHolderFrom);

	BOOL CopyTables(IProgress* pProgressBar = NULL);
	virtual int GetCount() { return m_WorkFlowEntries.size() + m_Links.size(); }

	CTableHolder* GetHolderTo()   
	{ 
		ASSERT(m_pHolderTo != nullptr);
		return m_pHolderTo.get(); 
	}
	CTableHolder* GetHolderFrom() 
	{ 
		ASSERT(m_pHolderFrom != nullptr);	
		return m_pHolderFrom.get();
	}

	bool IsPassed(CTableId pTblTo);
	void MarkRelatedXLinksPassed(CTableId pTblTo);

	virtual void OnSetIDs(COrderVariant* )	{}
	virtual bool Convert(COrderVariant* )
		{
			return false;
		}
	void SetFlags(DWORD dwCopyFlags)
		{
			m_dwCopyFlags = dwCopyFlags;
		}
	DWORD GetFlags()	{ return m_dwCopyFlags; }

	bool IsSerialLink(COrderLink* pLink);
	bool HasSameDatabases();

	int GetPass()		{ return m_nPass; }

	bool IsFastLoad()	{ return m_bFastLoad; }
	void SetFastLoad(bool bFastLoad)	{ m_bFastLoad = bFastLoad; }

private:
	std::shared_ptr<CTableHolder> m_pHolderTo;
    std::shared_ptr<CTableHolder> m_pHolderFrom;
	IProgress* m_pProgress;

    std::set<std::unique_ptr<CDataHandler>, IsPDataHandlerKeyLess> m_OrderVariants;
    std::set<std::unique_ptr<CDataProvider>, IsPDataHandlerKeyLess> m_DataProviders;

	int  m_nReferenceTables;

	enum { eNotDef = -1, eNot, eSame } m_eSameDBases;

	bool m_bLazyObjectBinding;

	int m_nProgressDelay;


	int m_nPass;

	bool m_bFastLoad;

#ifdef _DEBUG
	std::deque<std::wstring> m_arrVirginXLinks;
#endif// _DEBUG

protected:
	DWORD			 m_dwCopyFlags;
};

#define FIELD_MACRO(Table, Field) Field

#define COPY_ENTRY(ValueTo, ValueFrom, Table, Field, FilterType)			\
	AddEntry(std::make_unique<CTypedEntryLink<C##Table>>(CCopyIterator(ValueTo,ValueFrom),\
		CreateTableId<C##Table>(), FIELD_MACRO(C##Table, Field), FilterType))

#define COPY_ENTRY_ACCESSORY(Value, Table, Field)							\
	COPY_ENTRY(Value, Value, Table, Field, fltUndefined)

#define COPY_ENTRY_PK(ArrayId, Table, Field)								\
	AddEntry(std::make_unique<CTypedEntryLink<C##Table>>(ArrayId,			\
		CreateTableId<C##Table>(), FIELD_MACRO(C##Table, Field)))

#define COPY_LINK(MsTable, SlTable, SlField, FilterType)					\
	AddLink(std::make_unique<CTypedEntryLink<C##SlTable>>(CreateTableId<C##MsTable>(),\
		CreateTableId<C##SlTable>(), FIELD_MACRO(C##SlTable, SlField), FilterType))

#define COPY_XLINK(MsTable, SlTable, SlField)								\
	AddXLink(std::make_unique<CTypedEntryLink<C##MsTable>>(false, CreateTableId<C##MsTable>(),\
		CreateTableId<C##SlTable>(), FIELD_MACRO(C##SlTable, SlField)))

#define COPY_RLINK(MsTable, SlTable, SlField)								\
	AddXLink(std::make_unique<CTypedEntryLink<C##MsTable>>(true, CreateTableId<C##MsTable>(),\
		CreateTableId<C##SlTable>(), FIELD_MACRO(C##SlTable, SlField)))

#endif// CopyHelper_h__
