#pragma once

template <class T, unsigned Size>
class AlignedStack
{
public:
	AlignedStack();
	~AlignedStack();

	void Push(const T& element);
	//void Pop();
	T* Get(uint32_t )
};

