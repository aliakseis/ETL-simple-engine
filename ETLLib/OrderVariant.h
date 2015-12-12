#ifndef __OrderVariant_h__
#define __OrderVariant_h__


#include "BellsMacrosX.h"

#ifdef ETLLIB_EXPORTS
#define ETLLIB_EXPORT __declspec(dllexport)
#else
#define ETLLIB_EXPORT __declspec(dllimport)
#endif


class CTblCopyHelper;
class CDBTable;
class CDBAttrubuteDesc;

#include "identity.h"

#include "TableHolder.h"

#pragma warning(push, 3)
#include <string>
#pragma warning(disable:4284 4018 4146 4702)
#include <deque>
#pragma warning(pop)


#include <google/dense_hash_map>

#pragma warning(disable:4097)


class CMapIdentities : public GOOGLE_NAMESPACE::dense_hash_map<Identity, Identity>
{
public:
	CMapIdentities()
	{
		set_empty_key(Identity());
	}
	void SetAt(const Identity& key, const Identity& value)
	{
		insert(value_type(key, data_type())).first->second = value;
	}
};

struct CSubstRec
{
	Identity m_lKeyTo, m_lKeyFrom;
	CSubstRec() 
	{
		m_lKeyTo = ID_NOT_DEF;
		m_lKeyFrom = ID_NOT_DEF;
	}
	CSubstRec(Identity lKeyTo, Identity lKeyFrom)
	{
		m_lKeyTo = lKeyTo;
		m_lKeyFrom = lKeyFrom;
	}
};

class CSubstRecArray : public std::deque<CSubstRec>
{
	size_t m_nRefs;
public:    
	CSubstRecArray()	{ m_nRefs = 0; }
	~CSubstRecArray()	{ ASSERT(0 == m_nRefs); }
	void upcount()		{ ++m_nRefs; }
	void downcount()	{ if (--m_nRefs == 0) delete this; }
	int GetNumRefs()	{ return m_nRefs; }

	void Append(const CSubstRecArray& other)
	{
		insert(end(), other.begin(), other.end());
	}
};

template <class T> class Ptr 
{    
	T* p;
public:
	Ptr(T* p_) : p(p_) { p->upcount(); }    
	~Ptr()			{ p->downcount(); }
	operator T*()	{ return p; }    
	T& operator*() { return *p; }
	T* operator->(){ return p; }    
	Ptr& operator=(Ptr<T> &p_) { return operator=((T *) p_); }    
	Ptr& operator=(T* p_) 
	{
		p_->upcount(); 
		p->downcount(); 
		p = p_; 

		return *this;    
	}
};

typedef Ptr<CSubstRecArray> CSubstRecArrayPtr;

class CCopyIteratorData
{
protected:
	enum
	{
		ciNotDef = 0,
		ciSingle,
		ciMultiple,
		ciByPK
	}	
	m_ciKind;

	struct Pair
	{
		Identity m_lValueTo, m_lValueFrom;
		Pair() : m_lValueTo(ID_NOT_DEF), m_lValueFrom(ID_NOT_DEF) {}
	} 
	m_pair;

	union
	{
		CSubstRecArray* m_parrSubstRec;
		std::deque<Identity> * m_parrId;
	};

public:
	CCopyIteratorData()
	{
		m_ciKind = ciNotDef;
		m_parrSubstRec = NULL;
	}
};

