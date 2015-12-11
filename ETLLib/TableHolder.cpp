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
	std::for_each(m_mapDBTables.begin(), m_mapDBTables.end(), DeleteSecond());
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
		rpDBTable = iter->second;
		return true;
	}
	return false;
}

void CTableHolder::SetDBTable(LPCWSTR pszTableName, CDBTable* pDBTable)
{
	m_mapDBTables[pszTableName] = pDBTable;
}


struct DoFreeStatements
{
	template<typename T> void operator ()(const T& r) const
	{
		r.second->FreeStatements();
	}
};


void CTableHolder::FreeStatements()
{
	std::for_each(m_mapDBTables.begin(), m_mapDBTables.end(), DoFreeStatements());
}

Identity CTableHolder::GetIdentity(LPCWSTR pszValue)
{
	Identity id;
	id.Set(pszValue, m_IdentityShared);
	return id;
}
