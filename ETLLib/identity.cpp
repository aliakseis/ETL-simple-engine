
#include "stdafx.h"

#include "identity.h"

class get_visitor : public boost::static_visitor<void>
{
public:
	get_visitor(_variant_t& val) : m_val(val) {}

	void operator()(const NotDefinedValue&) const
	{
		m_val.Clear();
	}

	void operator()(const NullValue&) const
	{
		m_val.Clear();
		m_val.vt = VT_NULL;
	}

	void operator()(long val) const
	{
		m_val = val;
	}

	void operator()(const wstring* pStr) const
	{
		m_val = pStr->c_str();
	}

private:
	_variant_t& m_val;
};

class hash_visitor : public boost::static_visitor<UINT>
{
public:
	UINT operator()(const NotDefinedValue&) const
	{
		return 0;
	}

	UINT operator()(const NullValue&) const
	{
		return 0;
	}

	UINT operator()(long val) const
	{
		return val;
	}

	UINT operator()(const wstring* pStr) const
	{
		return ((UINT) pStr) >> 4;
	}
};


class is_valid_visitor : public boost::static_visitor<bool>
{
public:
	bool operator()(const NotDefinedValue&) const
	{
		return false;
	}

	bool operator()(const NullValue&) const
	{
		return false;
	}

	bool operator()(long) const
	{
		return true;
	}

	bool operator()(const wstring* pStr) const
	{
		return pStr != NULL;
	}
};

Identity::Identity(const NullValue&) : m_val(NullValue()) {}

Identity::Identity() {} // not defined

Identity::Identity(long val)
{
	m_val = val;
}

void Identity::Set(const _variant_t& val, CIdentityShared& context)
{
	switch (val.vt)
	{
	case VT_I4:
		m_val = val.lVal;
		break;
	case VT_BSTR:
		if (val.bstrVal != NULL)
		{
			Set(val.bstrVal, context);
			break;
		}
		// falls thru
	case VT_NULL:
		m_val = NullValue();
		break;
	case VT_DECIMAL:
		ASSERT(0 == val.decVal.Hi32 && 0 == val.decVal.Mid32 
			&& 0 == val.decVal.scale);
		m_val = val.decVal.Lo32;
		break;
	default: ASSERT(0); m_val = NotDefinedValue();
	}
}

void Identity::Set(LPCWSTR val, CIdentityShared& context)
{
	CIdentityShared::CharIds::iterator iter 
		= context.m_charIds.insert(val).first;
	m_val = &(*iter);
}

void Identity::Get(_variant_t& val) const
{
	get_visitor visitor(val);
	boost::apply_visitor(visitor, m_val);
}

bool Identity::operator <  (const Identity& other) const { return m_val < other.m_val; }
bool Identity::operator != (const Identity& other) const { return !(m_val == other.m_val); }
bool Identity::operator == (const Identity& other) const { return m_val == other.m_val; }

UINT Identity::GetHashKey() const 
{ 
	hash_visitor visitor;
	return  boost::apply_visitor(visitor, m_val); 
}

bool IsValid(const Identity& id) 
{ 
	is_valid_visitor visitor;
	return  boost::apply_visitor(visitor, id.m_val); 
}
