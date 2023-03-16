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
		ParticleInitialValue value
		{
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f},
			0.0f,
			5.0f,
		};
		return {64, value};
	}
	return {};
}
