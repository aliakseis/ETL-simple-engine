// BellsMacrosX.h : header file
//
#ifndef BELLS_MACROS_X__H__
#define BELLS_MACROS_X__H__

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

template<int v> struct Int2Type_
{
	enum { value = v };
};

template<class T> struct HasVirtualTable_
{
	class X : public T
	{
		X();
		virtual void dummy();
	};
	enum { has_table = sizeof(X) == sizeof(T) };
};

template<typename T> inline void AssertVTable(const void* pData, T)
{
	ASSERT(!::IsBadReadPtr(
		*static_cast<const void*const*>(pData), sizeof(void*)));
}

inline void AssertVTable(const void*, Int2Type_<0>) {}

template<class T> inline void AssertValidPointer(const T* pData, ...)
{
	if(!::IsBadWritePtr((void*)pData, sizeof(T)))
		AssertVTable(pData, Int2Type_<HasVirtualTable_<T>::has_table>());
	else
		ASSERT(0);
}


class CObject;

template<class T> inline void AssertValidPointer(const T* pOb, const CObject*)
{
	if(!::IsBadWritePtr((void*)pOb, sizeof(T)))
		if(AfxIsValidAddress(*reinterpret_cast<const void*const*>(pOb), sizeof(void*), FALSE))
			pOb->AssertValid();
		else
			ASSERT(0);
	else
		ASSERT(0);
}

template<class T> inline void CHECK_ADDRESS(const T* pData)
{
	AssertValidPointer(pData, pData);
}
template<class T> inline void CHECK_NULL_OR_ADDRESS(const T* pData)
{
	if(NULL != pData)
		AssertValidPointer(pData, pData);
}

#else

#define CHECK_ADDRESS(pData)			static_cast<void>(0)
#define CHECK_NULL_OR_ADDRESS(pData)	static_cast<void>(0)

#endif//_DEBUG


#endif
