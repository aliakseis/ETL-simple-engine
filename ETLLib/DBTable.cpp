// DBTable.cpp: implementation of the CDBTable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DBTable.h"
#include "OrderVariant.h"

#pragma warning(disable:4018 4663)
#include <vector>
#pragma warning(default:4018 4663)

#include "TableHolder.h"

#define DBINITCONSTANTS
#include <sqloledb.h>

//#include <oledberr.h>

using std::wstring;

using _com_util::CheckError;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum { pkFilters = fltAutoNumber | fltPrimaryKey };

const VARIANT g_vNull = { VT_NULL };

HRESULT SafeArrayZeroVector(SAFEARRAY * psa)
{
	LONG iLBound, iUBound;
	HRESULT hr = SafeArrayGetLBound(psa, 1, &iLBound);
	if(FAILED(hr))
		return hr;
	hr = SafeArrayGetUBound(psa, 1, &iUBound);
	if(FAILED(hr))
		return hr;
	UINT nElemSize = SafeArrayGetElemsize(psa);

	void* pData = NULL;

	hr = SafeArrayAccessData(psa, (void**)&pData);
	if(FAILED(hr))
		return hr;

	memset(pData, 0, (iUBound - iLBound + 1) * nElemSize);

	return SafeArrayUnaccessData(psa);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDBTable::CDBTable(CTableHolder* pHolder)
: m_pHolder(pHolder)
, m_pstrTableName(NULL)
, m_nAtomCount(0)
, m_pAtomDesc(NULL)

, m_dwFilter(DWORD(-1L))
, m_dwDeleteFilter(DWORD(-1L))
, m_bAddMode(false)
, m_bFieldsListed(false)
{
}

CDBTable::~CDBTable()
{
	if (m_spFastLoad != NULL)
	{
		m_cda.ReleaseAccessors(m_spFastLoad);
	}
}

void CDBTable::InitData()
{
    for (auto& v : m_mapValues)
        v.second = g_vNull;
}

void CDBTable::FreeStatements()
{
	if (m_pRFind != NULL)
		m_pRFind->Close();

	if (m_spFastLoad != NULL)
	{
		VERIFY(SUCCEEDED(CComQIPtr<IRowsetFastLoad>(m_spFastLoad)->Commit(TRUE)));
		VERIFY(SUCCEEDED(m_cda.ReleaseAccessors(m_spFastLoad)));
	}
	m_cda.Close();
	m_spFastLoad.Release();

	m_pRFind = NULL;
	m_pCFind = NULL;
	m_pCAdd = NULL;
	m_pCId = NULL;
	m_pCDelete = NULL;

	m_dwFilter = DWORD(-1L);
	m_dwDeleteFilter = DWORD(-1L);
	m_bAddMode = false;
}

//////////////////////////////////////////////////////////////////////

LPCWSTR CDBTable::GetColumnName(DWORD filter) const
{
	for (int i = 0; i < GetAtomCount(); i++)
	{
		DWORD id = GetAtomId(i);
		if ((id & filter) == id)
			return GetAtomName(i);
	}
	return NULL;
}


void CDBTable::SetIdentityValue(DWORD id, Identity value)
{
	if (id != 0)
	{
		LPCWSTR colName = GetColumnName(id);
		ASSERT(colName);
        m_mapValues[colName] = value.Get(); // attach
    }
}

Identity CDBTable::GetIdentityValue(DWORD id) const
{
	LPCWSTR colName = GetColumnName(id);
	if (!colName)
		return ID_NOT_DEF;
    auto it = m_mapValues.find(colName);
    if (it == m_mapValues.end())
        return ID_NOT_DEF;
    Identity identity;
	identity.Set(it->second, GetIdentityShared());
	return identity;
}

Identity CDBTable::GetPrimaryKey() const
{
	return GetIdentityValue(pkFilters);
}

bool CDBTable::HasPrimaryKey() const
{
	return GetColumnName(pkFilters) != NULL;
}

bool CDBTable::HasUniqueFilter() const
{
	return GetColumnName(fltUniqueIndex) != NULL;
}


//////////////////////////////////////////////////////////////////////


bool CDBTable::FindFirst(DWORD filter)
{
	if (m_dwFilter != filter)
	{
		wstring query(L"SELECT * FROM ");
		query += GetTableName();
		if (filter != 0)
			if (filter < 0x00010000)
			{
				bool first = true;
				for (int i = 0; i < GetAtomCount(); i++)
				{
					DWORD id = GetAtomId(i);
					if ((id & filter) == id)
					{
						if (first)
						{
							query += L" WHERE ";
							first = false;
						}
						else 
							query += L" AND ";
						query += GetAtomName(i);
						query += L"=?";
					}
				}
			}
			else
			{
				query += L" WHERE ";
				USES_CONVERSION;
				query += T2CW((LPCTSTR) filter);
			}

		if(FAILED(m_pCFind.CreateInstance(__uuidof(Command))))
		{
			ASSERT(0);
			return false;
		}

		m_pCFind->ActiveConnection = GetConnectionPtr();
		
		TRACE(_T("%ls\n"), query.c_str());
		m_pCFind->CommandText = query.c_str();
		m_pCFind->Prepared = true;

		if (filter != 0 && filter < 0x00010000)
			for (int i = 0; i < GetAtomCount(); i++)
			{
				DWORD id = GetAtomId(i);
				if ((id & filter) == id)
				{
					MapValues::iterator iter = m_mapValues.find(GetAtomName(i));
					if (iter != m_mapValues.end())
					{
						DataTypeEnum param_type = adPropVariant;
						int param_size = 0;
						switch (V_VT(&iter->second))
						{
						case VT_I4: 
							param_type = adInteger;
							param_size = 4;
							break;
						case VT_BSTR:
							param_type = adBSTR;
							param_size = 255;
							break;
						default: ASSERT(0);
						}

						_ParameterPtr pprm = 
							m_pCFind->CreateParameter(iter->first.c_str(), param_type, 
								adParamInput, param_size, iter->second);
						m_pCFind->Parameters->Append(pprm);
					}
					else
						ASSERT(0);
				}
			}

		m_dwFilter = filter;

		// recordset
		if(FAILED(m_pRFind.CreateInstance(__uuidof(Recordset))))
		{
			ASSERT(0);
			return false;
		}

		m_pRFind->Open(_variant_t((IDispatch*)m_pCFind), vtMissing
			, IsReadOnly()? adOpenForwardOnly : adOpenKeyset
			, IsReadOnly()? adLockReadOnly : adLockOptimistic
			, adCmdText);
	}
	else 
	{
		for (int i = 0; i < GetAtomCount(); i++)
		{
			DWORD id = GetAtomId(i);
			if ((id & filter) == id)
			{
				MapValues::iterator iter = m_mapValues.find(GetAtomName(i));
				if (iter != m_mapValues.end())
				{
					m_pCFind->Parameters->Item[iter->first.c_str()]->Value = iter->second;
				}
			}
		}
		m_pRFind->Requery(adCmdText);
	}

	if (m_pRFind->adoEOF)
	{
		return false;
	}
	DoCopyDataFromTable(this);
	return true;
}

bool CDBTable::FindNext()
{
	if (m_pRFind == NULL || !SUCCEEDED(m_pRFind->MoveNext())
			|| m_pRFind->adoEOF)
		return false;
	DoCopyDataFromTable(this);
	return true;
}

bool CDBTable::FindByPrimaryKey(Identity lId)
{
	SetIdentityValue(pkFilters, lId);
	return FindFirst(pkFilters);
}


//////////////////////////////////////////////////////////////////////

void CDBTable::CopyDataFromTable(const CDBTable* pTable, 
								 bool bCopyPK /*= true*/)
{
	if (!m_bFieldsListed)
	{
		if (NULL == m_pRFind)
		{
			_RecordsetPtr pRParams = GetParamsRecordset();
            for (long i = pRParams->Fields->Count; i--; )
                m_mapValues[LPCWSTR(pRParams->Fields->Item[i]->Name)] = g_vNull;
			pRParams->Close();
		}
		m_bFieldsListed = true;
	}
	if (!pTable)
	{
		ASSERT(0); return;
	}

	LPCWSTR pstrPKName = NULL;
	if (!bCopyPK)
	{
		pstrPKName = GetColumnName(fltAutoNumber);
	}

    for (const auto& v : pTable->m_mapValues)
	{
		if (pstrPKName && v.first == pstrPKName)
			continue;
		MapValues::iterator iterDest = m_mapValues.find(v.first);
		if (iterDest != m_mapValues.end())
			iterDest->second = v.second;
	}
}

void CDBTable::DoCopyDataFromTable(const CDBTable* pTable, 
								 bool bCopyPK /*= true*/)
{
	if (!pTable)
	{
		ASSERT(0); return;
	}
	_RecordsetPtr pOtherRecordset = pTable->GetFindRecorset();
	if (NULL == pOtherRecordset)
	{
		ASSERT(0); return;
	}

	LPCWSTR pstrPKName = NULL;
	if (!bCopyPK)
	{
		pstrPKName = GetColumnName(fltAutoNumber);
	}

	FieldsPtr fields(pOtherRecordset->Fields);
	for (long i = fields->Count; i--; )
	{
		FieldPtr item = fields->Item[i];
		_bstr_t pstrItemName(item->GetName());
		if (!pstrItemName)
		{
			ASSERT(0);
			continue;
		}
		if (pstrPKName && !wcscmp(pstrItemName, pstrPKName))
			continue;
        m_mapValues[LPCWSTR(pstrItemName)] = item->Value;
    }
}

//////////////////////////////////////////////////////////////////////

bool CDBTable::DeleteRecord()
{
	try
	{
		return m_pRFind != NULL && SUCCEEDED(m_pRFind->Delete(adAffectCurrent));
	}
	catch (_com_error& e)
	{
		// Print COM errors. 
		TRACE("Error\n");
		TRACE("\tCode meaning = %s\n", e.ErrorMessage());
		TRACE("\tSource = %s\n", LPCTSTR(e.Source()));
		TRACE("\tDescription = %s\n", LPCTSTR(e.Description()));

		m_pRFind = NULL;
		m_dwFilter = DWORD(-1L);

		return false;
	}
}

bool CDBTable::DeleteRecords(DWORD filter)
{
	bool result = true;

	if (m_dwDeleteFilter != filter)
	{
		wstring query(L"DELETE FROM ");
		query += GetTableName();
		if (filter != 0)
		{
			bool first = true;
			for (int i = 0; i < GetAtomCount(); i++)
			{
				DWORD id = GetAtomId(i);
				if ((id & filter) == id)
				{
					if (first)
					{
						query += L" WHERE ";
						first = false;
					}
					else 
						query += L" AND ";
					query += GetAtomName(i);
					query += L"=?";
				}
			}
		}

		if(FAILED(m_pCDelete.CreateInstance(__uuidof(Command))))
		{
			ASSERT(0); return false;
		}

		m_pCDelete->ActiveConnection = GetConnectionPtr();
		
		TRACE(_T("%ls\n"), query.c_str());
		m_pCDelete->CommandText = query.c_str();
		m_pCDelete->Prepared = true;

		
		m_dwDeleteFilter = filter;
	}
	
	std::vector<const _variant_t*> params;
	for (int i = 0; i < GetAtomCount(); i++)
	{
		DWORD id = GetAtomId(i);
		if ((id & filter) == id)
		{
			MapValues::iterator iter = m_mapValues.find(GetAtomName(i));
			if (iter != m_mapValues.end())
			{
				params.push_back(&iter->second);
			}
		}
	}

	try
	{
		_variant_t vsa;
		// Create SafeArray Bounds and initialize the array
		if (params.size() > 0)
		{
			V_ARRAY(&vsa) = SafeArrayCreateVector( VT_VARIANT, 0, params.size() );
			vsa.vt = VT_VARIANT | VT_ARRAY;

			// Set the values for each element of the array
   			for(LONG i = params.size(); i--; )
				CheckError(SafeArrayPutElement(V_ARRAY(&vsa), &i, (void*) params[i])); 
		}
		m_pCDelete->Execute(NULL, (VT_EMPTY != vsa.vt)? &vsa : NULL
			, adCmdText | adExecuteNoRecords);
	}
	catch (_com_error& e)
	{
		result = false;

		// Print COM errors. 
		TRACE("Error\n");
		TRACE("\tCode meaning = %s\n", e.ErrorMessage());
		TRACE("\tSource = %s\n", LPCTSTR(e.Source()));
		TRACE("\tDescription = %s\n", LPCTSTR(e.Description()));
	}

	return result;
}

//////////////////////////////////////////////////////////////////////

bool CDBTable::WasModified() const
{
	if (NULL == m_pRFind)
	{
		ASSERT(0); return false;
	}

	for (long i = m_pRFind->Fields->Count; i--; )
	{
		FieldPtr item = m_pRFind->Fields->Item[i];
		_bstr_t bstrItemName(item->GetName());
		if (!bstrItemName)
		{
			ASSERT(0);
			continue;
		}
        auto it = m_mapValues.find(LPCWSTR(bstrItemName));
        if (it == m_mapValues.end())
        {
            if (item->Value.vt != VT_EMPTY)
                return true;
        }
		else if (!(V_VT(&it->second) & VT_ARRAY) && it->second != item->Value)
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////

bool CDBTable::AddRecord(bool bInitialize /*= true*/)
{
	if (bInitialize)
		InitData();
	m_bAddMode = true;
	return true;
}

bool CDBTable::EditRecord()
{
	ASSERT(m_pRFind != NULL);
	m_bAddMode = false;
	return true;
}

bool CDBTable::Update(bool canBatch /*= false*/)
{
	LPCWSTR pstrPKName = GetColumnName(fltAutoNumber);
	if (m_bAddMode)
	{
		m_bAddMode = false;
		if (m_spFastLoad != NULL 
				|| canBatch && m_pCAdd == NULL 
					&& !pstrPKName && CreateFastLoadCommand())
			return FastInsert();
		else
			return OrdinaryInsert();
	}
	else
	{
		if (NULL == m_pRFind)
		{
			ASSERT(0); return false;
		}
		for (MapValues::iterator iter = m_mapValues.begin()
		; iter != m_mapValues.end()
		; ++iter)
		{
			if (pstrPKName && iter->first == pstrPKName)
				continue;
			m_pRFind->Fields->Item[iter->first.c_str()]->Value = iter->second;
		}
		return SUCCEEDED(m_pRFind->Update());
	}
}


bool CDBTable::OrdinaryInsert()
{
	LPCWSTR pstrPKName = GetColumnName(fltAutoNumber);
	bool result = false;
	for (bool second = false;; second = true)
	{
		if (NULL == m_pCAdd)
		{
			if (FAILED(m_pCAdd.CreateInstance(__uuidof(Command))))
			{
				ASSERT(0); return false;
			}
			m_pCAdd->ActiveConnection = GetConnectionPtr();
			m_pCAdd->Prepared = true;

			wstring query(L"INSERT INTO " + wstring(GetTableName()) + L" ( ");
			wstring values;
			bool first = true;
			_RecordsetPtr pRParams;

            for (auto& v : m_mapValues)
			{
				if (pstrPKName && v.first == pstrPKName)
					continue;
				if (first)
				{
					first = false;
				}
				else {
					query += L", ";
					values += L",?";
				}
				query += v.first;

				if (NULL == pRParams)
					pRParams = (m_pRFind != NULL)? m_pRFind
											: GetParamsRecordset();

				FieldPtr item = pRParams->Fields->Item[v.first.c_str()];
				DataTypeEnum type = item->Type;
				_ParameterPtr pprm = m_pCAdd->CreateParameter(v.first.c_str(), 
					type, adParamInput, 
					(adLongVarChar == type || adLongVarWChar == type)? 1 : item->DefinedSize);
				pprm->NumericScale = item->NumericScale;
				pprm->Precision = item->Precision;
				m_pCAdd->Parameters->Append(pprm);
			}
			if (pRParams != NULL && NULL == m_pRFind)
				pRParams->Close();

			query += L" ) VALUES ( ?" + values + L" )";
			if (pstrPKName && IsSqlServerDatabase())
				query += L"; SELECT SCOPE_IDENTITY()";

			TRACE(_T("%ls\n"), query.c_str());
			m_pCAdd->CommandText = query.c_str();
		}

		// Initialize and fill the SafeArray
		_variant_t vsa;
		// Create SafeArray Bounds and initialize the array
		V_ARRAY(&vsa) = SafeArrayCreateVector( VT_VARIANT, 0,  m_mapValues.size());
		vsa.vt = VT_VARIANT | VT_ARRAY;

		VARIANT* pParams = NULL;
		CheckError(SafeArrayAccessData(V_ARRAY(&vsa), (void**)&pParams));
		size_t i = 0;
        for (auto& v : m_mapValues)
        {
			if (pstrPKName && v.first == pstrPKName)
				continue;

			if ((VT_ARRAY | VT_UI1) == V_VT(&v.second))
			{
				m_pCAdd->Parameters->Item[v.first.c_str()]->Size
					= V_ARRAY(&v.second)->rgsabound[0].cElements + 1;
			}

			pParams[i++] = v.second;
		}
		CheckError(SafeArrayUnaccessData(V_ARRAY(&vsa)));
		if (i != m_mapValues.size())
		{
			SAFEARRAYBOUND saboundNew = { i, 0 };
			CheckError(SafeArrayRedim(V_ARRAY(&vsa), &saboundNew));
		}

		_variant_t count;
		_RecordsetPtr res;
		HRESULT hr = m_pCAdd->raw_Execute(&count, &vsa, 0, &res);

		CheckError(SafeArrayZeroVector(V_ARRAY(&vsa)));

		if (FAILED(hr))
			if (second)
				_com_issue_errorex(hr, m_pCAdd, __uuidof(m_pCAdd));
			else
			{// Possibly deadlock
				FreeStatements();
				continue;
			}

		result = (1 == long(count));

		// Identity matters
		if (pstrPKName != NULL) 
		{
			if (IsAccessDatabase())
			{
				if (NULL == m_pCId)
				{
					if (FAILED(m_pCId.CreateInstance(__uuidof(Command))))
					{
						ASSERT(0); return false;
					}
					m_pCId->ActiveConnection = GetConnectionPtr();
					m_pCId->Prepared = true;
					m_pCId->CommandText = L"SELECT @@IDENTITY";
				}
				//_RecordsetPtr 
				res = m_pCId->Execute(NULL, NULL, 0);
				if (res->adoEOF) 
				{
					ASSERT(0); return false;
				}
			}
			else if (IsSqlServerDatabase())
			{
				_variant_t vtAffected;
				res = res->NextRecordset(&vtAffected);
			}
			m_mapValues[pstrPKName] = res->Fields->Item[0L]->Value;
		}
		break;
	}
	return result;
}


bool CDBTable::FastInsert()
{
	for (ULONG ulColumn = 1; ulColumn <= m_cda.GetColumnCount(); ++ulColumn)
	{
		LPOLESTR pstrColumnName = m_cda.GetColumnName(ulColumn);
		MapValues::iterator iter(m_mapValues.find(pstrColumnName));
		if (iter == m_mapValues.end())
			VERIFY(m_cda.SetStatus(ulColumn, DBSTATUS_S_ISNULL));
		else
		{
			const _variant_t& val = iter->second;
			if(VT_NULL == V_VT(&val))
				VERIFY(m_cda.SetStatus(ulColumn, DBSTATUS_S_ISNULL));
			else
			{
				VERIFY(m_cda.SetStatus(ulColumn, DBSTATUS_S_OK));
				DBTYPE columnType = 0;
				VERIFY(m_cda.GetColumnType(ulColumn, &columnType));

				ULONG ulColumnOffset = ulColumn;
				VERIFY(m_cda.TranslateColumnNo(ulColumnOffset));
				void* pDataBuffer = m_cda._GetDataPtr(ulColumnOffset);

				int cbWrite = 0;
				switch (columnType)
				{
				case DBTYPE_WSTR:
					{
						if (V_VT(&val) != VT_BSTR)
						{
							ASSERT(0);
							return false;
						}
						const size_t ulLength = sizeof(wchar_t) * wcslen(V_BSTR(&val));
						if (ulLength >= m_cda.m_pColumnInfo[ulColumnOffset].ulColumnSize)
						{
							ASSERT(0);
							return false;
						}
						VERIFY(m_cda.SetLength(ulColumn, ulLength));
						wcscpy_s(
                            (LPWSTR) pDataBuffer, 
                            m_cda.m_pColumnInfo[ulColumnOffset].ulColumnSize / sizeof(wchar_t), 
                            V_BSTR(&val));
					}
					continue;
				case DBTYPE_STR:
					{
						if (V_VT(&val) != VT_BSTR)
						{
							ASSERT(0);
							return false;
						}
						USES_CONVERSION;
						LPCSTR pszVal = W2CA(V_BSTR(&val));
						const size_t ulLength = strlen(pszVal);
						if (ulLength >= m_cda.m_pColumnInfo[ulColumnOffset].ulColumnSize)
						{
							ASSERT(0);
							return false;
						}
						VERIFY(m_cda.SetLength(ulColumn, ulLength));
						strcpy_s((LPSTR) pDataBuffer, m_cda.m_pColumnInfo[ulColumnOffset].ulColumnSize, pszVal);
					}
					continue;
				case VT_UI1:
				case VT_I1:
					cbWrite = sizeof(BYTE);
					break;
				case VT_I2:
				case VT_UI2:
					cbWrite = sizeof(short);
					break;
				case VT_BOOL:
					cbWrite = sizeof(VARIANT_BOOL);
					break;
				case VT_I4:
				case VT_UI4:
				case VT_R4:
				case VT_INT:
				case VT_UINT:
				case VT_ERROR:
					cbWrite = sizeof(long);
					break;
				case VT_R8:
				case VT_CY:
				case VT_DATE:
					cbWrite = sizeof(double);
					break;
				default:
					ASSERT(0);
					return false;
				}

				_variant_t converted;
				converted.ChangeType(columnType, &val);

				VERIFY(m_cda.SetLength(ulColumn, cbWrite));
				memcpy(pDataBuffer, &converted.lVal, cbWrite);			
			}
		}
	}

	HRESULT hr = CComQIPtr<IRowsetFastLoad>(m_spFastLoad)->InsertRow(
				m_cda.GetHAccessor(0),
				m_cda.GetBuffer());
	CheckError(hr);
	return true;
}

//////////////////////////////////////////////////////////////////////

int CDBTable::IterateThruStrings(const IStringIterator& it, DWORD filter)
{
	int cnt = 0;

	for (int i = 0; i < GetAtomCount(); i++)
	{
		DWORD id = filter? GetAtomId(i) : 0;
		if ((id & filter) == id)
		{
			MapValues::iterator iter = m_mapValues.find(GetAtomName(i));
			if (iter != m_mapValues.end())
			{
				const _variant_t& val = iter->second;
				if (VT_BSTR == val.vt)
				{
					wstring buf(val.bstrVal);
					if (!it.VisitString(buf))
						return 0;
					iter->second = buf.c_str();
					++cnt;
				}
			}
		}
	}

	return cnt;
}

bool CDBTable::IsAccessDatabase()
{
	return !wcscmp(L"MS Jet", GetConnectionPtr()->Properties->Item[L"DBMS Name"]->Value.bstrVal);
}

bool CDBTable::IsSqlServerDatabase()
{
	return !wcscmp(L"Microsoft SQL Server", GetConnectionPtr()->Properties->Item[L"DBMS Name"]->Value.bstrVal);
}

_RecordsetPtr CDBTable::GetParamsRecordset()
{
	_RecordsetPtr pRParams;
	if(FAILED(pRParams.CreateInstance(__uuidof(Recordset))))
	{
		ASSERT(0);
		return NULL;
	}
	wstring query(L"SELECT * FROM " + wstring(GetTableName()) + L" WHERE 0=1");
	pRParams->Open(query.c_str(), _variant_t((IDispatch*)GetConnectionPtr()), adOpenForwardOnly, adLockReadOnly, 0);
	return pRParams;
}


bool CDBTable::CreateFastLoadCommand()
{
	ADOConnectionConstruction15Ptr adoConnectionConstruction(GetConnectionPtr());
	if (adoConnectionConstruction == NULL)
		return false;

	CComQIPtr<IOpenRowset> spOpenRowset(adoConnectionConstruction->GetSession());
	if (spOpenRowset == NULL)
		return false;

	DBID dbid;
	dbid.eKind = DBKIND_NAME;
	dbid.uName.pwszName = const_cast<LPOLESTR>(GetTableName());

	DBPROP rgRowsetProps[1];
	DBPROPSET RowsetPropSet;

	VariantInit(&rgRowsetProps[0].vValue);

	rgRowsetProps[0].dwOptions = DBPROPOPTIONS_REQUIRED; 
	rgRowsetProps[0].colid = DB_NULLID; 
	rgRowsetProps[0].dwStatus = DBPROPSTATUS_OK; 
	rgRowsetProps[0].dwPropertyID = SSPROP_IRowsetFastLoad; 
	rgRowsetProps[0].vValue.vt = VT_BOOL; 
	rgRowsetProps[0].vValue.boolVal = VARIANT_TRUE;

	RowsetPropSet.rgProperties = rgRowsetProps; 
	RowsetPropSet.cProperties = 1; 
	RowsetPropSet.guidPropertySet = DBPROPSET_SQLSERVERROWSET;

	CComPtr<IUnknown> spFastLoad;

	HRESULT hr = spOpenRowset->OpenRowset(NULL, &dbid, NULL, 
			IID_IRowsetFastLoad,
			1, &RowsetPropSet, 
			&spFastLoad);
	if (FAILED(hr))
		return false;

	hr = m_cda.BindColumns(spFastLoad);
	if (FAILED(hr))
		return false;

	m_spFastLoad.Attach(spFastLoad.Detach());
	return true;
}