class ETLLIB_EXPORT CCopyIterator : public CCopyIteratorData
{
public:
	CCopyIterator()	{ m_ciKind  = ciNotDef; }
	CCopyIterator(const CCopyIterator& other);
	CCopyIterator(CSubstRecArrayPtr& parrSubstRec)
	{
		SetData(parrSubstRec);
	}
	CCopyIterator(std::deque<Identity>* parrId)
	{
		SetData(parrId);
	}
	CCopyIterator(Identity lPK)
	{
		SetData(lPK);
	}
	CCopyIterator(Identity lValueTo, Identity lValueFrom)
	{
		SetData(lValueTo, lValueFrom);
	}
	~CCopyIterator() { Clear(); }
	const CCopyIterator& operator = (const CCopyIterator& other);
	void Clear();
	void SetData(Identity lValueTo, Identity lValueFrom);
	void SetData(CSubstRecArrayPtr& parrSubstRec);
	void SetData(std::deque<Identity>* parrId);
	void SetData(Identity lPK);
	void AddData(CSubstRecArrayPtr& parrSubstRec);
	BOOL ByPK();
	Identity GetValueTo();
	Identity GetValueFrom();
	Identity GetValueTo(int nIndex);
	Identity GetValueFrom(int nIndex);
	int GetSize();
};


#define OV_ENTRY				1
#define OV_LINK					2
#define OV_XLINK				3
#define OV_KIND					3


#define OV_LEAVE_UNIQUE			8
#define OV_LEAVE_DATA			16
#define OV_LEAVE_KIND			24

#define OV_NOT_SERIAL			32
#define OV_SERIAL				64
#define OV_SERIAL_KIND			96

#define OV_CONVERT_ONLY			128

#define OV_BY_REFERENCE			256

#define OV_UPDATE_TEMPL_REF	512

#define OV_DONT_COPY_REF		1024
#define OV_DONT_SHORTCUT_REF	2048

#define OV_USE_EXISTING			0
#define OV_ASK_CHOICE			4096
#define OV_CREATE_NEW			8192
#define OV_OVERWRITE			(OV_ASK_CHOICE | OV_CREATE_NEW)

#define OV_INITIALIZED			0x4000
#define OV_ABANDON_DEPENDANTS	0x8000

enum LeaveKind
{
	lkDeleteAll		= 0,
	lkLeaveUnique	= OV_LEAVE_UNIQUE,
//	lkLeaveUnique means don't delete recs at all; if record matching by UI is found, use it.
	lkLeaveData		= OV_LEAVE_DATA
//	lkLeaveData means don't delete recs at all; add new records always w/o checking by UI.
};

enum SerialKind
{
	skNotDefined	= 0,
	skNotSerial		= OV_NOT_SERIAL,
	skSerial			= OV_SERIAL
};

enum UIChoiceKind
{
	uiAskChoice		= OV_ASK_CHOICE,
	uiUseExisting	= OV_USE_EXISTING,// by default
	uiCreateNew		= OV_CREATE_NEW,
	uiOverwrite		= OV_OVERWRITE
};


void MakeCopyName(std::wstring& strName, int nInstance);


typedef CDBTable* (*TableObjCreator) (CTableHolder*);

class ETLLIB_EXPORT CTableId
{
public:
	CTableId() 
	{ 
		m_pfnCreator = NULL; 
		m_pszTableName = NULL; 
	}
	CTableId(TableObjCreator pfnCreator,
				LPCWSTR pszTableName
				)
	: m_pfnCreator(pfnCreator)
	, m_pszTableName(pszTableName)
	{
	}
	bool operator == (const CTableId& other) const;
	bool operator != (const CTableId& other) const
	{
		return !(*this == other);
	}
	bool operator < (const CTableId& other) const;
	bool operator == (const CDBTable* other) const;
	bool operator != (const CDBTable* other) const
	{
		return !(*this == other);
	}

	LPCWSTR GetTableName() const { return m_pszTableName; }
	bool operator ! () const	  { return NULL == m_pszTableName; }
	CDBTable* GetDBTable(CTableHolder* pHolder) const;

private:
	TableObjCreator m_pfnCreator;
	LPCWSTR	m_pszTableName;
};

template<class T> class CreateTableId
{
	LPCWSTR m_pszTableName;

	static CDBTable* CreateTable (CTableHolder* pHolder)
	{
		return new T(pHolder);
	}

public:
	CreateTableId()
		: m_pszTableName(T::g_szTableName)
	{}
	operator const CTableId&()
	{
		static CTableId id(CreateTable, m_pszTableName);
		return id;
	}
};



