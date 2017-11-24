#include "elapsedtimer.h"
#include <iostream>

using namespace std::chrono;

ElapsedTimer::ElapsedTimer()
{
	tic();
}

ElapsedTimer::ElapsedTimer(unsigned int period_ms)
	: interval_s(period_ms / 1000.0)
{
	tic();
}

ElapsedTimer::~ElapsedTimer() {}

void ElapsedTimer::tic()
{
	tic_time = high_resolution_clock::now();
}

// zwraca true, jesli od ostatniego wywolania tic() minal interval_ms
bool ElapsedTimer::elapsed() const
{
	const auto time_span = duration_cast<duration<double>>(high_resolution_clock::now() - tic_time);
	return (time_span.count() >= interval_s);
}
