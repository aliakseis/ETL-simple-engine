#ifndef SharedComVariant_h_
#define SharedComVariant_h_


//typedef _variant_t CSharedComVariant;



class CSharedComVariant
{
	class Element : public _variant_t
	{
		size_t m_nRefs;
	public:    
		Element() : m_nRefs(0) {}

		template <typename T>
		Element(const T& data) 
		: _variant_t(data), m_nRefs(0) {}

		~Element()	{ ASSERT(0 == m_nRefs); }

		template <typename T>
		Element& operator =(const T& data)
		{
			*(static_cast<_variant_t*>(this)) = data;
			return *this;
		}

		void upcount()		
		{ 
			if (this)
				++m_nRefs; 
		}
		void downcount()	
		{ 
			if (this && --m_nRefs == 0) 
				delete this; 
		}
		int GetNumRefs()	{ return m_nRefs; }
	};

	Element* m_pElement;

public:
	CSharedComVariant(const CSharedComVariant& other)
	{
		m_pElement = other.m_pElement;
		m_pElement->upcount();
	}

	CSharedComVariant()
	{
		m_pElement = NULL;
	}

	~CSharedComVariant()
	{
		m_pElement->downcount();
	}

	template<typename T>
	const CSharedComVariant& operator =(const T& other)
	{
		if (m_pElement && 1 == m_pElement->GetNumRefs())
			*m_pElement = other;
		else
		{
			m_pElement->downcount();
			m_pElement = new Element(other);
			m_pElement->upcount();
		}
		return *this;
	}

	const CSharedComVariant& operator =(const CSharedComVariant& other)
	{
		other.m_pElement->upcount();
		m_pElement->downcount();
		m_pElement = other.m_pElement;

		return *this;
	}

	void Attach(VARIANT& varSrc)
	{
		if (!m_pElement || 1 != m_pElement->GetNumRefs())
		{
			m_pElement->downcount();
			m_pElement = new Element();
			m_pElement->upcount();
		}
		m_pElement->Attach(varSrc);
	}


	bool operator == (const CSharedComVariant& other) const
	{
		return m_pElement && other.m_pElement
			&& *m_pElement == *other.m_pElement;
	}

	bool operator != (const CSharedComVariant& other) const
	{
		return !(operator ==(other));
	}


	bool operator == (const VARIANT& varSrc) const
	{
		return m_pElement && *m_pElement == varSrc;
	}

	bool operator != (const VARIANT& varSrc) const
	{
		return !(operator ==(varSrc));
	}

	operator const _variant_t&() const 
	{
		if (m_pElement)
			return *m_pElement;

		static _variant_t null_variant;
		ASSERT(VT_EMPTY == null_variant.vt);
		return null_variant;
	}

	_variant_t* operator &() 
	{ 
		if (!m_pElement)
		{
			m_pElement = new Element();
			m_pElement->upcount();
		}
		return m_pElement;
	}
};


#endif// SharedComVariant_h_