#define BOOL_PROP(Name, Mask)			\
bool Is##Name() const					\
	{ return !!(m_dwFlags & Mask); }	\
void Set##Name(bool bValue = true)		\
{										\
	if(bValue)							\
		m_dwFlags |= Mask;				\
	else								\
		m_dwFlags &= ~Mask;				\
}

class COrderVariant;


class CDataHandlerKey
{
public:
	CDataHandlerKey() {}
	CDataHandlerKey(const CTableId& id) : m_id(id) {}
	CTableId	GetCopyTableId() const { return m_id; }

protected:
	static void SetCopyTableId(CDataHandlerKey* pKey, const CTableId& id)
	{
		pKey->m_id = id;
	}
	CTableId		m_id;
};



class ETLLIB_EXPORT CDataProvider
: public CDataHandlerKey
{
public:
	CDataProvider()
	{
		m_pTblCopyHelper	= NULL;
		m_pTblCopyTo		= NULL;
	}
	virtual ~CDataProvider() {}

	void SetParameters(CTblCopyHelper* pTblCopyHelper,
				const CTableId&	id)	
	{ 
		m_pTblCopyHelper	= pTblCopyHelper; 
		m_id = id;
	}
	CTblCopyHelper* GetTblCopyHelper();

	virtual void FreeStatements() = 0;

	virtual BOOL FindNextFrom() = 0;
	virtual BOOL FindByPrimaryKeyFrom(Identity lId) = 0;

	virtual Identity GetFieldFrom(size_t) const = 0;
	virtual void SetFieldFrom(size_t, Identity) = 0;


	virtual BOOL FindFirstFrom(DWORD dwFilterType) = 0;
	virtual Identity GetPrimaryKeyFrom() = 0;
	virtual void CopyData(bool bCopyPK) = 0;

	virtual void FirstSubType()	{}	//	To be used also for lazy initialization

protected:
	CDBTable* GetTblCopyTo();
	const CDBTable* GetTblCopyTo() const 	
	{ 
		return const_cast<CDataProvider*>(this)->GetTblCopyTo(); 
	}

private:
	CTblCopyHelper* m_pTblCopyHelper;
	CDBTable* m_pTblCopyTo;
};

class ETLLIB_EXPORT CGenericDataProvider
: public CDataProvider
{
public:
	CGenericDataProvider()
	{
		m_pTblCopyFrom		= NULL;
	}

	virtual void FreeStatements();

	virtual BOOL FindNextFrom();
	virtual BOOL FindByPrimaryKeyFrom(Identity lId);

	virtual Identity GetFieldFrom(size_t) const;
	virtual void SetFieldFrom(size_t, Identity);


	virtual BOOL FindFirstFrom(DWORD dwFilterType);
	virtual Identity GetPrimaryKeyFrom();
	virtual void CopyData(bool bCopyPK);
	
protected:
	CDBTable* GetTblCopyFrom();
	const CDBTable* GetTblCopyFrom() const
	{ 
		return const_cast<CGenericDataProvider*>(this)->GetTblCopyFrom(); 
	}

private:
	CDBTable* m_pTblCopyFrom;
};

template<class T> class CTypedGenericDataProvider
: public CGenericDataProvider
{
};


//////////////////////////////////////////////////////////////////////////

typedef CDataProvider* (*ProviderCreateFunc)();

