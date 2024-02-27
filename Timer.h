#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer
{
private:
	bool active;
	std::chrono::steady_clock::time_point end_time;

public:

	void start_timer_ms(int ms)
	{
		active = true;
		end_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
	}

	bool is_expired()
	{
		if (active && std::chrono::steady_clock::now() >= end_time)
		{
			active = false;
			return true;
		}

		return false;
	}

	void stop_timer()
	{
		active = false;
	}

	bool is_active() const
	{
		return active;
	}

};

#endif