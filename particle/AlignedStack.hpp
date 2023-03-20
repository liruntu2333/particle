#pragma once
#include <functional>
#include <memory>
#include <xsimd/xsimd.hpp>

template <class T, size_t Capacity, size_t Alignment>
class AlignedStack
{
public:
	using AlignedAllocator = xsimd::aligned_allocator<T, Alignment>;

	AlignedStack();
	virtual ~AlignedStack();

	AlignedStack(const AlignedStack& other);
	AlignedStack(AlignedStack&& other) noexcept;

	AlignedStack& operator=(const AlignedStack& other);
	AlignedStack& operator=(AlignedStack&& other) noexcept;

	void Push(const T& element, size_t index);
	//void Pop();
	T* Get(size_t index);
	T& operator[](size_t index);

	//size_t Size() const { return m_Size; }

	//template<class Predicate>
	//size_t EraseIf(Predicate pred);

	void EraseN(size_t index, size_t cnt);

private:
	std::unique_ptr<AlignedAllocator> m_Allocator = nullptr;
	T* m_Data = nullptr;

	//size_t m_Size = 0;

	static constexpr size_t m_Capacity = Capacity;
};

template <class T, size_t Capacity, size_t Alignment>
AlignedStack<T, Capacity, Alignment>::AlignedStack()
{
	m_Allocator = std::make_unique<AlignedAllocator>();
	m_Data = m_Allocator->allocate(Capacity);
	for (size_t i = 0; i < Capacity; ++i)
	{
		m_Allocator->construct(m_Data + i);
	}
}

template <class T, size_t Capacity, size_t Alignment>
AlignedStack<T, Capacity, Alignment>::~AlignedStack()
{
	for (size_t i = 0; i < Capacity; ++i)
	{
		m_Allocator->destroy(m_Data + i);
	}
	m_Allocator->deallocate(m_Data, Capacity);
	m_Data = nullptr;
}

template <class T, size_t Capacity, size_t Alignment>
AlignedStack<T, Capacity, Alignment>::AlignedStack(const AlignedStack& other) :
	m_Allocator(std::make_unique<AlignedAllocator>(other.m_Allocator))
{
	m_Data = m_Allocator->allocate(Capacity);
	for (size_t i = 0; i < Capacity; ++i)
	{
		m_Allocator->construct(m_Data + i, *(other.m_Data + i));
	}
}

template <class T, size_t Capacity, size_t Alignment>
AlignedStack<T, Capacity, Alignment>::AlignedStack(AlignedStack&& other) noexcept :
	m_Allocator(std::move(other.m_Allocator)),
	m_Data(other.m_Data)
{
	other.m_Allocator = nullptr;
	other.m_Data = nullptr;
}

template <class T, size_t Capacity, size_t Alignment>
AlignedStack<T, Capacity, Alignment>& AlignedStack<T, Capacity, Alignment>::operator=(const AlignedStack& other)
{
	if (this == &other) return *this;

	for (size_t i = 0; i < Capacity; ++i)
	{
		m_Allocator->destroy(m_Data + i);
	}

	m_Allocator = std::make_unique<AlignedAllocator>(other.m_Allocator);
	for (size_t i = 0; i < Capacity; ++i)
	{
		m_Allocator->construct(m_Data + i, *(other.m_Data + i));
	}

	return *this;
}

template <class T, size_t Capacity, size_t Alignment>
AlignedStack<T, Capacity, Alignment>& AlignedStack<T, Capacity, Alignment>::operator=(AlignedStack&& other) noexcept
{
	if (this == &other) return *this;

	for (size_t i = 0; i < Capacity; ++i)
	{
		m_Allocator->destroy(m_Data + i);
	}
	m_Allocator->deallocate(m_Data, m_Capacity);

	m_Allocator = std::move(other.m_Allocator);
	m_Data = other.m_Data;

	other.m_Data = nullptr;
	return *this;
}

template <class T, size_t Capacity, size_t Alignment>
void AlignedStack<T, Capacity, Alignment>::Push(const T& element, std::size_t index)
{
	assert((index < m_Capacity) && "capacity not enough");
	m_Allocator->construct(m_Data + index, element);
}

template <class T, size_t Capacity, size_t Alignment>
T* AlignedStack<T, Capacity, Alignment>::Get(size_t index)
{
	return m_Data + index;
}

template <class T, size_t Capacity, size_t Alignment>
T& AlignedStack<T, Capacity, Alignment>::operator[](size_t index)
{
	return *Get(index);
}

template <class T, size_t Capacity, size_t Alignment>
void AlignedStack<T, Capacity, Alignment>::EraseN(size_t index, size_t cnt)
{
	for (size_t i = 0; i < cnt; ++i)
	{
		m_Allocator->destroy( m_Data + index + i);
	}
}

