#pragma once
#include <chrono>

class Timer
{
public:
	Timer();

	double Tick();

private:
	//double m_SecondsPerCount = 0.0;
	using time_point = std::chrono::time_point<std::chrono::steady_clock>;

	time_point m_CurrentTime;
};