class ETLLIB_EXPORT CDataHandler
: public CDataHandlerKey
{
public:
	CDataHandler()
	{
		m_dwFlags = 0;
		m_pTblCopyHelper	= NULL;
		m_pTblCopyTo		= NULL;
		m_pDataProvider	= NULL;
		m_ProviderCreateFunc = NULL;
	}
	virtual ~CDataHandler() {}

	void SetParameters(CTblCopyHelper* pTblCopyHelper,
				const CTableId&	id)	
	{ 
		m_pTblCopyHelper	= pTblCopyHelper; 
		m_id = id;
	}
	CTblCopyHelper* GetTblCopyHelper();

	CDataProvider* GetDataProvider();
	const CDataProvider* GetDataProvider() const
		{ return const_cast<CDataHandler*>(this)->GetDataProvider(); }

//	Data receiver-specific methods
	virtual BOOL DeleteRecord();
	virtual void DeleteRecords(DWORD dwFilterType);
	virtual BOOL FindMatchByUI();
	virtual bool SetUniqueName();

	void FreeStatements();
	BOOL FindNextFrom();
	BOOL FindByPrimaryKeyFrom(Identity lId);
	Identity GetFieldFrom(size_t) const;
	void SetFieldFrom(size_t, Identity);
	BOOL FindFirstFrom(DWORD dwFilterType);
	Identity GetPrimaryKeyFrom();
	void CopyData(bool bCopyPK);
	
	virtual void CorrectTableData() {}
	virtual BOOL AfterUpdate() { return TRUE; }
	virtual BOOL AfterCopyAll(CSubstRecArray*) { return TRUE; }
	virtual void SetIDs()			{}

	virtual void FirstSubType()	{}	//	To be used also for lazy initialization
	virtual bool NextSubType(CCopyIterator*, DWORD&)	{ return false; }
	virtual bool IsObsolete()		{ return false; }
	virtual bool IsObsoleteByRefs()	{ return false; }
	virtual void OnCopyRefSpecial()	{}
	virtual void OnRefAdded( Identity )	{}

//	Properties
	LeaveKind GetLeaveData()		{ return LeaveKind(m_dwFlags & OV_LEAVE_KIND); }
	void		 SetLeaveData(LeaveKind lk)		
											{ m_dwFlags = (m_dwFlags & ~OV_LEAVE_KIND) | lk; }
	BOOL_PROP(ConvertOnly, OV_CONVERT_ONLY)
	BOOL_PROP(UpdateDestination, OV_UPDATE_TEMPL_REF)
	BOOL_PROP(DontCopyRef, OV_DONT_COPY_REF)
	BOOL_PROP(DontShortcutRef, OV_DONT_SHORTCUT_REF)
	BOOL_PROP(Initialized, OV_INITIALIZED)
	BOOL_PROP(AbandonDependants, OV_ABANDON_DEPENDANTS)

	UIChoiceKind GetUIChoiceKind(){ return UIChoiceKind(m_dwFlags & OV_OVERWRITE); }
	void SetUIChoiceKind(UIChoiceKind ck) { m_dwFlags = (m_dwFlags & ~OV_OVERWRITE) | ck; }

protected:
	CDBTable* GetTblCopyTo();


	ProviderCreateFunc m_ProviderCreateFunc;

private:
	CTblCopyHelper* m_pTblCopyHelper;
	CDBTable* m_pTblCopyTo;
	DWORD			m_dwFlags;
	CDataProvider* m_pDataProvider;
};


typedef CDataHandler* (*VariantCreateFunc)();

class COrderVariantKey
{
public:
	COrderVariantKey()
	{
		m_dwFlags = 0; 
	}
	COrderVariantKey(const CTableId& id)
	: m_idSlave(id)
	{
		m_dwFlags = 0; 
	}

	CTableId GetTblSlaveTo() const	{ return m_idSlave; }
	CTableId	GetTblMasterTo() const	{ return m_idMaster; }
	CTableId	GetCopyTableId() const	
	{ 
		return (OV_XLINK == (m_dwFlags & OV_KIND))? m_idMaster : m_idSlave;
	}
	bool IsOrderVariant() const		{ return m_dwFlags != 0; }

protected:
	DWORD			m_dwFlags;
	CTableId		m_idMaster, m_idSlave;
};

