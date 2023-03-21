#pragma once

#include <list>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

class FileSelection
{
public:
	FileSelection() = default;
	FileSelection(std::initializer_list<std::wstring> names) : m_FileNames(names){}

	void SetName(const std::wstring& file, int index)
	{
		m_FileNames[index] = file;
		m_IsDirty = m_Observer;
	}
	void AddObserver() { ++m_Observer; }
	void RemoveObserver()
	{
		if (--m_Observer  < 0)
		{
			m_Observer = 0;
		}
	}
	int& GetFlag() { return m_IsDirty; }
	const auto& GetPaths() { return m_FileNames; }

private:
	std::vector<std::wstring> m_FileNames{};
	int m_IsDirty = 0;
	int m_Observer = 0;
};


