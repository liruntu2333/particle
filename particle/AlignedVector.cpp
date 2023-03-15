#include "AlignedVector.h"

#include <cassert>
#include <vector>

constexpr int RegisterAlignment = 4 * sizeof(float);

template <class T, unsigned N>
AlignedVector<T, N>::AlignedVector()
{
	static_assert(sizeof(T) * sizeof(N) % RegisterAlignment == 0);
}

template <class T, unsigned N>
AlignedVector<T, N>::~AlignedVector()
{
}

template <class T, unsigned N>
void AlignedVector<T, N>::PushBack(const T& element)
{
	if (m_Capacity == m_Size) Inflate();
	m_Data.get()[m_Size++] = element;
	std::vector<int> a;
}

template <class T, unsigned N>
void AlignedVector<T, N>::Clear()
{
	m_Data = nullptr;
}

template <class T, unsigned N>
void AlignedVector<T, N>::Inflate()
{
	auto oldCapacity = m_Capacity;

	assert(m_Capacity <= UINT32_MAX);
	if (m_Capacity == 0) m_Capacity = 4;
	else m_Capacity *= 2;

	std::unique_ptr<T[]> oldData = std::move(m_Data);

	m_Data = new T[m_Capacity]{};
	memcpy(m_Data, oldData.get(), oldCapacity * sizeof(T));
}
