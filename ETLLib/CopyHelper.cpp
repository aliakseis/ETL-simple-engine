/*
 	Replication & Migration stuff description.

	There are four useful macros describing structure and order of replication:
1. COPY_ENTRY, 2. COPY_LINK, 3. COPY_XLINK, 4. COPY_RLINK.

	Replication graph structure is tree-like; but it is a little more complicated then tree. It consists of 
several trees, which can interfere and contain self-linked branches. COPY_ENTRY macro describes 
tree root items. In a single item replication case this means that all the data in SlaveTable 
with SlaveField value = lIdFrom should be replicated using fltFilter for search and cleanup. SlaveField in 
table being copied to will be replaced by lIdTo. For database replication stuff, there are 
COPY_ENTRY macros with fltFilter = fltNoFilter. It means that all data in SlaveTable should be 
copied. SlaveField in this case should be PK field. In this case lIdTo, lIdFrom will be ignored 
automatically. There is also COPY_ENTRY_PK(ArrayId, SlaveTable, SlaveField) macro, where 
ArrayId is pointer to a CArray containing Ids of records to be replicated. SlaveField here is PK.
	Entries start walking through tree structure described by COPY_LINK macros. This macro 
means that all SlaveTable records with SlaveField equal PK in MasterTable should be copied and 
SlaveField value in table being copied to will be replaced by PK in data receiving MasterTable 
record which was being copied earlier. There exist also "indirect" entries referring to link slave 
tables. These entries do not start process of walking through tree. They are used together with links 
to support filters consisting of several fields. Several entries can be applied to one table 
simultaneously, too. However links are always treated separately.
	COPY_XLINK macro describes links being out of this copy force tree (e. g denormalized tables). 
It requires to update SlaveField values in SlaveTable according to PK in MasterTable. This x-link requires 
for MasterTable to be replicated first. It is important that it can cause deadlock and failure of replication 
process if links and x-links are constructed in wrong way and cross each other. If  MasterTable 
record with proper PK is not available, and matching record is not found with fltUniqueIndex filter, 
new record in MasterTable is created and its data is copied from corresponding record on the input side.
	R-link created by COPY_RLINK macro is similar to one created by COPY_XLINK. The 
main difference is that it does not require for MasterTable to be replicated first. This macro is 
destined for links to reference tables. These links are out of scope of tree-like replication structure.
	X- and r-links share cache maps for input and output primary keys organized by master table 
type for replication optimization. During database migration, reference tables are copied, and, 
correspondingly, cache maps are filled before main replication routine.
	Most links require no customization. There is a set of template-based classes for other links. 
Base class CDataHandler contains virtual functions. They do nothing or implement default actions:

	virtual BOOL DeleteRecord()
	{ 
		return GetTblCopyTo()->DeleteRecord(); 
	}
	virtual void CorrectTableData() {}
	virtual BOOL AfterUpdate() { return TRUE; }

	DeleteRecord() should be used for tables which records cannot be deleted automatically. In 
this case for COPY_ENTRY correct work link constructor should contain SetLeaveData(lkLeaveUnique) call, 
and only records which match by unique index will be deleted. 
	CorrectTableData() is used for tables where not all the data is set by replication routine. 
It can be also used for data transformation.
	AfterUpdate() is destined for actions which should take place after actual table update. 
	This set of virtual functions can be easily extended.
*/

#include "stdafx.h"
#include "CopyHelper.h"

#pragma warning(disable:4100)
#include <algorithm>
#pragma warning(default:4100)

#include <functional>

#include "TableHolder.h"
#include "DBTable.h"

#include <google/dense_hash_set>

#pragma warning(disable:4100)
#include <boost/foreach.hpp>
#pragma warning(default:4100)

#include <boost/bind.hpp>

using std::for_each;
using std::pair;
using std::swap;
using std::find_if;
using std::equal_to;
using std::logical_and;
using std::logical_not;

using namespace boost;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////

CMapIdentities* CMapTbl2MapId::GetAtNew(CTableId pTableTo)
{
	if(!pTableTo)
	{
		ASSERT(0);
		return NULL;
	}

	CMapIdentities* pMapId;
	CMapTbl2MapId::iterator lb = lower_bound(pTableTo);
	if (lb != end() && !(key_comp()(pTableTo, lb->first)))
		pMapId = lb->second;
	else
	{
		pMapId = new CMapIdentities;
		insert(lb, value_type(pTableTo, pMapId));
	}

	ASSERT(pMapId);
	return pMapId;
}

//////////////////////////////////////////////////////////////

struct Delete
{
	template<class T> void operator ()(T* pOb)
	{
		delete pOb;
	}
};


//////////////////////////////////////////////////////////////

class transfer_exception {};

void HandleTransferIds(CSubstRecArrayPtr& arrOutpSubstRec, 
								 Identity lIdTo, Identity lIdFrom, bool bAdded)
{
	if(bAdded)
		arrOutpSubstRec->push_back(CSubstRec(lIdTo, lIdFrom));
}

