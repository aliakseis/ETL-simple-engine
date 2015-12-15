
#ifndef __CopyHelper_h__
#define __CopyHelper_h__


#include "OrderVariant.h"


#pragma warning(disable:4100 4702)
#include <map>
#include <set>
#pragma warning(default:4100 4702)

//////////////////////////////////////////////////////////////////////////

inline bool HasParams(DWORD dwFilterType)
{
	return fltNoFilter != dwFilterType && dwFilterType < 0x00010000;
}

//////////////////////////////////////////////////////////////////////////

#define REFERENCE_PROGRESS_REDUCING_COEFF	4

typedef std::deque<CDBTable*> CDBTablePtrArray;
typedef std::deque<CXLink*> CXLinkPtrArray;


class ETLLIB_EXPORT CMapTbl2MapId 
: public std::map<CTableId, CMapIdentities*>
{
public:
	CMapIdentities* GetAtNew(CTableId pTableTo);
	bool Lookup(const CTableId& key, CMapIdentities*& rValue) const
	{
		const_iterator it = find(key);
		if (end() == it)
			return false;
		rValue = it->second;
		return true;
	}
};

enum EHandleResult { handled, postpone, error };


typedef std::deque<COrderEntry*> CArrayEntries;

typedef COrderVariantKey* POrderVariantKey;

struct IsPOrderEntryLess
{
	bool operator()(POrderVariantKey p1, POrderVariantKey p2) const
	{
		CHECK_ADDRESS(p1); CHECK_ADDRESS(p2);
		return p1->GetCopyTableId() < p2->GetCopyTableId();
	}
};

typedef std::multiset<COrderVariant*, IsPOrderEntryLess>	CMultiSetEntries;

typedef CDataHandlerKey* PDataHandlerKey;

struct IsPDataHandlerKeyLess
{
	bool operator()(PDataHandlerKey p1, PDataHandlerKey p2) const
	{
		CHECK_ADDRESS(p1); CHECK_ADDRESS(p2);
		return p1->GetCopyTableId() < p2->GetCopyTableId();
	}
};

typedef std::set<CDataHandler*, IsPDataHandlerKeyLess> CSetVariantBases;
typedef std::set<CDataProvider*, IsPDataHandlerKeyLess> CSetDataProviders;

typedef std::multimap<long, long>							CMultiMapIds;

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

protected:
	CArrayEntries			m_Entries;
	CMultiSetEntries		m_WorkFlowEntries;
	CMultiSetEntries		m_NextPassEntries;
	std::deque<COrderLink*> m_Links;
	std::deque<CXLink*>		m_XLinks;
	CMapTbl2MapId m_mapTbl2MapId;

	BOOL GoDownstairs(CCopyIterator CopyIterator,
							COrderVariant* pTwinTables);
	BOOL DoCopyLinkedTables(CSubstRecArrayPtr& parrSubstRec,
							COrderVariantKey* pTwinTables, bool bPrimary);
	BOOL HasVirginXLinks(CTableId pTblTo, int nCount);
	COrderLink* GetSelfLink(CTableId pTblTo);
	BOOL GoUpstairs(CDBTable* pTblTo, int nCount);
	virtual void SetDefValues(COrderVariant* ) {}
	void AddEntry(COrderEntry* pEntry);
	void AddLink(COrderLink* pLink);
	void AddXLink(CXLink* pL);


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

	CMapIdentities* GetMap(LPCWSTR pszTableName);

	bool IsLazyObjectBinding() const		{ return m_bLazyObjectBinding; }
	void SetLazyObjectBinding(bool bSet)	{ m_bLazyObjectBinding = bSet; }

	CMapIdentities* GetMap(const CTableId& id)
	{
		return m_mapTbl2MapId.GetAtNew(id);
	}

	void DoHandleRecord(COrderVariant* pLink, bool& bUpdate, bool& bHandleDependants);
	void ShowProgress();

	void SetDataProvider(const CTableId& id, CDataProvider* pDataProvider);

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
	void FreeDBHolders();
	void SetDataSources(CTableHolder* pHolderTo, 
		CTableHolder* pHolderFrom = NULL, 
		BOOL bAutoDelete = FALSE);

	BOOL CopyTables(IProgress* pProgressBar = NULL);
	CMapTbl2MapId* GetMapTbl2MapId() { return  &m_mapTbl2MapId; };
	virtual int GetCount() { return m_WorkFlowEntries.size() + m_Links.size(); }

	CTableHolder* GetHolderTo()   
	{ 
		CHECK_ADDRESS(m_pHolderTo);
		return m_pHolderTo; 
	}
	CTableHolder* GetHolderFrom() 
	{ 
		CHECK_ADDRESS(m_pHolderFrom);	
		return m_pHolderFrom;
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
	CTableHolder* m_pHolderTo;
	CTableHolder* m_pHolderFrom;
	BOOL m_bAutoDelete;
	IProgress* m_pProgress;

	CSetVariantBases m_OrderVariants;
	CSetDataProviders m_DataProviders;

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
	AddEntry(new CTypedEntryLink<C##Table>(CCopyIterator(ValueTo,ValueFrom),\
		CreateTableId<C##Table>(), FIELD_MACRO(C##Table, Field), FilterType))

#define COPY_ENTRY_ACCESSORY(Value, Table, Field)							\
	COPY_ENTRY(Value, Value, Table, Field, fltUndefined)

#define COPY_ENTRY_PK(ArrayId, Table, Field)								\
	AddEntry(new CTypedEntryLink<C##Table>(ArrayId,							\
		CreateTableId<C##Table>(), FIELD_MACRO(C##Table, Field)))

#define COPY_LINK(MsTable, SlTable, SlField, FilterType)					\
	AddLink(new CTypedEntryLink<C##SlTable>(CreateTableId<C##MsTable>(),	\
		CreateTableId<C##SlTable>(), FIELD_MACRO(C##SlTable, SlField), FilterType))

#define COPY_XLINK(MsTable, SlTable, SlField)								\
	AddXLink(new CTypedEntryLink<C##MsTable>(false, CreateTableId<C##MsTable>(),\
		CreateTableId<C##SlTable>(), FIELD_MACRO(C##SlTable, SlField)))

#define COPY_RLINK(MsTable, SlTable, SlField)								\
	AddXLink(new CTypedEntryLink<C##MsTable>(true, CreateTableId<C##MsTable>(),\
		CreateTableId<C##SlTable>(), FIELD_MACRO(C##SlTable, SlField)))

#endif// __CopyHelper_h__
