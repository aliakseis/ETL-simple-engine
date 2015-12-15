// DBTable.h: interface for the CDBTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBTABLE_H__BD32C68F_D3AE_11D6_9CE6_AEFF2E4B123A__INCLUDED_)
#define AFX_DBTABLE_H__BD32C68F_D3AE_11D6_9CE6_AEFF2E4B123A__INCLUDED_



#pragma once


#include "identity.h"

#pragma warning(disable:4100 4702)
#include <map>
#include <string>
#pragma warning(default:4100 4702)

#include <atldbcli.h>

#include "TableHolder.h"
#include "SharedComVariant.h"

#include <memory>
#include <functional>


#ifdef ETLLIB_EXPORTS
#define ETLLIB_EXPORT __declspec(dllexport)
#else
#define ETLLIB_EXPORT __declspec(dllimport)
#endif


struct AtomDesc
{
	DWORD	m_id;
	LPCWSTR	m_name;
};


class ETLLIB_EXPORT CDBTable  
{
public:
	CDBTable(CTableHolder* pHolder);
	virtual ~CDBTable();

	LPCWSTR GetTableName() const { return m_pstrTableName; }
	Identity GetPrimaryKey() const;
	bool HasPrimaryKey() const;
	bool HasUniqueFilter() const;

	bool WasModified() const;

	void InitData();
	void FreeStatements();
	bool DeleteRecord();
	bool DeleteRecords(DWORD filter);
	bool FindFirst(DWORD filter);
	bool FindByPrimaryKey(Identity lId);
	bool FindNext();

	bool AddRecord(bool bInitialize = true);
	bool EditRecord();
	bool Update(bool canBatch = false);

	void CopyDataFromTable(const CDBTable* pOther, bool bCopyPK = true);
	int IterateThruStrings(std::function<bool(std::wstring&)> iter, DWORD filter);

	void SetIdentityValue(DWORD id, Identity value);
	Identity GetIdentityValue(DWORD id) const;

protected:
	int GetAtomCount() const			{ return m_nAtomCount; }
	DWORD GetAtomId(int idx) const		{ return m_pAtomDesc[idx].m_id; }
	LPCWSTR GetAtomName(int idx) const	{ return m_pAtomDesc[idx].m_name; }

	LPCWSTR GetColumnName(DWORD id) const;

	_ConnectionPtr& GetConnectionPtr() const { return m_pHolder->m_pConn; }

	CIdentityShared& GetIdentityShared() const { return m_pHolder->m_IdentityShared; }

	_RecordsetPtr GetFindRecorset() const { return m_pRFind; }

	void DoCopyDataFromTable(const CDBTable* pTable, bool bCopyPK = true);

	bool IsAccessDatabase();
	bool IsSqlServerDatabase();

	_RecordsetPtr GetParamsRecordset();

	bool IsReadOnly() const
	{
		return m_pHolder->IsReadOnly();
	}
	bool OrdinaryInsert();
	bool FastInsert();
	bool CreateFastLoadCommand();


	LPCWSTR m_pstrTableName;
	int m_nAtomCount;
	const AtomDesc* m_pAtomDesc;

private:
	CTableHolder* m_pHolder;

	_CommandPtr m_pCFind;
	_RecordsetPtr m_pRFind;

	_CommandPtr m_pCAdd;

	_CommandPtr m_pCId;

	_CommandPtr m_pCDelete;

	DWORD m_dwFilter;
	DWORD m_dwDeleteFilter;

	typedef std::map<std::wstring, SharedComVariant> MapValues;
	MapValues m_mapValues;

	bool m_bAddMode;
	bool m_bFieldsListed;

	CComPtr<IUnknown> m_spFastLoad;
    CDynamicAccessor m_cda;
};

#endif // !defined(AFX_DBTABLE_H__BD32C68F_D3AE_11D6_9CE6_AEFF2E4B123A__INCLUDED_)