template<typename T, typename C, typename H>
// Does not work with other result types
inline void TransferData(T* pHolder, const C& rContext, COrderVariant* pVar, int nCount, 
	 bool bForceAdd, CMapIdentities* pSubstId, H& rIdHandler, bool canBatch)
{
	CHECK_ADDRESS(pHolder);
	CHECK_ADDRESS(pVar);
	CHECK_ADDRESS(pSubstId);

	CDBTable* pTblTo = pVar->GetTblCopyTo();

	pVar->CopyData(false);
	pVar->SetIDs();

	if(!pHolder->DoConvertAndFilter(pVar, pSubstId, rContext))
		return;

	if(pVar->IsObsolete())
		return;
	if(!pHolder->GoUpstairs(pTblTo, nCount - 1))
		throw transfer_exception();

	pHolder->SetDefValues(pVar);
	pVar->CorrectTableData();
	if(pVar->IsObsoleteByRefs())
		return;

	bool bUpdate = false;
	bool bHandleDependants = false;

	Identity lIdTo = ID_NULL;

	if(bForceAdd || !pTblTo->HasUniqueFilter()
	|| !pVar->FindMatchByUI())
	{
		pHolder->DoAddDropRecord(pVar, bUpdate);
		bHandleDependants = bUpdate;
	}
	else
	{
		if(pHolder->HasSameDatabases() && pTblTo->HasPrimaryKey()
		&& pTblTo->GetPrimaryKey() == pVar->GetPrimaryKeyFrom())
			return;
		pHolder->DoHandleRecord(pVar, bUpdate, bHandleDependants);
		lIdTo = pTblTo->GetPrimaryKey();
	}
	if(bUpdate)
	{
		if(!pTblTo->Update(canBatch) || !pVar->AfterUpdate()) 
			throw transfer_exception();
		lIdTo = pTblTo->GetPrimaryKey();
	}
	if(IsValid(lIdTo))
	{
		pSubstId->SetAt(pVar->GetPrimaryKeyFrom(), lIdTo);
		HandleTransferIds(rIdHandler, lIdTo, pVar->GetPrimaryKeyFrom(), bHandleDependants);
	}
}

//////////////////////////////////////////////////////////////


CTblCopyHelper::CTblCopyHelper()
{
	m_pHolderTo	  = NULL;
	m_pHolderFrom = NULL;
	m_bAutoDelete = FALSE;
	m_pProgress	  = NULL;
	m_nReferenceTables = -1;
	m_nPass = 0;
	m_dwCopyFlags = 0;
	m_eSameDBases = eNotDef;

	m_bLazyObjectBinding = true;

	m_nProgressDelay = 0;

	m_bFastLoad = false;
}

CTblCopyHelper::~CTblCopyHelper()
{
	FreeArrays();
	FreeDBHolders();
}

template <typename T> void FreePtrContainer(T& container)
{
	for_each(container.begin(), container.end(), Delete());
	container.clear();
}

void CTblCopyHelper::FreeArrays()
{
	FreePtrContainer(m_Entries);
	FreePtrContainer(m_WorkFlowEntries);
	FreePtrContainer(m_NextPassEntries);
	FreePtrContainer(m_Links);
	FreePtrContainer(m_XLinks);
	FreePtrContainer(m_OrderVariants);
	FreePtrContainer(m_DataProviders);

	for_each(m_mapTbl2MapId.begin(), m_mapTbl2MapId.end(), DeleteSecond());
	m_mapTbl2MapId.clear();

	m_nReferenceTables = -1;
	m_nPass = 0;
}

void CTblCopyHelper::FreeDBHolders()
{
	if(m_bAutoDelete) 
	{
		delete m_pHolderTo;
		delete m_pHolderFrom;
	}
	m_pHolderTo = NULL;
	m_pHolderFrom = NULL;
}

void CTblCopyHelper::SetDataSources(CTableHolder* pHolderTo, 
		CTableHolder* pHolderFrom /*= NULL*/, 
		BOOL bAutoDelete /*= FALSE*/)
{
	FreeArrays();
	FreeDBHolders();
	if(!pHolderFrom)
		pHolderFrom = pHolderTo;
	CHECK_ADDRESS(pHolderTo);
	CHECK_ADDRESS(pHolderFrom);
	m_pHolderTo	  = pHolderTo;
	m_pHolderFrom = pHolderFrom;
	m_bAutoDelete = bAutoDelete;
}



BOOL CTblCopyHelper::CopyTables(IProgress* pProgressBar /*= NULL*/)
{
	CHECK_NULL_OR_ADDRESS(pProgressBar);

	m_nProgressDelay = 0;
	
	PumpPaintMsg();

	GetHolderTo()->SetReadOnly(false);
	if (GetHolderFrom() != GetHolderTo())
		GetHolderFrom()->SetReadOnly(true);

//	SetAutoCommitOff();

	BOOL bOK = BeforeCopyTables(pProgressBar) 
				&& DoCopyTables(pProgressBar)
				&& AfterCopyTables();

	//if(bOK)
	//	Commit();
	//else
	//	Rollback();
	//SetAutoCommitOn();

	return bOK;
}


enum { EXTRA_SPACE = 1000 };

