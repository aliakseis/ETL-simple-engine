#include "stdafx.h"

#include "CopyHelper.h"

#pragma warning(disable:4100)
#include <algorithm>
#pragma warning(default:4100)

#include "BellsMacrosX.h"
#include "DBTable.h"
#include "TableHolder.h"

#include <sstream>


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

CCopyIterator& CCopyIterator::operator = (const CCopyIterator& other)
{
	Clear();

    switch (other.m_ciKind)
    {
    case ciSingle: new (&m_pair) Pair(other.m_pair); break;
    case ciMultiple: new (&m_parrSubstRec) CSubstRecArrayPtr(other.m_parrSubstRec); break;
    case ciByPK: m_parrId = other.m_parrId; break;
    }

    m_ciKind = other.m_ciKind;

	return *this;
}

void CCopyIterator::Clear()
{
    switch (m_ciKind)
    {
    case ciSingle: m_pair.~Pair(); break;
    case ciMultiple: m_parrSubstRec.~CSubstRecArrayPtr(); break;
    case ciByPK: m_parrId = nullptr; break;
    }
    m_ciKind  = ciNotDef;
}


void CCopyIterator::SetData(Identity lValueTo, Identity lValueFrom)
{
	Clear();
    new (&m_pair) Pair(lValueTo, lValueFrom);
	m_ciKind = ciSingle;
}

void CCopyIterator::SetData(const CSubstRecArrayPtr& parrSubstRec)
{
	Clear();
    new (&m_parrSubstRec) CSubstRecArrayPtr(parrSubstRec);
	m_ciKind = ciMultiple;
}

void CCopyIterator::SetData(const std::deque<Identity>* parrId)
{
	Clear();
	m_parrId = parrId;
    m_ciKind = ciByPK;
}

void CCopyIterator::SetData(Identity lPK)
{
	Clear();
    new (&m_pair) Pair(ID_NOT_DEF, lPK);
    m_ciKind = ciSingle;
}

void CCopyIterator::AddData(CSubstRecArrayPtr& parrSubstRec)
{
	if(m_ciKind != ciMultiple)
	{
		ASSERT(0); return;
	}
//	Copy on write
    if (!m_parrSubstRec.unique())
    {
        m_parrSubstRec = std::make_shared<CSubstRecArray>(*m_parrSubstRec);
    }
    m_parrSubstRec->insert(m_parrSubstRec->end(), parrSubstRec->begin(), parrSubstRec->end());
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
		if(IsValid(GetFieldFollowerFrom()))
		{
			CMapIdentities::iterator it = m_pMapId->find(GetFieldFollowerFrom());
			if (it != m_pMapId->end())
				SetFieldFollowerTo(it->second);
			else
			{
				if(IsConvertOnly())
				{
					SetFieldFollowerTo(ID_NULL);
					return TRUE;
				}
				else if(GetTblCopyHelper()->Convert(this))
				{
					if(IsValid(GetFieldFollowerTo()))
					{
						m_pMapId->SetAt(GetFieldFollowerFrom(), GetFieldFollowerTo());
						GetOrderVariantBase()->OnRefAdded(GetFieldFollowerTo());
					}
				}
				else
					return FALSE;
			}
		}
		else
			SetFieldFollowerTo(GetFieldFollowerFrom());
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

bool COrderVariant::SetUniqueName(int nInstance)			
                                    { return GetOrderVariantBase()->SetUniqueName(nInstance); }


Identity	COrderVariant::GetFieldFollowerTo()	 const
{ 
	return GetIdentityValue(const_cast<COrderVariant*>(this)->GetTblFollowerTo_(), GetFieldOffset());
} 

Identity	COrderVariant::GetFieldFollowerFrom()  const
{ 
	const CDataHandler* pBase 
		= const_cast<COrderVariant*>(this)->GetFollowerOrderVariantBase();
	return (pBase)?
		pBase->GetFieldFrom(GetFieldOffset())
		: GetIdentityValue(const_cast<COrderVariant*>(this)->GetTblFollowerFrom_(), GetFieldOffset());
} 

void	COrderVariant::SetFieldFollowerTo(Identity lId)
{ 
	CHECK_ADDRESS(this);
	SetIdentityValue(GetTblFollowerTo_(), GetFieldOffset(), lId);
} 

void	COrderVariant::SetFieldFollowerFrom(Identity lId)
{ 
	CDataHandler* pBase = GetFollowerOrderVariantBase();
	if(pBase)
		pBase->SetFieldFrom(GetFieldOffset(), lId);
	else
		SetIdentityValue(GetTblFollowerFrom_(), GetFieldOffset(), lId);
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

CDataHandler* COrderVariant::GetFollowerOrderVariantBase()
{
	if(OV_XLINK != (m_dwFlags & OV_KIND))
		return m_pOrderVariant;
	if(!m_pFollowerOV)
	{
		CTblCopyHelper* pTblCopyHelper = GetTblCopyHelper();
		if(pTblCopyHelper)
		{
			CDataHandlerKey key(GetTblFollowerTo());
			CSetVariantBases::const_iterator iter 
				= pTblCopyHelper->m_OrderVariants.find(static_cast<CDataHandler*>(&key));
			if (iter != pTblCopyHelper->m_OrderVariants.end())
				m_pFollowerOV = *iter;
			else
			{
				return NULL;
			}

		}
	}
	CHECK_ADDRESS(m_pFollowerOV);
	return m_pFollowerOV;
}


CDBTable* COrderVariant::GetTblFollowerFrom_() 
{
	return HandleCreateTable(
		m_idFollower,
		m_pTblSlFrom,
		GetTblCopyHelper()->GetHolderFrom());
}


CDBTable* COrderVariant::GetTblFollowerTo_() 
{
	return HandleCreateTable(
		m_idFollower,
		m_pTblSlTo,
		GetTblCopyHelper()->GetHolderTo());
}


LPCWSTR COrderVariant::GetEntityName() const
{
	return (OV_XLINK == (m_dwFlags & OV_KIND))?
		m_idMaster.GetTableName() : m_idFollower.GetTableName();
}


bool IsEqual(COrderVariant* pVar1, COrderVariant* pVar2)
{
	CHECK_ADDRESS(pVar1); CHECK_ADDRESS(pVar2);
	return pVar1->m_idMaster == pVar2->m_idMaster 
		&& pVar1->m_idFollower == pVar2->m_idFollower
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

bool CDataHandler::SetUniqueName(int nInstance)
{
    return !!GetTblCopyTo()->IterateThruStrings([nInstance](wstring& str) {
        MakeCopyName(str, nInstance);
        return true;
    }, fltUniqueIndex);
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

