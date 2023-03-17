#include "ParticleGenerator.h"
#include "ParticleSOA.h"

SimpleGenerator::SimpleGenerator(double timeSpan) : m_TimeSpan(timeSpan)
{
}

std::vector<ParticleInitialValue> SimpleGenerator::Generates(double dt)
{
	m_Counter += dt;
	if (m_Counter > m_TimeSpan)
	{
		m_Counter = 0.0;
		ParticleInitialValue value
		{
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f},
			0.0f,
			4.0f,
		};
		return {100, value};
	}
	return {};
}