BOOL CTblCopyHelper::DoCopyTables(IProgress* pProgress /*= NULL*/)
{
	m_pProgress = pProgress;

#ifdef _DEBUG
	int i;
	for(i = m_Links.size(); --i >= 0; )
	{
		COrderLink* pLink1 = m_Links[i];
		CHECK_ADDRESS(pLink1);
		for(int j = i; --j >=0; )
		{
			COrderLink* pLink2 = m_Links[j];
			CHECK_ADDRESS(pLink2);
			if(pLink1->GetTblSlaveTo() == pLink2->GetTblSlaveTo())
			{
				TRACE("Replication: Duplicate links to table \"%ls\" from tables \"%ls\" and \"%ls\".\n", 
						pLink1->GetTblSlaveTo().GetTableName(),
						pLink1->GetTblMasterTo().GetTableName(), 
						pLink2->GetTblMasterTo().GetTableName());

			}
		}
	}
#endif//_DEBUG


	ASSERT(m_WorkFlowEntries.size() > 0);
	int nMaxCount = m_Links.size() + m_XLinks.size() + EXTRA_SPACE;
	bool bResult = true;
	bool bFirst = true;
	do
	{
		m_nPass++;

		if (!bFirst)
		{
			ArrangeOrphanXLinks(false);
		}
		bFirst = false;

		for(int nIteration = 0; m_WorkFlowEntries.size() > 0; nIteration++) 
		{
			bool bHandled = false;
			for(CMultiSetEntries::iterator iter = m_WorkFlowEntries.begin()
			; iter != m_WorkFlowEntries.end()
			; )// Increment in the operator body
			{
#ifdef _DEBUG
				m_arrVirginXLinks.clear();
#endif// _DEBUG
				COrderEntry* pEntry = static_cast<COrderEntry*>(*iter);
				CHECK_ADDRESS(pEntry);
				switch(HandleEntry(pEntry))
				{
				case handled:
					bHandled = true;
					m_WorkFlowEntries.erase(iter++);
					delete pEntry;
					ShowProgress();
					continue;
				case error:
					bResult = FALSE;
					ASSERT(0);
					break;
				case postpone:
					iter++;
					continue;
				}
				break;
			}
			if(!bResult || !bHandled || nIteration >= nMaxCount 
			|| int(m_WorkFlowEntries.size()) >= nMaxCount) 
			{
#ifdef _DEBUG
				for(CMultiSetEntries::const_iterator iter = m_WorkFlowEntries.begin()
				; iter != m_WorkFlowEntries.end(); ++iter)
					TRACE(_T("Replication Error: Stuck table \"%ls\"\n"), (*iter)->GetTblSlaveTo().GetTableName());

				TRACE("Replication Error: List of virgin x-links\n");

				for(i = m_arrVirginXLinks.size(); --i >= 0; )
					TRACE(_T("%ls\n"), m_arrVirginXLinks[i].c_str());

				TRACE("Replication Error: End of List of virgin x-links\n");
#endif
				ASSERT(0); 
				bResult = FALSE;
				break;
			}
		}

		m_WorkFlowEntries.swap(m_NextPassEntries);
	} 
	while (bResult && !m_WorkFlowEntries.empty());

#ifdef _DEBUG
	BOOST_FOREACH(COrderLink* pLink, m_Links)
	{
		if(!pLink->IsEverPassed())
		{
			TRACE("Replication: Link from table \"%ls\" to table \"%ls\" isn't passed.\n", 
						pLink->GetTblMasterTo().GetTableName(), pLink->GetTblSlaveTo().GetTableName());
		}
	}
#endif

	return bResult;
}

EHandleResult CTblCopyHelper::HandleEntry (COrderEntry* pEntry)
{
	if(HasVirginXLinks(pEntry->GetTblSlaveTo(), m_Links.size() + m_XLinks.size())) 
		return postpone;

	COrderEntry* pCreator = NULL;
	BOOST_FOREACH(COrderLink* pLink, m_Links)
	{
		if(IsEqual(pLink, pEntry))
		{
			ASSERT(!pCreator);
			pCreator = pLink;
		}
		else if(pLink->GetTblSlaveTo() == pEntry->GetTblSlaveTo()
		&& !pLink->IsEverForked() && !IsSerialLink(pLink))
		{
			ASSERT(!pLink->IsPassed());
			return postpone;
		}
	}

	pEntry->SetPassed();
	if(pCreator && !pCreator->IsPassed())
	{
		ShowProgress();
		pCreator->SetPassed();
	}
	BOOL bResult = GoDownstairs(pEntry->m_CopyIterator, pEntry);
	return bResult? handled : error;
}


BOOL CTblCopyHelper::HasVirginXLinks(CTableId pTblTo, int nCount)
{
	if(0 > nCount) 
	{
		ASSERT(FALSE); return FALSE;
	}

	BOOST_FOREACH(CXLink* pXLink, m_XLinks)
	{
		if(!pXLink->IsPassed() && pXLink->GetTblSlaveTo() == pTblTo
		&& !pXLink->IsSelfLink()
		&& (!pXLink->IsByReference() || HasVirginXLinks(pXLink->GetTblMasterTo(), nCount-1)))
		{
#ifdef _DEBUG
			m_arrVirginXLinks.push_back(pXLink->GetTblMasterTo().GetTableName() 
				+ wstring(L"\"-\"") + pXLink->GetTblSlaveTo().GetTableName());
#endif// _DEBUG
			return TRUE;
		}
	}

	return FALSE;
}

COrderLink* CTblCopyHelper::GetSelfLink(CTableId pTblTo)
{
	COrderLink* pSelfLink = NULL;
	BOOST_FOREACH(COrderLink* pLink, m_Links)
	{
		if(pLink->GetTblMasterTo() == pTblTo && pLink->IsSelfLink())
		{
			ASSERT(NULL == pSelfLink);
			pSelfLink = pLink;
#ifndef _DEBUG
			break;
#endif//_DEBUG
		}
	}
	return pSelfLink;
}

class CIdSet : public GOOGLE_NAMESPACE::dense_hash_set<Identity>
{
public:
	CIdSet()
	{
		set_empty_key(::Identity());
	}
};

bool IdToPresents(CMapIdentities* pSubstId, Identity lIdToFind, CIdSet& rIdCache)
{
	if(pSubstId->empty())
		return false;
	
	if(rIdCache.empty())
	{
		CMapIdentities::const_iterator iterEnd = pSubstId->end();
		for(CMapIdentities::const_iterator iter = pSubstId->begin()
		; iter != iterEnd
		; ++iter)
		{
			rIdCache.insert(iter->second);
		}
	}
	return IsValid(lIdToFind) && (rIdCache.end() != rIdCache.find(lIdToFind));
}

bool CTblCopyHelper::IsPassed(CTableId pTblTo)
{
	BOOST_FOREACH(COrderLink* pLink, m_Links)
	{
		if(pLink->GetTblSlaveTo() == pTblTo && !pLink->IsPassed())
			return false;
	}

	typedef CMultiSetEntries::const_iterator CIterator;
	COrderVariantKey key(pTblTo);
	pair<CIterator, CIterator> 
		range(m_WorkFlowEntries.equal_range(static_cast<COrderVariant*>(&key)));

	for(CMultiSetEntries::const_iterator iter = range.first
	; iter != range.second
	; ++iter)
	{
		COrderEntry* pEntry = static_cast<COrderEntry*>(*iter);
		CHECK_ADDRESS(pEntry);
		ASSERT(pEntry->GetTblSlaveTo() == pTblTo);
		if(!pEntry->IsPassed())
			return false;
	}
	return true;
}