class ETLLIB_EXPORT COrderVariant : public COrderVariantKey
{
	friend bool ETLLIB_EXPORT IsEqual(COrderVariant* pVar1, COrderVariant* pVar2);
public:
	COrderVariant()						
	{ 
		m_pTblCopyTo		= NULL;
		m_pTblSlTo			= NULL;
		m_pTblSlFrom		= NULL;
		m_dwFilterType = 0; 
		m_dwFlags = 0; 
		m_pMapId				= NULL;
		m_pTblCopyHelper	= NULL;
		m_pForkEntry		= NULL;
		m_nForkedAt			= INT_MIN; // Pseudo null
		m_pOrderVariant	= NULL;
		m_pSlaveOV			= NULL;
		m_VariantCreateFunc = NULL;
		m_nFieldOffset		= 0;
		m_nPassedAt			= INT_MIN; // Pseudo null
	}
	~COrderVariant()						
	{
		if(m_pForkEntry)
			m_pForkEntry->m_pForkEntry = NULL;
	}

	void SetTblCopyHelper(CTblCopyHelper* pTblCopyHelper)	
											{ m_pTblCopyHelper = pTblCopyHelper; }
	CTblCopyHelper* GetTblCopyHelper();

	DWORD		 GetFilterType()		{ return m_dwFilterType; }

	void SetPassed();

	BOOL_PROP(ByReference, OV_BY_REFERENCE)
	SerialKind GetSerialKind()			{ return SerialKind(m_dwFlags & OV_SERIAL_KIND); }
	void SetSerialKind(SerialKind sk)	{ m_dwFlags |= sk; }
	bool			IsAccessory()		{ return 0 == m_dwFilterType; }

	BOOL IsPassed();
	bool IsEverPassed()					{ return m_nPassedAt != INT_MIN || IsPassed(); }

	LeaveKind GetLeaveData();

	UIChoiceKind GetUIChoiceKind();
	void SetUIChoiceKind(UIChoiceKind ck);

	bool IsEntry()		{ return OV_ENTRY == (m_dwFlags & OV_KIND); }
	bool IsLink()		{ return OV_LINK == (m_dwFlags & OV_KIND); }
	bool IsXLink()		{ return OV_XLINK == (m_dwFlags & OV_KIND); }

	CDBTable* GetTblCopyTo();

	Identity	 GetFieldSlaveTo() const;
	Identity	 GetFieldSlaveFrom() const;
	void		 SetFieldSlaveTo(Identity);
	void		 SetFieldSlaveFrom(Identity);

	Identity	GetPrimaryKeyFrom();

	Identity	GetIteratorValueTo()		{ return m_CopyIterator.GetValueTo();   }
	Identity	GetIteratorValueFrom()	{ return m_CopyIterator.GetValueFrom(); }

	void		CopyData(bool bCopyPK = true);

	void AddCouple();
	BOOL Convert();

	bool		 IsSelfLink()				{ return m_idMaster == m_idSlave; }

	void AddSubstRecs(CSubstRecArrayPtr& SubstRecArrayPtr)
	{
		m_CopyIterator.AddData(SubstRecArrayPtr);
	}
	COrderVariant* ForkEntry(CSubstRecArrayPtr& );
	void ConvertToEntry(CSubstRecArrayPtr& SubstRecArrayPtr);

	BOOL DeleteRecord();
	void DeleteRecords(DWORD dwFilterType);

	void SetIDs();

	void FirstSubType();
	void CorrectTableData();
	BOOL FindMatchByUI();
	BOOL FindFirstFrom(DWORD dwFilterType);
	bool IsObsolete();	
	bool IsObsoleteByRefs();
	BOOL AfterUpdate();
	BOOL FindNextFrom();
	bool NextSubType(CCopyIterator* pCI, DWORD& rDW);
	BOOL AfterCopyAll(CSubstRecArray* pArr);
	void FreeStatements();
	BOOL FindByPrimaryKeyFrom(Identity lId);
	void OnCopyRefSpecial();
	bool IsUpdateDestination();
	bool IsDontCopyRef();
	bool IsDontShortcutRef();
	bool IsAbandonDependants();
	bool IsConvertOnly();

