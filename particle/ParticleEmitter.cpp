#include "ParticleEmitter.h"
#include "ParticleSOA.h"

SimpleEmitter::SimpleEmitter(double timeSpan) : m_TimeSpan(timeSpan)
{
}

std::vector<ParticleInitialValue> SimpleEmitter::Generates(double dt)
{
	m_Counter += dt;
	if (m_Counter >= m_TimeSpan)
	{
		m_Counter -= m_TimeSpan;
		ParticleInitialValue value
		{
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f},
			0.0f,
			4.0f,
		};
		return {1, value};
	}
	return {};
}