void CTblCopyHelper::MarkRelatedXLinksPassed(CTableId pTblTo)
{
	if(IsPassed(pTblTo))
		BOOST_FOREACH(CXLink* pXLink, m_XLinks)
		{
			if(pXLink->GetTblMasterTo() == pTblTo)
			{
				pXLink->SetPassed();
			}
		}
}

//////////////////////////////////////////////////////////////

struct CDownstairsContext
{
	Identity m_lPKTo;
	CMapIdentities* m_pSubstParentId;
	bool m_bConvert;

	CDownstairsContext()
	{
		m_lPKTo = ID_NOT_DEF;
		m_pSubstParentId = NULL;
		m_bConvert = false;
	}
};

class CTblCopyHelperDownstairs : public CTblCopyHelper
{
	template<typename T, typename C, typename H>
	friend void TransferData(T* pHolder, const C& rContext, COrderVariant* pVar, 
		int nCount, bool bForceAdd, CMapIdentities* pSubstId, H& rIdHandler, bool canBatch);

	bool DoConvertAndFilter(COrderVariant* pVar, CMapIdentities* pSubstId, 
		const CDownstairsContext& rContext)
	{
		if(pVar->HasPrimaryKey())
		{
			Identity lId = pVar->GetPrimaryKeyFrom();
			if(IsValid(lId) && pSubstId->find(lId) != pSubstId->end())
				return false;
		}

		CDBTable* pTblTo		= pVar->GetTblCopyTo();
		size_t nFieldOffset = pVar->GetFieldOffset();

		BOOST_FOREACH(COrderEntry* pEntry, m_Entries)
		{
			if(pEntry->GetTblCopyTo() == pTblTo
			&& !pEntry->m_CopyIterator.ByPK())
			{
				if(nFieldOffset != pEntry->GetFieldOffset() 
				&& pEntry->GetFieldSlaveFrom() != pEntry->GetIteratorValueFrom())
					return false;
				pEntry->SetFieldSlaveTo(pEntry->GetIteratorValueTo());
			}
		}

		Identity lPkTo;
		if(IsValid(rContext.m_lPKTo))
			lPkTo = rContext.m_lPKTo; // Has priority to entries
		else if(rContext.m_pSubstParentId)
		{
			CMapIdentities::iterator iter = rContext.m_pSubstParentId->find(pVar->GetFieldSlaveFrom());
			if (rContext.m_pSubstParentId->end() == iter)
			{
				ASSERT(0);
				throw transfer_exception();
			}
			lPkTo = iter->second;
		}
		if (IsValid(lPkTo))
			pVar->SetFieldSlaveTo(lPkTo);
		if(rContext.m_bConvert)
			pVar->Convert();
		return true;
	}

	void DoAddDropRecord(COrderVariant* pVar, bool& bUpdate)
	{
		VERIFY(pVar->GetTblCopyTo()->AddRecord(FALSE));
		bUpdate = true;
	}
};

//////////////////////////////////////////////////////////////

