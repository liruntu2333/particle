#pragma once
#include <iostream>
#include <vector>

#include "AlignedStack.hpp"

struct ParticleInitialValue
{
	float Position[3];
	float Velocity[3];
	float Life;
	float MaxLife;
};

template <std::size_t Capacity, class Arch>
class ParticleSystemCPU;

template <size_t Capacity, class Arch>
class ParticleSOA
{
public:

	static constexpr size_t CAPACITY = Capacity;
	static constexpr size_t ALIGNMENT = Arch::alignment();

	using FloatSOA  = AlignedStack<float, Capacity, ALIGNMENT>;
	using IntSOA    = AlignedStack<int, Capacity, ALIGNMENT>;
	using UintSOA   = AlignedStack<uint32_t, Capacity, ALIGNMENT>;
	using DoubleSOA = AlignedStack<double, Capacity, ALIGNMENT>;

	using Vector3FField = std::array<FloatSOA, 3>;
	using Vector4FField = std::array<FloatSOA, 4>;
	using ScalarFField  = FloatSOA;

	[[nodiscard]] size_t Size() const { return m_Size; }

	bool Push(const ParticleInitialValue& particle);
	bool Push(const std::vector<ParticleInitialValue>& particles);

	template <class Predicate>
	size_t EraseIf(Predicate pred);
	
	void PrintLog();

	friend class ParticleSystemCPU<Capacity, Arch>;
	
private:
	Vector3FField m_Position{};
	Vector3FField m_Velocity{};
	Vector4FField m_Color{};

	ScalarFField m_Age{};
	ScalarFField m_LifeSpan{};

	size_t m_Size = 0;
};

template <size_t Capacity, class Arch>
bool ParticleSOA<Capacity, Arch>::Push(const ParticleInitialValue& particle)
{
	if (CAPACITY <= m_Size) return false;

	for (int i = 0; i < 3; ++i)
	{
		m_Position[i].Push(particle.Position[i], m_Size);
		m_Velocity[i].Push(particle.Velocity[i], m_Size);
	}
	m_Age.Push(particle.Life, m_Size);
	m_LifeSpan.Push(particle.MaxLife, m_Size);
	++m_Size;
	return true;
}

template <size_t Capacity, class Arch>
bool ParticleSOA<Capacity, Arch>::Push(const std::vector<ParticleInitialValue>& particles)
{
	for (const auto & particle : particles)
	{
		if (!Push(particle)) return false;
	}
	return true;
}

template <size_t Capacity, class Arch>
template <class Predicate>
size_t ParticleSOA<Capacity, Arch>::EraseIf(Predicate pred)
{
	size_t iLft = 0;
	size_t iRht = 0;
	while (iRht < m_Size)
	{
		if (!pred(this, iRht))
		{
			auto assignVector = [](auto& container, size_t lft, size_t rht)
			{
				for (auto & array : container)
				{
					array[lft] = array[rht];
				}
			};
			auto assignScalar = [](auto& arr, size_t lft, size_t rht)
			{
				arr[lft] = arr[rht];
			};
			assignVector(m_Position, iLft, iRht);
			assignVector(m_Velocity, iLft, iRht);
			assignVector(m_Color, iLft, iRht);
			assignScalar(m_Age, iLft, iRht);
			assignScalar(m_LifeSpan, iLft, iRht);
			++iLft;
		}
		++iRht;
	}

	const size_t erasedCount = iRht - iLft;
	auto eraseVector = [](auto& container, size_t index, size_t cnt)
	{
		for (auto & arr : container)
		{
			arr.EraseN(index, cnt);
		}
	};
	auto eraseScalar = [](auto& arr, size_t index, size_t cnt)
	{
		arr.EraseN(index, cnt);
	};
	eraseVector(m_Position, iLft, erasedCount);
	eraseVector(m_Velocity, iLft, erasedCount);
	eraseVector(m_Color, iLft, erasedCount);
	eraseScalar(m_Age, iLft, erasedCount);
	eraseScalar(m_LifeSpan, iLft, erasedCount);
	m_Size = iLft;
	return erasedCount;
}

template <size_t Capacity, class Arch>
void ParticleSOA<Capacity, Arch>::PrintLog()
{
	for (size_t i = 0; i < m_Size; ++i)
	{
		printf("Particle No. %3d Pos : %2.1f, %2.1f, %2.1f ", 
			i, m_Position[0][i], m_Position[1][i], m_Position[2][i]);
		printf("Age : %2.1f, LifeSpan : %1.1f R: %2.2f, G %2.2f, B %2.2f, A %2.2f\n", 
			m_Age[i], m_LifeSpan[i], m_Color[0][i], m_Color[1][i], m_Color[2][i], m_Color[3][i]);
	}
}
