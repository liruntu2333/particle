#pragma once
#include <memory>

/**
 * \brief Aligned vector for CPU SIMD instructions.
 * \tparam T element type
 * \tparam N element alignment
 */
template <class T, unsigned N>
class AlignedVector
{
public:
	AlignedVector();
	~AlignedVector();

	void PushBack(const T& element);
	void Clear();

	T* Get();

private:

	void Inflate();

	std::unique_ptr<T[]> m_Data = nullptr;
	uint32_t m_Size = 0;
	uint32_t m_Capacity = 0;
};