BOOL CTblCopyHelper::GoDownstairs(CCopyIterator CopyIterator,
											 COrderVariant* pTwinTables)
{
	CDBTable* pTblTo	 = pTwinTables->GetTblCopyTo();

	CMapIdentities* pSubstParentId = NULL;		
	CMapIdentities* pSubstId = m_mapTbl2MapId.GetAtNew(pTwinTables->GetTblSlaveTo());		

// We should mark related Xlinks as passed anyway
	MarkRelatedXLinksPassed(pTwinTables->GetTblSlaveTo());

	COrderLink* pSelfLink = NULL;

	CSubstRecArrayPtr arrOutpSubstRec = new CSubstRecArray;
	size_t nPrevCount = 0;

	const bool bHasPrimaryKey = pTwinTables->HasPrimaryKey();
	bool bNextSubType = false;

	CIdSet IdCache;

	pTwinTables->FirstSubType();
	do
	{
		DWORD dwFilterType = pTwinTables->GetFilterType();
		ASSERT(dwFilterType != 0);
		LeaveKind eLeaveData = pTwinTables->GetLeaveData();

		do
		{
			PumpPaintMsg();

			for(int nValue = CopyIterator.GetSize(); --nValue >= 0; )
			{
				bool bForceAdd = (CopyIterator.ByPK() || lkLeaveData == eLeaveData) 
					&& !pTwinTables->IsUpdateDestination()	
					|| fltUniqueIndex == dwFilterType && lkLeaveUnique != eLeaveData;

				if(CopyIterator.ByPK())
				{
					if(!pTwinTables->FindByPrimaryKeyFrom(CopyIterator.GetValueFrom(nValue)))
						continue;
					if(!bNextSubType && lkLeaveData != eLeaveData 
						&& pTblTo->HasUniqueFilter() && !pTwinTables->IsUpdateDestination())
					{
						pTwinTables->CopyData();

						if(!GoUpstairs(pTblTo, EXTRA_SPACE))
							return FALSE;

						SetDefValues(pTwinTables);
						pTwinTables->CorrectTableData();
						if(pTwinTables->FindMatchByUI())
						{
							if(HasSameDatabases() && bHasPrimaryKey
							&& pTblTo->GetPrimaryKey() == pTwinTables->GetPrimaryKeyFrom())
								continue;
							else
								if (!pTblTo->DeleteRecord())
									bForceAdd = false;
						}
					}
				}
				else
				{
					bool bRestart = false;
					pTblTo->InitData();
		// First iterate thru sibling entries - both in and out
					BOOST_FOREACH(COrderEntry* pEntry, m_Entries)
					{
						if(pEntry->GetTblCopyTo() == pTblTo) 
						{
							ASSERT(pEntry == pTwinTables || fltNoFilter != pEntry->GetFilterType());
							if(pEntry->m_CopyIterator.ByPK())
							{
								if(!pEntry->IsPassed())
								{//	Substitute CopyIterator and restart iteration
									VERIFY(m_mapTbl2MapId.Lookup(pTwinTables->GetTblMasterTo(), 
																			pSubstParentId));
									CopyIterator = pEntry->m_CopyIterator;
									nValue		 = CopyIterator.GetSize();
									dwFilterType = pEntry->GetFilterType();
									bRestart = true;
									pEntry->SetPassed();
									break;
								}
							}
							else
							{
								pEntry->SetFieldSlaveFrom(pEntry->GetIteratorValueFrom());
								pEntry->SetFieldSlaveTo(pEntry->GetIteratorValueTo());
								pEntry->SetPassed();
							}
						}
					}
					if(bRestart)
						continue;

					if(dwFilterType != fltNoFilter)
						pTwinTables->SetFieldSlaveFrom(CopyIterator.GetValueFrom(nValue)); // Has priority to entries
					pTwinTables->SetFieldSlaveTo(CopyIterator.GetValueTo(nValue));
		// Remove dependants
		// Dependants should be removable - enable cascade deletes
					if(!bNextSubType && lkDeleteAll == eLeaveData)
					{
						DWORD filter = (dwFilterType < 0x00010000)? dwFilterType : fltNoFilter;
						if (pSubstId->empty())
							pTwinTables->DeleteRecords(filter);
						else
						{
							while (pTblTo->FindFirst(filter)) 
							{
								if(bHasPrimaryKey)
								{
									while(IdToPresents(pSubstId, pTblTo->GetPrimaryKey(), IdCache))
										if(fltUniqueIndex == dwFilterType || !pTblTo->FindNext())
											goto LoopExit;
								}
								if (!pTwinTables->DeleteRecord())
									break;
							}
			LoopExit:	;
						}
					}
					if(!pTwinTables->FindFirstFrom(dwFilterType))
						continue;
				}

				CDownstairsContext context;
				context.m_bConvert = (NULL != pSelfLink);
				context.m_lPKTo = CopyIterator.ByPK()? ID_NOT_DEF : CopyIterator.GetValueTo(nValue);
				context.m_pSubstParentId = pSubstParentId;

				size_t nPos = arrOutpSubstRec->size();
				do 
				{
					if (HasParams(dwFilterType) && !CopyIterator.ByPK()
					&& pTwinTables->GetFieldSlaveFrom() != CopyIterator.GetValueFrom(nValue)) 
						continue;
					try
					{
						TransferData(static_cast<CTblCopyHelperDownstairs*>(this), context,
							pTwinTables, EXTRA_SPACE, bForceAdd,
							pSubstId, arrOutpSubstRec, m_bFastLoad);
					}
					catch(transfer_exception&)
					{
						return false;
					}
				} 
				while(!CopyIterator.ByPK() && dwFilterType != fltUniqueIndex 
				&& pTwinTables->FindNextFrom());

				if(bHasPrimaryKey && !bNextSubType && lkLeaveData != eLeaveData)
				{
					if(IdCache.empty())
						IdToPresents(pSubstId, ID_NOT_DEF, IdCache); // To fill for the first time
					else
						for(; nPos < arrOutpSubstRec->size(); ++nPos)
							IdCache.insert((*arrOutpSubstRec)[nPos].m_lKeyTo);
				}
			}
			bNextSubType = true;
		}
		while(NULL == pSelfLink
		&& pTwinTables->NextSubType(&CopyIterator, dwFilterType));

		if(NULL == pSelfLink)
		{
			pSelfLink = GetSelfLink(pTwinTables->GetTblSlaveTo());
			ASSERT(pSelfLink != pTwinTables);
			if(NULL != pSelfLink)
				pSelfLink->SetPassed();
		}
		if(NULL != pSelfLink)
			if(arrOutpSubstRec->size() > nPrevCount)
			{
				CSubstRecArrayPtr arrSelfSubstRec = new CSubstRecArray;
				//for(size_t i = nPrevCount, nPrevCount = arrOutpSubstRec->size();
				//	 i < nPrevCount;
				//	 i++)
                for (size_t i = arrOutpSubstRec->size(); i-- > nPrevCount;)
						arrSelfSubstRec->push_back((*arrOutpSubstRec)[i]);
				CopyIterator.SetData(arrSelfSubstRec);
				pTwinTables = pSelfLink;
			}
			else pSelfLink = NULL;
	}
	while(NULL != pSelfLink);
	if(!pTwinTables->AfterCopyAll(arrOutpSubstRec))
		return FALSE;

	GetHolderTo()->FreeStatements();
	if (!HasSameDatabases())
		GetHolderFrom()->FreeStatements();

	return DoCopyLinkedTables(arrOutpSubstRec, pTwinTables, true);
}

