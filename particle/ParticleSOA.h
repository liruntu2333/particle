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

template <std::size_t Capacity>
class ParticleSystem;

template <size_t Capacity, size_t Alignment = xsimd::best_arch::alignment()>
class ParticleSOA
{

public:
	using FloatSOA  = AlignedStack<float, Capacity, Alignment>;
	using IntSOA    = AlignedStack<int, Capacity, Alignment>;
	using UintSOA   = AlignedStack<uint32_t, Capacity, Alignment>;
	using DoubleSOA = AlignedStack<double, Capacity, Alignment>;

	using Vector3FField = std::array<FloatSOA, 3>;
	using Vector4FField = std::array<FloatSOA, 4>;
	using ScalarFField  = FloatSOA;

	ParticleSOA() = default;
	virtual ~ParticleSOA() = default;

	ParticleSOA(const ParticleSOA&) = default;
	ParticleSOA(ParticleSOA&&) = default;
	ParticleSOA& operator=(const ParticleSOA&) = default;
	ParticleSOA& operator=(ParticleSOA&&) = default;

	[[nodiscard]] size_t Size() const { return m_Size; }

	void Push(const ParticleInitialValue& particle);
	void Push(const std::vector<ParticleInitialValue>& particles);

	template <class Predicate>
	size_t EraseIf(Predicate pred);
	
	void PrintLog();

	friend class ParticleSystem<Capacity>;
	
private:
	Vector3FField m_Position{};
	Vector3FField m_Velocity{};
	Vector4FField m_Color{};

	ScalarFField m_Life{};
	ScalarFField m_MaxLife{};

	size_t m_Size = 0;
	static constexpr size_t m_Capacity = Capacity;
};

template <size_t Capacity, size_t Alignment>
void ParticleSOA<Capacity, Alignment>::Push(const ParticleInitialValue& particle)
{
	if (m_Capacity <= m_Size)
		return;

	for (int i = 0; i < 3; ++i)
	{
		m_Position[i].Push(particle.Position[i], m_Size);
		m_Velocity[i].Push(particle.Velocity[i], m_Size);
	}
	m_Life.Push(particle.Life);
	m_MaxLife.Push(particle.MaxLife);
	++m_Size;
}

template <size_t Capacity, size_t Alignment>
void ParticleSOA<Capacity, Alignment>::Push(const std::vector<ParticleInitialValue>& particles)
{
	for (const auto & initVal : particles)
	{
		if (m_Capacity <= m_Size) break;

		for (int i = 0; i < 3; ++i)
		{
			m_Position[i].Push(initVal.Position[i], m_Size);
			m_Velocity[i].Push(initVal.Velocity[i], m_Size);
		}
		m_Life.Push(initVal.Life, m_Size);
		m_MaxLife.Push(initVal.MaxLife, m_Size);
		++m_Size;
	}
}

template <size_t Capacity, size_t Alignment>
template <class Predicate>
size_t ParticleSOA< Capacity, Alignment>::EraseIf(Predicate pred)
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
			assignScalar(m_Life, iLft, iRht);
			assignScalar(m_MaxLife, iLft, iRht);
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
	eraseScalar(m_Life, iLft, erasedCount);
	eraseScalar(m_MaxLife, iLft, erasedCount);
	m_Size = iLft;
	return erasedCount;
}

template <size_t Capacity, size_t Alignment>
void ParticleSOA<Capacity, Alignment>::PrintLog()
{
	for (size_t i = 0; i < m_Size; ++i)
	{
		printf("Particle No. %3d Pos : %2.1f, %2.1f, %2.1f\t", 
			i, m_Position[0][i], m_Position[1][i], m_Position[2][i]);
		printf("Life : %2.1f, R: %2.2f, G %2.2f, B %2.2f, A %2.2f\n", 
			m_Life[i], m_Color[0][i], m_Color[1][i], m_Color[2][i], m_Color[3][i]);
	}
}
