#pragma once
#include <functional>
#include <memory>
#include "../xsimd/xsimd.hpp"

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

	void Push(const T& element);
	//void Pop();
	T* Get(size_t index);
	size_t Size() const { return m_Size; }

	template<class Predicate>
	size_t EraseIf(Predicate pred);

private:
	std::unique_ptr<AlignedAllocator> m_Allocator = nullptr;
	T* m_Data = nullptr;

	size_t m_Size = 0;

	constexpr size_t m_Capacity = Capacity;
};

template <class T, size_t Capacity, size_t Alignment>
AlignedStack<T, Capacity, Alignment>::AlignedStack()
{
	m_Allocator = std::make_unique<AlignedAllocator>();
	m_Data = m_Allocator->allocate(Capacity);
	for (int i = 0; i < m_Size; ++i)
	{
		m_Allocator->construct(m_Data + i);
	}
}

template <class T, size_t Capacity, size_t Alignment>
AlignedStack<T, Capacity, Alignment>::~AlignedStack()
{
	for (int i = 0; i < m_Size; ++i)
	{
		m_Allocator->destroy(m_Data + i);
	}
	m_Allocator->deallocate(m_Data);
	m_Data = nullptr;
	m_Size = 0;
}

template <class T, size_t Capacity, size_t Alignment>
AlignedStack<T, Capacity, Alignment>::AlignedStack(const AlignedStack& other) :
	m_Allocator(std::make_unique<AlignedAllocator>(other.m_Allocator)),
	m_Size(other.m_Size)
{
	m_Data = m_Allocator->allocate(Capacity);
	for (int i = 0; i < m_Size; ++i)
	{
		m_Allocator->construct(m_Data + i, *(other.m_Data + i));
	}
}

template <class T, size_t Capacity, size_t Alignment>
AlignedStack<T, Capacity, Alignment>::AlignedStack(AlignedStack&& other) noexcept :
	m_Allocator(std::move(other.m_Allocator)),
	m_Data(other.m_Data),
	m_Size(other.m_Size)
{
	other.m_Allocator = nullptr;
	other.m_Data = nullptr;
	other.m_Size = 0;
}

template <class T, size_t Capacity, size_t Alignment>
AlignedStack<T, Capacity, Alignment>& AlignedStack<T, Capacity, Alignment>::operator=(const AlignedStack& other)
{
	if (this == &other) return *this;

	for (int i = 0; i < m_Size; ++i)
	{
		m_Allocator->destroy(m_Data + i);
	}

	m_Allocator = std::make_unique<AlignedAllocator>(other.m_Allocator);
	m_Size = other.m_Size;
	for (int i = 0; i < m_Size; ++i)
	{
		m_Allocator->construct(m_Data + i, *(other.m_Data + i));
	}

	return *this;
}

template <class T, size_t Capacity, size_t Alignment>
AlignedStack<T, Capacity, Alignment>& AlignedStack<T, Capacity, Alignment>::operator=(AlignedStack&& other) noexcept
{
	if (this == &other) return *this;

	for (int i = 0; i < m_Size; ++i)
	{
		m_Allocator->destroy(m_Data + i);
	}
	m_Allocator->deallocate(m_Data);

	m_Allocator = std::move(other.m_Allocator);
	m_Data = other.m_Data;
	m_Size = other.m_Size;

	other.m_Data = nullptr;
	other.m_Size = 0;
	return *this;
}

template <class T, size_t Capacity, size_t Alignment>
void AlignedStack<T, Capacity, Alignment>::Push(const T& element)
{
	assert((m_Size < m_Capacity) && "capacity not enough");
	m_Allocator->construct(m_Data + m_Size++, element);
}

template <class T, size_t Capacity, size_t Alignment>
T* AlignedStack<T, Capacity, Alignment>::Get(size_t index)
{
	assert((index < m_Size) && "index out of range");
	return m_Data[index];

	int* p;
}

template <class T, size_t Capacity, size_t Alignment>
template <class Predicate>
size_t AlignedStack<T, Capacity, Alignment>::EraseIf(Predicate pred)
{
	T* lft = m_Data;
	T* rht = m_Data;
	const T* end = m_Data + m_Size;
	while (rht != end)
	{
		if (!pred(*lft, *rht))
			*(lft++) = *rht;
		++rht;
	}
	const size_t erasedCount = rht - lft;
	for (int i = 0; i < erasedCount; ++i)
	{
		m_Allocator->destroy(lft + i);
	}
	m_Size = lft - m_Data;
	return erasedCount;
}

