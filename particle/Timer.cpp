#include "Timer.h"
#include <windows.h>

Timer::Timer()
{
	uint64_t cntPerSec;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&cntPerSec));
	m_SecondsPerCount = 1.0 / static_cast<double>(cntPerSec);
}

double Timer::Tick()
{
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_CurrentTime));
	double deltaTime = static_cast<double>(m_CurrentTime - m_PreviousTime) * m_SecondsPerCount;
	m_PreviousTime = m_CurrentTime;
	if (deltaTime < 0.0)
	{
		deltaTime = 0.0;
	}
	return deltaTime;
}
