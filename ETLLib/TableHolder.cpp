// TableHolder.cpp: implementation of the CTableHolder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TableHolder.h"


#include <algorithm>
#include "BellsMacrosX.h"
#include "DBTable.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTableHolder::CTableHolder()
{
	m_bReadOnly = false;
}

CTableHolder::~CTableHolder()
{
}

bool CTableHolder::HasSameDatabase(const CTableHolder& other) const
{
	ASSERT(GetDBManager() != NULL);
	ASSERT(other.GetDBManager() != NULL);
	return GetDBManager()->ConnectionString == other.GetDBManager()->ConnectionString;
}

bool CTableHolder::Lookup(LPCWSTR pszTableName, CDBTable*& rpDBTable) const
{
	CMapLPCWSTR2PDBTable::const_iterator iter = m_mapDBTables.find(pszTableName);
	if (iter != m_mapDBTables.end())
	{
		rpDBTable = iter->second.get();
		return true;
	}
	return false;
}

void CTableHolder::SetDBTable(LPCWSTR pszTableName, std::unique_ptr<CDBTable> pDBTable)
{
	m_mapDBTables[pszTableName] = std::move(pDBTable);
}

void CTableHolder::FreeStatements()
{
    for (const auto& r : m_mapDBTables)
        r.second->FreeStatements();
}

Identity CTableHolder::GetIdentity(LPCWSTR pszValue)
{
	Identity id;
	id.Set(pszValue, m_IdentityShared);
	return id;
}
