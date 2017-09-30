#ifndef identity_h_
#define identity_h_

#pragma once

#pragma warning(push, 3)
#pragma warning(disable:4101 4702 4345)

#include <set>
#include <string>

#include <boost/variant.hpp>

#pragma warning(pop)


#include "BellsMacrosX.h"


#ifdef ETLLIB_EXPORTS
#define ETLLIB_EXPORT __declspec(dllexport)
#else
#define ETLLIB_EXPORT __declspec(dllimport)
#endif


enum {
	fltUndefined = 0,
	fltNoFilter = 1,
	fltAutoNumber = 2,
	fltPrimaryKey = 4,
	fltUniqueIndex = 8,
	fltRelation = 16,
};


using std::wstring;

class CIdentityShared
{
friend class Identity;
	typedef std::set<wstring> CharIds;
	CharIds m_charIds;
};


class NotDefinedValue {};

inline static bool operator == (const NotDefinedValue&, const NotDefinedValue&)
{
	return true;
}
inline static bool operator < (const NotDefinedValue&, const NotDefinedValue&)
{
	return false;
}

class NullValue {};

inline static bool operator == (const NullValue&, const NullValue&)
{
	return true;
}
inline static bool operator < (const NullValue&, const NullValue&)
{
	return false;
}


class ETLLIB_EXPORT Identity 
{
friend ETLLIB_EXPORT bool IsValid(const Identity&);

protected:

	typedef boost::variant<NotDefinedValue, 
		 NullValue, 
		 long, 
		 const wstring*> Variant;
	Variant m_val;

    explicit Identity(const NullValue&);

public:
	Identity();

    explicit Identity(long val);

	void Set(const _variant_t& val, CIdentityShared& context);

	void Set(LPCWSTR val, CIdentityShared& context);

    _variant_t Get() const;

	bool operator <  (const Identity& other) const;
	bool operator != (const Identity& other) const;
	bool operator == (const Identity& other) const;

	UINT GetHashKey() const;
};


class NullIdentity : public Identity
{
public:
	NullIdentity() : Identity(NullValue()) {}
};

#define ID_NOT_DEF Identity()

#define ID_NULL NullIdentity()


ETLLIB_EXPORT bool IsValid(const Identity& id);

//	MFC compatible HashKey prototype
template <typename T> UINT __stdcall HashKey(T);

template <> inline UINT __stdcall HashKey<Identity>(Identity key)
{ 
	return key.GetHashKey();
}

namespace __gnu_cxx
{

template<class T> class hash;

template<> class hash <Identity>
{
public:
	size_t operator ()(Identity key) const
	{ 
		return key.GetHashKey();
	}
};

}

// custom specialization of std::hash can be injected in namespace std
namespace std
{

template<> struct hash<Identity>
{
    typedef Identity argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& key) const
    {
        return key.GetHashKey();
    }
};

}
#endif// identity_h_

