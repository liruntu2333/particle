#include "Timer.h"

Timer::Timer() : m_CurrentTime(std::chrono::high_resolution_clock::now())
{
}

double Timer::Tick()
{
	const auto time = std::chrono::high_resolution_clock::now();
	const std::chrono::duration<double> diff = time - m_CurrentTime;
	double dt = diff.count();
	if (dt < 0.0)
	{
		dt = 0.0;
	}
	m_CurrentTime = time;
	return dt;
}

