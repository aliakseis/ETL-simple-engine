// TableHolder.h: interface for the CTableHolder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TABLEFACTORY_H__BD32C68D_D3AE_11D6_9CE6_AEFF2E4B123A__INCLUDED_)
#define AFX_TABLEFACTORY_H__BD32C68D_D3AE_11D6_9CE6_AEFF2E4B123A__INCLUDED_

#pragma once

#include "identity.h"

#include <map>
#include <memory>

#ifdef ETLLIB_EXPORTS
#define ETLLIB_EXPORT __declspec(dllexport)
#else
#define ETLLIB_EXPORT __declspec(dllimport)
#endif

class CDBTable;


class ETLLIB_EXPORT CTableHolder
{
    CTableHolder(const CTableHolder&) = delete;
    CTableHolder& operator =(const CTableHolder&) = delete;

public:
	friend class CDBTable;

	CTableHolder();
	virtual ~CTableHolder();


	void SetDBManager(const _ConnectionPtr& pDBMan)
	{
		m_pConn = pDBMan;
	}

	_ConnectionPtr GetDBManager() const
	{
		return m_pConn;
	}

	bool HasSameDatabase(const CTableHolder& other) const;

	bool Lookup(LPCWSTR pszTableName, CDBTable*& rpDBTable) const;
	void SetDBTable(LPCWSTR pszTableName, std::unique_ptr<CDBTable> pDBTable);

	void FreeStatements();

	Identity GetIdentity(LPCWSTR pszValue);

	void SetReadOnly(bool bReadOnly)
	{
		m_bReadOnly = bReadOnly;
	}
	bool IsReadOnly() const
	{
		return m_bReadOnly;
	}

private:
	_ConnectionPtr m_pConn;

	struct CompareWStrings
	{
		bool operator() (LPCWSTR s1, LPCWSTR s2) const
		{
			return wcscmp(s1, s2) < 0;
		}
	};
	typedef std::map<LPCWSTR, std::unique_ptr<CDBTable>, CompareWStrings> CMapLPCWSTR2PDBTable;
	CMapLPCWSTR2PDBTable m_mapDBTables;

	CIdentityShared m_IdentityShared;

	bool m_bReadOnly;
};

#endif // !defined(AFX_TABLEFACTORY_H__BD32C68D_D3AE_11D6_9CE6_AEFF2E4B123A__INCLUDED_)
