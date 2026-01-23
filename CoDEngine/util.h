#pragma once
#include "stdafx.h"

struct rate_limiter
{
	int attempts_allowed_in_time_period;
	uint32_t time_period;
	uint32_t last_event_time;
	int num_attempts_allowed;

	rate_limiter(uint32_t time_period_ms, int num_allowed_attempts) :
		num_attempts_allowed(num_allowed_attempts),
		time_period(time_period_ms)
	{

	}

	//Returns true if the rate limit has been exceeded
	bool process()
	{
		if (GetTickCount() - last_event_time < time_period)
		{
			if(++num_attempts_allowed > attempts_allowed_in_time_period)
				return true;
		}
		else
		{
			last_event_time = GetTickCount();
			num_attempts_allowed = 1;
		}

		return false;
	}

	//Rate limit was exceeded by last process.
	bool exceeded_last_process()
	{
		return (num_attempts_allowed - 1) == attempts_allowed_in_time_period;
	}

};

static uint32_t seconds_to_ms(uint32_t seconds)
{
	return seconds * 1000;
}