#ifndef SharedComVariant_h_
#define SharedComVariant_h_

#pragma once

#include <memory>
#include <type_traits>


//typedef _variant_t SharedComVariant;


class SharedComVariant
{
    std::shared_ptr<_variant_t> m_element;

public:
    SharedComVariant() = default;
    SharedComVariant(const SharedComVariant&) = default;
    SharedComVariant(SharedComVariant&&) = default;

    SharedComVariant& operator =(const SharedComVariant& other) = default;
    SharedComVariant& operator =(SharedComVariant&& other) = default;

    template<typename T>
    typename std::enable_if<
        std::is_reference<T>::value || !std::is_base_of<VARIANT, T>::value, SharedComVariant&>::type
    operator =(T&& other)
	{
        if (m_element.unique())
            *m_element = std::forward<T>(other);
        else
            m_element = std::make_shared<_variant_t>(std::forward<T>(other));

		return *this;
	}

    SharedComVariant& operator =(VARIANT&& other)
    {
        if (m_element.unique())
            m_element->Attach(other);
        else
            m_element = std::make_shared<_variant_t>(other, false);

        return *this;
    }

	bool operator == (const SharedComVariant& other) const
	{
		return m_element && other.m_element
			&& *m_element == *other.m_element;
	}

	bool operator != (const SharedComVariant& other) const
	{
		return !(operator ==(other));
	}


	bool operator == (const VARIANT& varSrc) const
	{
		return m_element && *m_element == varSrc;
	}

	bool operator != (const VARIANT& varSrc) const
	{
		return !(operator ==(varSrc));
	}

	operator const _variant_t&() const 
	{
		if (m_element)
			return *m_element;

		static _variant_t null_variant;
		ASSERT(VT_EMPTY == null_variant.vt);
		return null_variant;
	}

    const _variant_t* operator &() const
    {
        return &operator const _variant_t&();
    }
};


#endif// SharedComVariant_h_
