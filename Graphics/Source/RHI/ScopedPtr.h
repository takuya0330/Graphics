#pragma once

#include <type_traits>

#include "DeviceObject.h"

namespace RHI {

#if 0
template<class T, typename = void>
class ScopedPtr;

template<>
class ScopedPtr<IDevice, void>
{
public:
	ScopedPtr() noexcept
	    : m_ptr(nullptr)
	{
	}

	explicit ScopedPtr(std::nullptr_t) noexcept
	    : m_ptr(nullptr)
	{
	}

	explicit ScopedPtr(IDevice* ptr) noexcept
	    : m_ptr(ptr)
	{
	}

	explicit ScopedPtr(const ScopedPtr& ptr) noexcept
	    : m_ptr(ptr.m_ptr)
	{
	}

	~ScopedPtr() noexcept
	{
		if (m_ptr)
		{
			delete m_ptr;
			m_ptr = nullptr;
		}
	}

	operator bool() const
	{
		return m_ptr != nullptr;
	}

	IDevice* operator->() const
	{
		return m_ptr;
	}

	IDevice* Get() const
	{
		return m_ptr;
	}

	IDevice* const* GetAddress() const
	{
		return &m_ptr;
	}

	IDevice** GetAddress()
	{
		return &m_ptr;
	}

private:
	IDevice* m_ptr;
};

template<class T>
class ScopedPtr<T, std::enable_if_t<std::is_base_of_v<IDeviceObject, T>>>
{
	template<class U>
	friend class ScopedPtr;

public:
	ScopedPtr() noexcept
	    : m_ptr(nullptr)
	{
	}

	explicit ScopedPtr(std::nullptr_t) noexcept
	    : m_ptr(nullptr)
	{
	}

	template<class U>
	explicit ScopedPtr(U* ptr) noexcept
	    : m_ptr(ptr)
	{
		addRef();
	}

	explicit ScopedPtr(const ScopedPtr& ptr) noexcept
	    : m_ptr(ptr.m_ptr)
	{
		addRef();
	}

	template<class U>
	explicit ScopedPtr(const ScopedPtr<U>& ptr) noexcept
	    : m_ptr(ptr.m_ptr)
	{
		addRef();
	}

	explicit ScopedPtr(ScopedPtr&& ptr) noexcept
	    : m_ptr(nullptr)
	{
		if (this != reinterpret_cast<ScopedPtr*>(&reinterpret_cast<uint8_t&>(ptr)))
		{
			Swap(other);
		}
	}

	~ScopedPtr() noexcept
	{
		release();
	}

	ScopedPtr& operator=(std::nullptr_t)
	{
		release();
		return *this;
	}

	ScopedPtr& operator=(T* ptr)
	{
		if (m_ptr != ptr)
		{
			ScopedPtr(ptr).swap(*this);
		}
		return *this;
	}

	template<class U>
	ScopedPtr& operator=(U* ptr)
	{
		ScopedPtr(ptr).swap(*this);
		return *this;
	}

	ScopedPtr& operator=(const ScopedPtr& ptr)
	{
		if (m_ptr != ptr.m_ptr)
		{
			ScopedPtr(ptr).swap(*this);
		}
		return *this;
	}

	template<class U>
	ScopedPtr& operator=(const ScopedPtr<U>& ptr)
	{
		ScopedPtr(ptr).swap(*this);
		return *this;
	}

	ScopedPtr& operator=(const ScopedPtr&& ptr)
	{
		ScopedPtr(static_cast<ScopedPtr&&>(ptr)).swap(*this);
		return *this;
	}

	template<class U>
	ScopedPtr& operator=(const ScopedPtr<U>& ptr)
	{
		ScopedPtr(static_cast<ScopedPtr<U>&&>(ptr)).swap(*this);
		return *this;
	}

	operator bool() const
	{
		return m_ptr != nullptr;
	}

	T* operator->() const
	{
		return m_ptr;
	}

	T* Get() const
	{
		return m_ptr;
	}

	T* const* GetAddress() const
	{
		return &m_ptr;
	}

	T** GetAddress()
	{
		return &m_ptr;
	}

protected:
	void addRef() const
	{
		if (m_ptr != nullptr)
		{
			m_ptr->AddRef();
		}
	}

	void release()
	{
		T* temp = m_ptr;
		if (temp != nullptr)
		{
			m_ptr = nullptr;
			temp->Release();
		}
	}

	void swap(ScopedPtr&& ptr)
	{
		T* temp   = m_ptr;
		m_ptr     = ptr.m_ptr;
		ptr.m_ptr = temp;
	}

	void swap(ScopedPtr& ptr)
	{
		T* temp   = m_ptr;
		m_ptr     = ptr.m_ptr;
		ptr.m_ptr = temp;
	}

private:
	T* m_ptr;
};
#endif

} // namespace RHI
