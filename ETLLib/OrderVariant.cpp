#include "stdafx.h"

#include "CopyHelper.h"

#pragma warning(disable:4100)
#include <algorithm>
#pragma warning(default:4100)

#include "BellsMacrosX.h"
#include "DBTable.h"
#include "TableHolder.h"

#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using std::wstring;

///////////////////////////////////////////////////////////////////////////////////

Identity GetIdentityValue(const CDBTable* pTable, size_t offset)
{
	return pTable->GetIdentityValue(offset);
}

void SetIdentityValue(CDBTable* pTable, size_t offset, Identity value) 
{
	CHECK_ADDRESS(pTable);
	pTable->SetIdentityValue(offset, value);
}

///////////////////////////////////////////////////////////////////////////////////

inline CDBTable* HandleCreateTable(const CTableId& idTable, CDBTable*& rpTable, CTableHolder* pHolder)
{
	if(!rpTable)
		rpTable = idTable.GetDBTable(pHolder);
	return rpTable;
}

///////////////////////////////////////////////////////////////////////////////////

CCopyIterator::CCopyIterator(const CCopyIterator& other)
{
	m_ciKind  = ciNotDef; 
	*this = other;
}

const CCopyIterator& CCopyIterator::operator = (const CCopyIterator& other)
{
	Clear();
	*((CCopyIteratorData*)this) = (CCopyIteratorData&)other;
	if(ciMultiple == m_ciKind)
	{
		CHECK_ADDRESS(m_parrSubstRec);
		ASSERT(m_parrSubstRec->GetNumRefs() > 0);
		m_parrSubstRec->upcount();
	}
	return *this;
}

void CCopyIterator::Clear()
{
	if(ciMultiple == m_ciKind)
	{
		CHECK_ADDRESS(m_parrSubstRec);
		ASSERT(m_parrSubstRec->GetNumRefs() > 0);
		m_parrSubstRec->downcount();
		m_parrSubstRec = NULL;
	}
	m_ciKind  = ciNotDef; 
}


void CCopyIterator::SetData(Identity lValueTo, Identity lValueFrom)
{
	Clear();
	m_ciKind = ciSingle;
	m_pair.m_lValueTo = lValueTo; 
	m_pair.m_lValueFrom = lValueFrom;
}

void CCopyIterator::SetData(CSubstRecArrayPtr& parrSubstRec)
{
	Clear();
	m_ciKind = ciMultiple;
	m_parrSubstRec = parrSubstRec;
	m_parrSubstRec->upcount();
}

void CCopyIterator::SetData(std::deque<Identity>* parrId)
{
	Clear();
	m_ciKind = ciByPK;
	m_parrId = parrId;
}

void CCopyIterator::SetData(Identity lPK)
{
	Clear();
	m_ciKind = ciSingle;
	m_pair.m_lValueTo = ID_NOT_DEF; 
	m_pair.m_lValueFrom = lPK;
}

void CCopyIterator::AddData(CSubstRecArrayPtr& parrSubstRec)
{
	if(m_ciKind != ciMultiple)
	{
		ASSERT(0); return;
	}
//	Copy on write
	if(1 == m_parrSubstRec->GetNumRefs())
		m_parrSubstRec->Append(*parrSubstRec);
	else
	{
		ASSERT(m_parrSubstRec->GetNumRefs() > 0);
		CSubstRecArrayPtr arrSubstRec = new CSubstRecArray;
		arrSubstRec->Append(*m_parrSubstRec);
		arrSubstRec->Append(*parrSubstRec);
		m_parrSubstRec->downcount();
		m_parrSubstRec = arrSubstRec;
		m_parrSubstRec->upcount();
	}
}

BOOL CCopyIterator::ByPK()
{
	return ciByPK == m_ciKind 
	|| ciSingle == m_ciKind 
		&& ID_NOT_DEF == m_pair.m_lValueTo 
		&& IsValid(m_pair.m_lValueFrom);
}

Identity CCopyIterator::GetValueTo()
{
	if(ciSingle == m_ciKind)
		return m_pair.m_lValueTo;
	else
	{
		ASSERT(0);
		return ID_NOT_DEF;
	}
}

Identity CCopyIterator::GetValueFrom()
{
	if(ciSingle == m_ciKind)
		return m_pair.m_lValueFrom;
	else
	{
		ASSERT(0);
		return ID_NOT_DEF;
	}
}

