#ifndef ELAPSEDTIMER_H
#define ELAPSEDTIMER_H

#include <chrono>

class ElapsedTimer
{
private:
	double											interval_s = 0.1;
	std::chrono::high_resolution_clock::time_point	tic_time;

public:
	ElapsedTimer();
	ElapsedTimer(unsigned int period);
	~ElapsedTimer();

	void tic();
	bool elapsed() const;
};

#endif
