#include "ParticleEmitter.h"
#include "ParticleSOA.h"

SimpleEmitter::SimpleEmitter(float timeSpan) : m_TimeSpan(timeSpan)
{
}

std::vector<ParticleInitialValue> SimpleEmitter::Generates(float dt)
{
	
	m_Counter += dt;
	std::vector<ParticleInitialValue> v;
	auto cnt = static_cast<unsigned>(m_Counter / m_TimeSpan);
	m_Counter -= static_cast<float>(cnt) * m_TimeSpan;
	constexpr auto value = ParticleInitialValue
	{
		{0.0f, 0.0f, 0.0f},
		{0.0f, 20.0f, -10.0f},
		{20.0f, 20.0f},
		0.0f,
		4.0f,
	};
	return {cnt, value};
}