BOOL CTblCopyHelper::DoCopyLinkedTables(CSubstRecArrayPtr& parrSubstRec,
							COrderVariantKey* pTwinTables, bool bPrimary)
{
	MarkRelatedXLinksPassed(pTwinTables->GetTblMasterTo());

	CTableId	pTblTo = pTwinTables->GetCopyTableId();
	BOOST_FOREACH(COrderLink* pLink, m_Links)
	{
		if(pLink->GetTblMasterTo() == pTblTo)
			if(bPrimary && pLink->IsSelfLink()) 
			{
				pLink->SetPassed();
				MarkRelatedXLinksPassed(pLink->GetTblSlaveTo());
			}
			else
			{
				if(parrSubstRec->size() 
				|| bPrimary && pLink->GetTblSlaveTo() != pTwinTables->GetTblMasterTo())
				{
					ASSERT(pLink->GetForkedAt() <= GetPass());
					bool bForkedThisPass = pLink->GetForkedAt() == GetPass();
					COrderEntry* pEntry = pLink->ForkEntry(parrSubstRec);
					if(NULL != pEntry)
					{
						if (bForkedThisPass)
							m_NextPassEntries.insert(pEntry);
						else
							m_WorkFlowEntries.insert(pEntry);

						++m_nProgressDelay;
					}
				}
				else
				{
					pLink->SetPassed();
					MarkRelatedXLinksPassed(pLink->GetTblSlaveTo());
				}
			}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////

struct CUpstairsContext {};
		
class CTblCopyHelperUpstairs : public CTblCopyHelper
{
	template<typename T, typename C, typename H>
	friend void TransferData(T* pHolder, const C& rContext, COrderVariant* pVar, 
		int nCount, bool bForceAdd, CMapIdentities* pSubstId, H& rIdHandler, bool canBatch);

	bool DoConvertAndFilter(COrderVariant*, CMapIdentities*, 
		const CUpstairsContext&)
	{
		return true;
	}

	void DoAddDropRecord(COrderVariant* pVar, bool& bUpdate)
	{
		VERIFY(pVar->GetTblCopyTo()->AddRecord(FALSE));
		bUpdate = true;
	}
};

//////////////////////////////////////////////////////////////

BOOL CTblCopyHelper::GoUpstairs(CDBTable* pTblTo, int nCount)
{
  	if(0 > nCount) 
	{
		ASSERT(FALSE); return FALSE;
	}
	BOOST_FOREACH(CXLink* pXLink, m_XLinks)
	{
		if(pXLink->GetTblSlaveTo() == pTblTo)
		{
			pXLink->FirstSubType();

			if(pXLink->Convert()
			|| !pXLink->IsDontShortcutRef() && pXLink->IsByReference() && HasSameDatabases())
				continue;

			Identity idFrom = pXLink->GetFieldSlaveFrom();

			if(!IsValid(idFrom))
			{
				pXLink->SetFieldSlaveTo(idFrom);
			}
			else if (pXLink->GetTblMasterTo() == pTblTo)
			{
				pXLink->SetFieldSlaveTo(ID_NULL);
			}
			else 
			{
				if(pXLink->FindByPrimaryKeyFrom(idFrom)) 
				{
					CSubstRecArrayPtr arrOutpSubstRec = new CSubstRecArray;
					try
					{
						TransferData(static_cast<CTblCopyHelperUpstairs*>(this), CUpstairsContext(),
							pXLink, nCount, false, pXLink->m_pMapId, arrOutpSubstRec, false);
					}
					catch(transfer_exception&)
					{
						return false;
					}

					pXLink->SetFieldSlaveTo(pXLink->GetTblCopyTo()->GetPrimaryKey());

					if(!pXLink->IsAbandonDependants() && arrOutpSubstRec->size() > 0)
						DoCopyLinkedTables(arrOutpSubstRec, pXLink, false);
				}
				else 
					pXLink->SetFieldSlaveTo(ID_NULL);
//				pXLink->FreeStatements();
			}
		}
	}
	return TRUE;
}

void CTblCopyHelper::AddEntry(COrderEntry* pEntry)
{
	CHECK_ADDRESS(pEntry);
	ASSERT(pEntry->IsEntry());
	pEntry->SetTblCopyHelper(this);
	m_Entries.push_back(pEntry);
	if(!IsLazyObjectBinding())
		VERIFY(pEntry->OrderVariantBase());
}

void CTblCopyHelper::AddLink(COrderLink* pL)
{
	CHECK_ADDRESS(pL);
	ASSERT(pL->IsLink());
	BOOST_FOREACH(COrderLink* pLink, m_Links)
	{
		if(IsEqual(pL, pLink)) 
		{
			ASSERT(pL->GetFilterType() == pLink->GetFilterType());
			delete pL;
			if(!IsLazyObjectBinding())
				VERIFY(pLink->OrderVariantBase());
			return;
		}
	}
	pL->m_pMapId = m_mapTbl2MapId.GetAtNew(pL->GetTblMasterTo());
	pL->SetTblCopyHelper(this);
	if(!IsLazyObjectBinding())
		VERIFY(pL->OrderVariantBase());
	m_Links.push_back(pL);
}

void CTblCopyHelper::AddXLink(CXLink* pL)
{
	CHECK_ADDRESS(pL);
	ASSERT(pL->IsXLink());

	BOOST_FOREACH(CXLink* pXLink, m_XLinks)
	{
		if(IsEqual(pL, pXLink)) 
		{
			delete pL;
			if(!IsLazyObjectBinding())
				VERIFY(pXLink->OrderVariantBase());
			return;
		}
	}
	pL->m_pMapId = m_mapTbl2MapId.GetAtNew(pL->GetTblMasterTo());
	pL->SetTblCopyHelper(this);
	if(!IsLazyObjectBinding())
		VERIFY(pL->OrderVariantBase());
	m_XLinks.push_back(pL);
}


bool CTblCopyHelper::IsSerialLink(COrderLink* pL)
{
	ASSERT(pL);
	switch(pL->GetSerialKind())
	{
		case skNotDefined:
			if(pL->GetTblMasterTo() == pL->GetTblSlaveTo())
			{
				pL->SetSerialKind(skSerial);
				return TRUE;
			}
			else
			{
				BOOST_FOREACH(COrderLink* pLink, m_Links)
				{
					if(pL->GetTblMasterTo() == pLink->GetTblSlaveTo() &&
					pLink->GetTblMasterTo() == pL->GetTblSlaveTo())
					{
						pL->SetSerialKind(skSerial);
						return TRUE;
					}
				}
				pL->SetSerialKind(skNotSerial);
				return FALSE;
			}
		case skNotSerial:  return FALSE;
		case skSerial:		 return TRUE;
	}
	ASSERT(0); return FALSE;
}

int CTblCopyHelper::EnumReferenceTables(CXLinkPtrArray* pArray /*= NULL*/)
{
	int nCount = 0;
	BOOST_FOREACH(CXLink* pXLink, m_XLinks)
	{
		if(pXLink->IsByReference())	
		{
			if(pArray)
				pArray->push_back(pXLink);
			nCount++;
		}
	}
	return nCount;
}

//////////////////////////////////////////////////////////////

struct CRefContext {};
		
class CTblCopyHelperRef : public CTblCopyHelper
{
	template<typename T, typename C, typename H>
	friend void TransferData(T* pHolder, const C& rContext, COrderVariant* pVar, 
		int nCount, bool bForceAdd, CMapIdentities* pSubstId, H& rIdHandler, bool canBatch);

	bool DoConvertAndFilter(COrderVariant* pVar, CMapIdentities* pMapId, 
		const CRefContext&)
	{
		return pMapId->find(pVar->GetPrimaryKeyFrom()) == pMapId->end();
	}

	void DoAddDropRecord(COrderVariant* pVar, bool& bUpdate)
	{
		VERIFY(pVar->GetTblCopyTo()->AddRecord(FALSE));
		bUpdate = true;
	}
};

//////////////////////////////////////////////////////////////

BOOL CTblCopyHelper::CopyReferenceTables(IProgress* pProgress, bool bClear /*= false*/)
{
	size_t i, j;
	CXLinkPtrArray XLinks;
//	Get reference links
	EnumReferenceTables(&XLinks);
//	Sort them
	for(i = 0; i < XLinks.size() - 1; i++)
		do
			for(j = i + 1; j < XLinks.size(); j++)
			{
				CXLink* pXLink1 = XLinks[i];
				CXLink* pXLink2 = XLinks[j];
				if(pXLink1->GetTblMasterTo() == pXLink2->GetTblSlaveTo())
				{
					ASSERT(pXLink2->GetTblMasterTo() != pXLink1->GetTblSlaveTo());
					swap(XLinks[i], XLinks[j]);
					break;
				}
			}
		while(j < XLinks.size());

//	Remove superfluous links
	for(i = 0; i < XLinks.size() - 1; i++)
	{
		CXLink* pXLink1 = XLinks[i];
		for(j = i + 1; j < XLinks.size(); j++)
		{
			CXLink* pXLink2 = XLinks[j];
			if(pXLink1->GetTblMasterTo() == pXLink2->GetTblMasterTo())
			{
				XLinks.erase(XLinks.begin() + j);
				j--;
				continue;
			}
		}
	}

	if(pProgress)
	{
		pProgress->SetRange(0,
			(short)(GetCount() + XLinks.size() / REFERENCE_PROGRESS_REDUCING_COEFF));
	}

	if(bClear)
	{
		DoClear(XLinks);
	}

//	Replicate data
	for(i = 0; i < XLinks.size(); i++)
	{
		PumpPaintMsg();

		CXLink* pXLink = XLinks[i];
		if(pXLink->IsDontCopyRef())
		{
			pXLink->OnCopyRefSpecial();
		}
		else
		{
			CMapIdentities* pMapId = NULL;
			VERIFY(m_mapTbl2MapId.Lookup(pXLink->GetTblMasterTo(), pMapId));

			CSubstRecArrayPtr arrOutpSubstRec = new CSubstRecArray;
			DWORD dwFilterType = fltNoFilter;
			pXLink->FirstSubType();
			do
			{
				if(pXLink->FindFirstFrom(dwFilterType)) do
				{
					try
					{
						TransferData(static_cast<CTblCopyHelperRef*>(this), CRefContext(),
							pXLink, XLinks.size(), false, pMapId, arrOutpSubstRec, m_bFastLoad);
					}
					catch(transfer_exception&)
					{
						return false;
					}
				}
				while(pXLink->FindNextFrom());
			}
			while(pXLink->NextSubType(NULL, dwFilterType));

			if(!pXLink->AfterCopyAll(arrOutpSubstRec))
				return FALSE;
			if(arrOutpSubstRec->size())
				DoCopyLinkedTables(arrOutpSubstRec, pXLink, false);
		}
		pXLink->FreeStatements();

		if(!(i % REFERENCE_PROGRESS_REDUCING_COEFF))
			ShowProgress();
	}
	return TRUE;
}

void CTblCopyHelper::ArrangeOrphanXLinks(bool bInitial)
{
	for (int i = m_XLinks.size(); --i > 0; )
	{
		CXLink* pXLink1 = m_XLinks[i];
		CHECK_ADDRESS(pXLink1);
		if(pXLink1->IsSelfLink())
			TRACE("Replication: X-link from table \"%ls\" to itself. Take special care about copying order.\n", 
					pXLink1->GetTblSlaveTo().GetTableName());
		if(!pXLink1->IsByReference() && !pXLink1->IsPassed())
		{
			CMultiSetEntries::const_iterator iter = m_WorkFlowEntries.find(pXLink1);

			if(iter == m_WorkFlowEntries.end())
			{
				if (m_Links.end() == find_if(m_Links.begin(), m_Links.end()
				, bind<bool>(equal_to<CTableId>(), pXLink1->GetTblMasterTo()
								, bind(&COrderLink::GetTblSlaveTo, _1))))
				{
					if (bInitial)
					{
						pXLink1->SetByReference();
						TRACE(_T("Orfan x-link from the table \"%ls\" to the table \"%ls\" converted to a r-link.\n"), 
							pXLink1->GetTblMasterTo().GetTableName(), pXLink1->GetTblSlaveTo().GetTableName());
					}
					else
					{
						pXLink1->SetPassed();
					}
				}
			}
		}
	}
}

BOOL CTblCopyHelper::BeforeCopyTables(IProgress* pProgressBar)
{
	TRACE(_T("Replication initial tables list:\n"));

	for (int j = m_Entries.size(); --j >= 0; )
	{
		COrderEntry* pEntry = m_Entries[j];
		if(pEntry->IsAccessory())
			continue;

		if (m_Links.end() == find_if(m_Links.begin(), m_Links.end()
		, bind<bool>(logical_and<bool>()
			, bind<bool>(logical_not<bool>(), bind(&CTblCopyHelper::IsSerialLink, this, _1))
			, bind<bool>(equal_to<CTableId>(), pEntry->GetTblSlaveTo()
								, bind(&COrderLink::GetTblSlaveTo, _1)))))
		{
			TRACE(_T("\"%ls\"\n"), m_Entries[j]->GetTblSlaveTo().GetTableName());
			m_WorkFlowEntries.insert(m_Entries[j]);
			m_Entries.erase(m_Entries.begin() + j);
		}
	}
	TRACE(_T("End of Replication initial tables list.\n"));

	ArrangeOrphanXLinks(true);

#ifdef _DEBUG
	for (int i = m_XLinks.size(); --i > 0; )
	{
		CXLink* pXLink1 = m_XLinks[i];
		CHECK_ADDRESS(pXLink1);
		if(pXLink1->IsSelfLink())
			TRACE("Replication: X-link from table \"%ls\" to itself. Take special care about copying order.\n", 
					pXLink1->GetTblSlaveTo().GetTableName());
		if (!pXLink1->IsByReference() || !pXLink1->GetTblSlaveTo())
			continue;
		for (int j = i; --j >=0; )
		{
			CXLink* pXLink2 = m_XLinks[j];
			if(!pXLink2->IsByReference())
				continue;
			CHECK_ADDRESS(pXLink2);
			if(pXLink1->GetTblSlaveTo() == pXLink2->GetTblSlaveTo())
			{
				for(int k = m_XLinks.size(); --k >= 0; )
				{
					CXLink* pXLink3 = m_XLinks[k];
					if(!pXLink3->IsByReference())
						continue;
					if(pXLink3->GetTblMasterTo() == pXLink1->GetTblSlaveTo())
					{
						TRACE("Replication: Duplicate ref.links to table \"%ls\" from tables \"%ls\" and \"%ls\".\n", 
								pXLink1->GetTblSlaveTo().GetTableName(),
								pXLink1->GetTblMasterTo().GetTableName(), 
								pXLink2->GetTblMasterTo().GetTableName());
						break;
					}
				}
			}
		}
	}
#endif//_DEBUG
	
	if (pProgressBar)
	{
		pProgressBar->SetRange(0, (short)GetCount());
		pProgressBar->SetStep(1);
		pProgressBar->SetPos(0);
	}

	return TRUE;
}

////////////////////////////////
// Maps stuff

CMapIdentities* CTblCopyHelper::GetMap(LPCWSTR pszTableName)
{
	if(NULL == pszTableName || 0 == *pszTableName)
	{
		ASSERT(0); return NULL;
	}
	CTableId id(NULL, pszTableName);
	CMapIdentities* pMap = NULL;
	m_mapTbl2MapId.Lookup(id, pMap);
	return pMap;
}

////////////////////////////////

bool CTblCopyHelper::HasSameDatabases()
{
	if(eNotDef == m_eSameDBases && GetHolderTo() && GetHolderFrom())
		m_eSameDBases = (GetHolderTo()->HasSameDatabase(*GetHolderFrom()))? eSame : eNot;
	return eSame == m_eSameDBases;
}


void CTblCopyHelper::DoHandleRecord(COrderVariant* pLink, bool& bUpdate
												, bool& bHandleDependants)
{
	bool bEditExisting = false;
	bool bAddNew = false;

	CDBTable* pTblTo = pLink->GetTblCopyTo();

	if(pLink->IsUpdateDestination())
		bEditExisting = true;
	else
	{
		UIChoiceKind choice = pLink->GetUIChoiceKind();
		if(uiAskChoice == choice)
		{
			choice = AskChoice(pLink);
		}

		switch(choice)
		{
		case uiCreateNew:
			bAddNew = true;
			bUpdate = true;
			break;
		case uiOverwrite:
			bEditExisting = true;
			break;
		}
	}
	if(bEditExisting)
	{
		bHandleDependants = true;
		VERIFY(pTblTo->EditRecord());
	}
	else if (bAddNew)
	{
		bHandleDependants = true;
		VERIFY(pLink->GetTblCopyTo()->AddRecord(FALSE));
	}
	else
		return;

	pLink->CopyData(false);
	VERIFY(GoUpstairs(pTblTo, m_XLinks.size()));
	SetDefValues(pLink);
	pLink->CorrectTableData();
	if (bAddNew)
		pLink->SetUniqueName();
	bUpdate = bAddNew || pTblTo->WasModified();
}

void CTblCopyHelper::ShowProgress()
{
	if(m_nProgressDelay)
		--m_nProgressDelay;
	else if(m_pProgress)
		m_pProgress->OffsetPos(1);
}

void CTblCopyHelper::SetDataProvider(const CTableId& id, CDataProvider* pDataProvider)
{
	pDataProvider->SetParameters(this, id);

	CDataHandlerKey key(id);
	CSetDataProviders::iterator iter 
		= m_DataProviders.find(static_cast<CDataProvider*>(&key));
	if(iter != m_DataProviders.end())
	{
		ASSERT(0);
		delete *iter;
        m_DataProviders.erase(iter);
	}
	m_DataProviders.insert(pDataProvider);
}