	CCopyIterator m_CopyIterator;
	CMapIdentities* m_pMapId;

	const CDataHandler* OrderVariantBase() const
	{
		return const_cast<COrderVariant*>(this)->GetOrderVariantBase();
	}
	LPCWSTR GetEntityName() const;

	size_t GetFieldOffset() const { return m_nFieldOffset; }

	int GetForkedAt()			{ return m_nForkedAt; }
	bool IsEverForked()			{ return m_nForkedAt != INT_MIN; }

	bool SetUniqueName();

	bool HasPrimaryKey();

protected:
	CDataHandler* GetOrderVariantBase();
	CDataHandler* GetSlaveOrderVariantBase();
	CDBTable*		 GetTblSlaveTo_();
	CDBTable*		 GetTblSlaveFrom_();
	void		 SetPassedAt(int nPass)			{ m_nPassedAt = nPass; }

	DWORD			m_dwFilterType;

	VariantCreateFunc m_VariantCreateFunc;

	size_t		m_nFieldOffset;

private:
	CTblCopyHelper* m_pTblCopyHelper;
	CDataHandler* m_pOrderVariant;
	CDataHandler* m_pSlaveOV;

	CDBTable*	m_pTblCopyTo;
	CDBTable*			m_pTblSlTo;
	CDBTable*			m_pTblSlFrom;

//	Mutual reference between link and entry
	COrderVariant* m_pForkEntry;
	int			m_nForkedAt;

	int m_nPassedAt;
};

#undef BOOL_PROP


typedef COrderVariant COrderEntry;
typedef COrderVariant COrderLink;
typedef COrderVariant CXLink;


template<class T> class COrderVariantBase : public CDataHandler
{
public:
	COrderVariantBase()		{ m_ProviderCreateFunc = GetDataProvider; }
protected:
	T* GetTypedTblCopyTo()	{ return static_cast<T*>(GetTblCopyTo()); }
private:
	static CDataProvider* GetDataProvider()	
		{ return new CTypedGenericDataProvider<T>(); }	
};

template<class T> class CTypedOrderVariant : public COrderVariantBase<T>
{
};


//////////////////////////////////////////////////////////////////////////

template<class T> class CTypedOrderVarDesc : public CTypedOrderVariant<T>
{
};


template<class T> class CTypedEntryLink : public COrderVariant
{
public:
	CTypedEntryLink( CCopyIterator		CopyIterator,
							const CTableId&	idSlave,
							size_t	nFieldOffset,
							DWORD	dwFilterType = fltAutoNumber | fltPrimaryKey)
	{
		m_CopyIterator		= CopyIterator;
		m_idSlave			= idSlave;
		m_nFieldOffset		= nFieldOffset;
		m_dwFilterType		= dwFilterType;
		m_dwFlags		  |= OV_ENTRY;
		CommonConstruct();
	}

	CTypedEntryLink(  const CTableId&	idMaster,
							const CTableId&	idSlave,
							size_t				nFieldOffset,
							DWORD					dwFilterType)
	{
		m_idMaster			= idMaster;
		m_idSlave			= idSlave;
		m_nFieldOffset		= nFieldOffset;
		m_dwFilterType		= dwFilterType;
		m_dwFlags		  |= OV_LINK;
		CommonConstruct();
	}

	CTypedEntryLink(	bool					bByReference,
							const CTableId&	idMaster,
							const CTableId&	idSlave,
							size_t				nFieldOffset)
	{
		m_idMaster			= idMaster;
		m_idSlave			= idSlave;
		m_nFieldOffset		= nFieldOffset;
		m_dwFlags		  |= OV_XLINK;
		if(bByReference)
			SetByReference();
		CommonConstruct();
	}

protected:
	void CommonConstruct()	{ m_VariantCreateFunc = DoGetOrderVariantBase; }

	static CDataHandler* DoGetOrderVariantBase() 
	{ return new CTypedOrderVarDesc<T>; }
};

#endif //__OrderVariant_h__
