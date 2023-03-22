#pragma once
#include <cassert>

//template <class T>
//struct StructWithFlag
//{
//	T Object;
//
//	int m_IsDirty = 0;
//	int m_Observer = 0;
//
//	void AddObserver() { ++m_Observer; }
//	void RemoveObserver()
//	{
//		--m_Observer;
//		assert(m_Observer >= 0);
//	}
//	void SetDirty()
//	{
//		m_IsDirty = m_Observer;
//	}
//	void SetClean()
//	{
//		--m_IsDirty;
//		assert(m_IsDirty >= 0);
//	}
//};
