#pragma once
#include <cstdint>

class Timer
{
public:
	Timer();

	double Tick();

private:
	double m_SecondsPerCount = 0.0;

	std::uint64_t m_PreviousTime = 0;
	std::uint64_t m_CurrentTime = 0;
};