Identity CCopyIterator::GetValueTo(int nIndex)
{
	switch(m_ciKind)
	{
		case ciSingle:
			ASSERT(0 == nIndex);
			return m_pair.m_lValueTo;
		case ciMultiple:
			CHECK_ADDRESS(m_parrSubstRec);
			return m_parrSubstRec->at(nIndex).m_lKeyTo;
		default:
			ASSERT(0);
			return ID_NOT_DEF;
	}
}

Identity CCopyIterator::GetValueFrom(int nIndex)
{
	switch(m_ciKind)
	{
		case ciSingle:
			ASSERT(0 == nIndex);
			return m_pair.m_lValueFrom;
		case ciMultiple:
			CHECK_ADDRESS(m_parrSubstRec);
			return m_parrSubstRec->at(nIndex).m_lKeyFrom;
		case ciByPK:
			CHECK_ADDRESS(m_parrId);
			return m_parrId->at(nIndex);
		default:
			ASSERT(0);
			return ID_NOT_DEF;
	}
}

int CCopyIterator::GetSize()
{
	switch(m_ciKind)
	{
		case ciSingle:
			return 1;
		case ciMultiple:
			CHECK_ADDRESS(m_parrSubstRec);
			return m_parrSubstRec->size();
		case ciByPK:
			if(NULL == m_parrId)
				return 0;
			CHECK_ADDRESS(m_parrId);
			return m_parrId->size();
		default:
			ASSERT(0);
			return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////

inline int safe_cscmp(LPCWSTR string1, LPCWSTR string2)
{
	if(NULL == string1)
		return (NULL == string2)? 0 : -1;
	if(NULL == string2)
		return 1;
	return wcscmp(string1, string2);
}


bool CTableId::operator == (const CTableId& other) const
{
	return !safe_cscmp(m_pszTableName, other.m_pszTableName);
}

bool CTableId::operator < (const CTableId& other) const
{
	return safe_cscmp(m_pszTableName, other.m_pszTableName) < 0;
}

bool CTableId::operator == (const CDBTable* other) const
{
	return !safe_cscmp(m_pszTableName, other->GetTableName());
}

CDBTable* CTableId::GetDBTable(CTableHolder* pHolder) const
{
	if(!m_pfnCreator || !pHolder)
	{
		ASSERT(0); return NULL;
	}
	CDBTable* pTable = NULL;
	if (!pHolder->Lookup(m_pszTableName, pTable))
	{
		pTable = m_pfnCreator(pHolder);
		pHolder->SetDBTable(m_pszTableName, pTable);
	}
	return pTable;
}


///////////////////////////////////////////////////////////////////////////////////


void MakeCopyName(wstring& strName, int nInstance)
{
	wstring strBuf = strName;
	int nIndex = strBuf.find_last_of(L" Copy");
	if(nIndex > 1)
	{
		size_t i = nIndex + 5;
		for( ; i < strBuf.length(); i++)
			if(strBuf[i] > L'9')
				break;
		if(strBuf.length() == i)
		{
			strName = strName.substr(0, nIndex);
		}
	}
	if(nInstance > 0)
	{
		std::wostringstream s;
		s << strName << L" Copy #" << nInstance;
		strName = s.str();
	}
	else
	{
		strName += L" Copy";
	}
}

///////////////////////////////////////////////////////////////////////////////////

void CXLink::AddCouple()
{
	m_pMapId->SetAt(GetPrimaryKeyFrom(), GetTblCopyTo()->GetPrimaryKey());
	SetPassed();
}

CTblCopyHelper* COrderVariant::GetTblCopyHelper()	
{ 
	CHECK_ADDRESS(m_pTblCopyHelper);
	return m_pTblCopyHelper; 
}

void COrderVariant::SetPassed()
{ 
	SetPassedAt(GetTblCopyHelper()->GetPass()); 
}


BOOL COrderVariant::IsPassed()
{
	ASSERT(GetTblCopyHelper()->GetPass() >= m_nPassedAt);
	if((m_dwFlags & OV_KIND) != OV_XLINK)
		return GetTblCopyHelper()->GetPass() == m_nPassedAt;
	if((GetTblCopyHelper()->GetPass() == m_nPassedAt))
		return TRUE;
	if(!m_pMapId)
	{
		ASSERT(FALSE);
		return FALSE;
	}
	if(!m_pMapId->empty() && GetTblCopyHelper()->IsPassed(GetTblMasterTo()))
	{
		SetPassed();
		return TRUE;
	}
	return FALSE;
}

BOOL COrderVariant::Convert()
{
	if(NULL != m_pMapId)
	{
		if(IsValid(GetFieldSlaveFrom()))
		{
			CMapIdentities::iterator it = m_pMapId->find(GetFieldSlaveFrom());
			if (it != m_pMapId->end())
				SetFieldSlaveTo(it->second);
			else
			{
				if(IsConvertOnly())
				{
					SetFieldSlaveTo(ID_NULL);
					return TRUE;
				}
				else if(GetTblCopyHelper()->Convert(this))
				{
					if(IsValid(GetFieldSlaveTo()))
					{
						m_pMapId->SetAt(GetFieldSlaveFrom(), GetFieldSlaveTo());
						GetOrderVariantBase()->OnRefAdded(GetFieldSlaveTo());
					}
				}
				else
					return FALSE;
			}
		}
		else
			SetFieldSlaveTo(GetFieldSlaveFrom());
		return TRUE;
	}
	else
		return FALSE;
}


void COrderVariant::SetIDs()
{
	GetTblCopyHelper()->OnSetIDs(this);
	GetOrderVariantBase()->SetIDs();
}

LeaveKind COrderVariant::GetLeaveData()		{ return GetOrderVariantBase()->GetLeaveData(); }

COrderVariant* COrderVariant::ForkEntry(CSubstRecArrayPtr& parrSubstRec)
{
	m_nForkedAt = GetTblCopyHelper()->GetPass();

	if(m_pForkEntry)
		if(m_pForkEntry->IsPassed())
		{
			m_pForkEntry->m_pForkEntry = NULL;
			m_pForkEntry = NULL;
		}
		else
		{
			m_pForkEntry->AddSubstRecs(parrSubstRec);
			return NULL;
		}

	COrderVariant* pEntry = new COrderVariant(*this);
	pEntry->ConvertToEntry(parrSubstRec);
	pEntry->m_pMapId = m_pMapId;
	pEntry->SetTblCopyHelper(GetTblCopyHelper());
	m_pForkEntry = pEntry;
	m_pForkEntry->m_pForkEntry = this;
	return pEntry;
}

void COrderVariant::ConvertToEntry(CSubstRecArrayPtr& SubstRecArrayPtr)
{
	ASSERT(OV_XLINK != (m_dwFlags & OV_KIND));
	m_CopyIterator.SetData(SubstRecArrayPtr);
	m_dwFlags = (m_dwFlags & ~OV_KIND) | OV_ENTRY;
	m_nPassedAt = INT_MIN;
}

BOOL COrderVariant::DeleteRecord()
{ 
	ASSERT((m_dwFlags & OV_KIND) != OV_XLINK);
	return GetOrderVariantBase()->DeleteRecord(); 
}

void COrderVariant::DeleteRecords(DWORD dwFilterType)
{ 
	ASSERT((m_dwFlags & OV_KIND) != OV_XLINK);
	GetOrderVariantBase()->DeleteRecords(dwFilterType); 
}

void COrderVariant::FirstSubType()			
{ 
	if(!GetOrderVariantBase()->IsInitialized())
	{
		GetOrderVariantBase()->SetInitialized();
		GetOrderVariantBase()->FirstSubType(); 
		GetOrderVariantBase()->GetDataProvider()->FirstSubType(); 
	}
}

bool COrderVariant::NextSubType(CCopyIterator* pCI, DWORD& rDW)
{ 
	GetOrderVariantBase()->SetInitialized(false);// ?
	return GetOrderVariantBase()->NextSubType(pCI, rDW); 
}

void COrderVariant::CorrectTableData()		{ GetOrderVariantBase()->CorrectTableData(); }
BOOL COrderVariant::FindMatchByUI()			{ return GetOrderVariantBase()->FindMatchByUI(); }
bool COrderVariant::IsObsolete()			{ return GetOrderVariantBase()->IsObsolete(); }
bool COrderVariant::IsObsoleteByRefs()		{ return GetOrderVariantBase()->IsObsoleteByRefs(); }
BOOL COrderVariant::AfterUpdate()			{ return GetOrderVariantBase()->AfterUpdate(); }
BOOL COrderVariant::FindNextFrom()			{ return GetOrderVariantBase()->FindNextFrom(); }

BOOL COrderVariant::AfterCopyAll(CSubstRecArray* pArr)
									{ return GetOrderVariantBase()->AfterCopyAll(pArr); }

void COrderVariant::FreeStatements()			
{ 
	GetTblCopyTo()->FreeStatements(); 
	GetOrderVariantBase()->FreeStatements(); 
}

BOOL COrderVariant::FindByPrimaryKeyFrom(Identity lId)
											{ return GetOrderVariantBase()->FindByPrimaryKeyFrom(lId); }
void COrderVariant::OnCopyRefSpecial()		{ GetOrderVariantBase()->OnCopyRefSpecial(); }
bool COrderVariant::IsUpdateDestination()	{ return GetOrderVariantBase()->IsUpdateDestination(); }
bool COrderVariant::IsDontCopyRef()			{ return GetOrderVariantBase()->IsDontCopyRef(); }
bool COrderVariant::IsDontShortcutRef()		{ return GetOrderVariantBase()->IsDontShortcutRef(); }
bool COrderVariant::IsAbandonDependants()	{ return GetOrderVariantBase()->IsAbandonDependants(); }
bool COrderVariant::IsConvertOnly()			{ return GetOrderVariantBase()->IsConvertOnly(); }


UIChoiceKind COrderVariant::GetUIChoiceKind() 
									{ return GetOrderVariantBase()->GetUIChoiceKind(); }
void COrderVariant::SetUIChoiceKind(UIChoiceKind ck)
									{ GetOrderVariantBase()->SetUIChoiceKind(ck); }

bool COrderVariant::SetUniqueName()			{ return GetOrderVariantBase()->SetUniqueName(); }


Identity	COrderVariant::GetFieldSlaveTo()	 const
{ 
	return GetIdentityValue(const_cast<COrderVariant*>(this)->GetTblSlaveTo_(), GetFieldOffset());
} 

Identity	COrderVariant::GetFieldSlaveFrom()  const
{ 
	const CDataHandler* pBase 
		= const_cast<COrderVariant*>(this)->GetSlaveOrderVariantBase();
	return (pBase)?
		pBase->GetFieldFrom(GetFieldOffset())
		: GetIdentityValue(const_cast<COrderVariant*>(this)->GetTblSlaveFrom_(), GetFieldOffset());
} 

void	COrderVariant::SetFieldSlaveTo(Identity lId)
{ 
	CHECK_ADDRESS(this);
	SetIdentityValue(GetTblSlaveTo_(), GetFieldOffset(), lId);
} 

void	COrderVariant::SetFieldSlaveFrom(Identity lId)
{ 
	CDataHandler* pBase = GetSlaveOrderVariantBase();
	if(pBase)
		pBase->SetFieldFrom(GetFieldOffset(), lId);
	else
		SetIdentityValue(GetTblSlaveFrom_(), GetFieldOffset(), lId);
} 


BOOL COrderVariant::FindFirstFrom(DWORD dwFilterType)
									{ return GetOrderVariantBase()->FindFirstFrom(dwFilterType); }
Identity COrderVariant::GetPrimaryKeyFrom()
									{ return GetOrderVariantBase()->GetPrimaryKeyFrom(); }
void COrderVariant::CopyData(bool bCopyPK)
									{ GetOrderVariantBase()->CopyData(bCopyPK); }

CDBTable* COrderVariant::GetTblCopyTo()
{ 
	return HandleCreateTable(
		GetCopyTableId(),
		m_pTblCopyTo,
		GetTblCopyHelper()->GetHolderTo());
}

CDataHandler* COrderVariant::GetOrderVariantBase()
{
	if(!m_pOrderVariant)
	{
		CTblCopyHelper* pTblCopyHelper = GetTblCopyHelper();
		if(pTblCopyHelper)
		{
			CDataHandlerKey key(GetCopyTableId());
			CSetVariantBases::const_iterator iter 
				= pTblCopyHelper->m_OrderVariants.find(static_cast<CDataHandler*>(&key));
			if(iter != pTblCopyHelper->m_OrderVariants.end())
				m_pOrderVariant = *iter;
			else if (m_VariantCreateFunc)
			{
				m_pOrderVariant = m_VariantCreateFunc();
				if (m_pOrderVariant)
				{
					m_pOrderVariant->SetParameters(pTblCopyHelper, GetCopyTableId());
					pTblCopyHelper->m_OrderVariants.insert(m_pOrderVariant);
				}
			}
		}
	}
	CHECK_ADDRESS(m_pOrderVariant);
	return m_pOrderVariant;
}

CDataHandler* COrderVariant::GetSlaveOrderVariantBase()
{
	if(OV_XLINK != (m_dwFlags & OV_KIND))
		return m_pOrderVariant;
	if(!m_pSlaveOV)
	{
		CTblCopyHelper* pTblCopyHelper = GetTblCopyHelper();
		if(pTblCopyHelper)
		{
			CDataHandlerKey key(GetTblSlaveTo());
			CSetVariantBases::const_iterator iter 
				= pTblCopyHelper->m_OrderVariants.find(static_cast<CDataHandler*>(&key));
			if (iter != pTblCopyHelper->m_OrderVariants.end())
				m_pSlaveOV = *iter;
			else
			{
				return NULL;
			}

		}
	}
	CHECK_ADDRESS(m_pSlaveOV);
	return m_pSlaveOV;
}


CDBTable* COrderVariant::GetTblSlaveFrom_() 
{
	return HandleCreateTable(
		m_idSlave,
		m_pTblSlFrom,
		GetTblCopyHelper()->GetHolderFrom());
}


CDBTable* COrderVariant::GetTblSlaveTo_() 
{
	return HandleCreateTable(
		m_idSlave,
		m_pTblSlTo,
		GetTblCopyHelper()->GetHolderTo());
}


LPCWSTR COrderVariant::GetEntityName() const
{
	return (OV_XLINK == (m_dwFlags & OV_KIND))?
		m_idMaster.GetTableName() : m_idSlave.GetTableName();
}


bool IsEqual(COrderVariant* pVar1, COrderVariant* pVar2)
{
	CHECK_ADDRESS(pVar1); CHECK_ADDRESS(pVar2);
	return pVar1->m_idMaster == pVar2->m_idMaster 
		&& pVar1->m_idSlave == pVar2->m_idSlave
		&& pVar1->m_nFieldOffset == pVar2->m_nFieldOffset;
}



bool COrderVariant::HasPrimaryKey()	{ return GetTblCopyTo()->HasPrimaryKey(); }


///////////////////////////////////////////////////////////////////////////////////

CDBTable* CDataProvider::GetTblCopyTo()
{
	return HandleCreateTable(
		GetCopyTableId(),
		m_pTblCopyTo,
		GetTblCopyHelper()->GetHolderTo());
}

CTblCopyHelper* CDataProvider::GetTblCopyHelper()
{
	CHECK_ADDRESS(m_pTblCopyHelper);
	return m_pTblCopyHelper; 
}

///////////////////////////////////////////////////////////////////////////////////

CTblCopyHelper* CDataHandler::GetTblCopyHelper()	
{ 
	CHECK_ADDRESS(m_pTblCopyHelper);
	return m_pTblCopyHelper; 
}

BOOL CDataHandler::DeleteRecord()
{ 
	return GetTblCopyTo()->DeleteRecord(); 
}

void CDataHandler::DeleteRecords(DWORD dwFilterType)
{ 
	GetTblCopyTo()->DeleteRecords(dwFilterType); 
}

BOOL CDataHandler::FindMatchByUI()	
{ 
	return GetTblCopyTo()->FindFirst(fltUniqueIndex); 
}

bool CDataHandler::SetUniqueName()
{
	class CMakeCopyName : public IStringIterator
	{
		const int m_nInstance;
	public:
		CMakeCopyName(int nInstance) : m_nInstance(nInstance) {}
		virtual bool VisitString(wstring& str) const
		{
			MakeCopyName(str, m_nInstance);
			return true;
		}
	};

	CTableHolder tempHolder;
	tempHolder.SetDBManager(GetTblCopyHelper()->GetHolderTo()->GetDBManager());
	CDBTable* pTblTemp = NULL;
	if(!HandleCreateTable(GetCopyTableId(), pTblTemp, &tempHolder))
		return false;
	int nInstance = -1;
	pTblTemp->CopyDataFromTable(GetTblCopyTo());
	while(pTblTemp->FindFirst(fltUniqueIndex))
	{
		pTblTemp->CopyDataFromTable(GetTblCopyTo());
		if(!pTblTemp->IterateThruStrings(CMakeCopyName(++nInstance), fltUniqueIndex))
			return false;
	}
	if(nInstance != -1)
	{
		CMakeCopyName makeName(nInstance);
		GetTblCopyTo()->IterateThruStrings(makeName, fltUniqueIndex);
	}
	return true;
}

CDBTable* CDataHandler::GetTblCopyTo()
{
	return HandleCreateTable(
		GetCopyTableId(),
		m_pTblCopyTo,
		GetTblCopyHelper()->GetHolderTo());
}

CDataProvider* CDataHandler::GetDataProvider()
{
	if(!m_pDataProvider)
	{
		CTblCopyHelper* pTblCopyHelper = GetTblCopyHelper();
		if(pTblCopyHelper)
		{
			CDataHandlerKey key(GetCopyTableId());
			CSetDataProviders::const_iterator iter 
				= pTblCopyHelper->m_DataProviders.find(static_cast<CDataProvider*>(&key));
			if(iter != pTblCopyHelper->m_DataProviders.end())
			{
				m_pDataProvider = *iter;
				SetCopyTableId(m_pDataProvider, GetCopyTableId());
			}
			else if(m_ProviderCreateFunc)
			{
				m_pDataProvider = m_ProviderCreateFunc();
				if(m_pDataProvider)
				{
					m_pDataProvider->SetParameters(pTblCopyHelper, GetCopyTableId());
					pTblCopyHelper->m_DataProviders.insert(m_pDataProvider);
				}
			}
		}
	}
	CHECK_ADDRESS(m_pDataProvider);
	return m_pDataProvider;
}


void CDataHandler::FreeStatements()						{ GetDataProvider()->FreeStatements(); }
BOOL CDataHandler::FindNextFrom()						{ return GetDataProvider()->FindNextFrom(); }
BOOL CDataHandler::FindByPrimaryKeyFrom(Identity lId)	{ return GetDataProvider()->FindByPrimaryKeyFrom(lId); }
Identity CDataHandler::GetFieldFrom(size_t nOffset) const		
	{ return GetDataProvider()->GetFieldFrom(nOffset); }
void CDataHandler::SetFieldFrom(size_t nOffset, Identity lId)		
	{ GetDataProvider()->SetFieldFrom(nOffset, lId); }
BOOL CDataHandler::FindFirstFrom(DWORD dwFilterType)	{ return GetDataProvider()->FindFirstFrom(dwFilterType); }
Identity CDataHandler::GetPrimaryKeyFrom()				{ return GetDataProvider()->GetPrimaryKeyFrom(); }
void CDataHandler::CopyData(bool bCopyPK)				{ GetDataProvider()->CopyData(bCopyPK); }


///////////////////////////////////////////////////////////////////////////////////

void CGenericDataProvider::FreeStatements() 
{ 
	GetTblCopyFrom()->FreeStatements(); 
}

BOOL CGenericDataProvider::FindNextFrom()		
{ 
	return GetTblCopyFrom()->FindNext(); 
}

BOOL CGenericDataProvider::FindByPrimaryKeyFrom(Identity lId) 
{ 
	return GetTblCopyFrom()->FindByPrimaryKey(lId); 
}

CDBTable* CGenericDataProvider::GetTblCopyFrom()
{ 
	return HandleCreateTable(
		GetCopyTableId(),
		m_pTblCopyFrom,
		GetTblCopyHelper()->GetHolderFrom());
}

Identity CGenericDataProvider::GetFieldFrom(size_t nOffset) const
{
	return GetIdentityValue(GetTblCopyFrom(), nOffset);
}

void CGenericDataProvider::SetFieldFrom(size_t nOffset, Identity lId)
{
	SetIdentityValue(GetTblCopyFrom(), nOffset, lId);
}

BOOL CGenericDataProvider::FindFirstFrom(DWORD dwFilterType)
{
	return GetTblCopyFrom()->FindFirst(dwFilterType);
}

Identity CGenericDataProvider::GetPrimaryKeyFrom()
{
	return GetTblCopyFrom()->GetPrimaryKey();
}

void CGenericDataProvider::CopyData(bool bCopyPK)
{
	GetTblCopyTo()->CopyDataFromTable(GetTblCopyFrom(), bCopyPK);
}

