//
// Copyright 2010, Darren Lafreniere
// <http://www.lafarren.com/image-completer/>
// 
// This file is part of lafarren.com's Image Completer.
// 
// Image Completer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Image Completer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Image Completer, named License.txt. If not, see
// <http://www.gnu.org/licenses/>.
//

#include "Pch.h"
#include "tech/Time.h"

#ifdef _MSC_VER
#define USE_A_WINDOWS_TIMER 1
#else
#define USE_A_WINDOWS_TIMER 0
#endif

#define USE_WINDOWS_QUERY_PERFORMANCE_TIMER     (0 && USE_A_WINDOWS_TIMER)
#define USE_WINDOWS_MULTIMEDIA_TIMER            (1 && USE_A_WINDOWS_TIMER)
#define USE_ANSI_TIMER                          (1 && !USE_A_WINDOWS_TIMER)

#define NUM_TIMERS_DEFINED              (USE_WINDOWS_QUERY_PERFORMANCE_TIMER + USE_WINDOWS_MULTIMEDIA_TIMER + USE_ANSI_TIMER)
wxCOMPILE_TIME_ASSERT(NUM_TIMERS_DEFINED >= 1, NoTimersAreDefined);
wxCOMPILE_TIME_ASSERT(NUM_TIMERS_DEFINED <= 1, MultipleTimersAreDefined);

#if USE_A_WINDOWS_TIMER
// Favor std::min and std::max over the min max macros defined by Windows
#define NOMINMAX
#include <windows.h>
#endif

#if USE_WINDOWS_MULTIMEDIA_TIMER
#include <mmsystem.h>
#include "tech/Atomic.h"
#endif

#include "tech/DbgMem.h"

// The Time singleton should be created early and destroyed late:
#pragma warning(disable: 4073)
#pragma init_seg(lib)

#if USE_WINDOWS_QUERY_PERFORMANCE_TIMER
#pragma message("Time.cpp using Windows' Query Performance Timer")
// Implements a Time class based on Windows' QueryPerformanceFrequency and
// QueryPerformanceCounter.
static const double LARGE_MULTIPLIER = 4294967296.0;
class Time
{
	Time()
	{
		LARGE_INTEGER rate;
		const BOOL result = QueryPerformanceFrequency(&rate);
		wxASSERT_MSG(result, "QueryPerformanceFrequency() failed.");
		m_timerRate = rate.u.LowPart + LARGE_MULTIPLIER * rate.u.HighPart;
	}

	static Time instance;

public:
	static const Time& GetInstance()
	{
		return instance;
	}

	double CurrentTime() const
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return (li.u.HighPart * LARGE_MULTIPLIER + li.u.LowPart) / m_timerRate;
	}

private:
	double m_timerRate;
};
#endif // USE_WINDOWS_QUERY_PERFORMANCE_TIMER

#if USE_WINDOWS_MULTIMEDIA_TIMER
#pragma message("Time.cpp using Windows' Multimedia Timer")
// Implements a Time class based on a Windows multimedia timer.
class Time
{
	static const UINT RESOLUTION_MS = 1;
	static const UINT INTERVAL_MS = 1;

	Time() :
	  m_resolution(0),
	  m_timerId(0),
	  m_timeMs(0)
	{
		if (m_resolution == 0.0 && m_timerId == 0)
		{
			TIMECAPS timeCaps;
			if (timeGetDevCaps(&timeCaps, sizeof(timeCaps)) == TIMERR_NOERROR)
			{
				m_resolution = std::max(RESOLUTION_MS, std::min(timeCaps.wPeriodMin, timeCaps.wPeriodMax));
				timeBeginPeriod(m_resolution);

				m_timerId = timeSetEvent(INTERVAL_MS, m_resolution, OnTimerStatic, (DWORD)this, TIME_PERIODIC);
			}
		}
	}

	~Time()
	{
		if (m_timerId != 0)
		{
			timeKillEvent(m_timerId);
			m_timerId = 0;
		}

		if (m_resolution != 0)
		{
			timeEndPeriod(m_resolution);
			m_resolution = 0;
		}
	}

	static Time instance;

public:
	static const Time& GetInstance()
	{
		return instance;
	}

	double CurrentTime() const
	{
		return double(m_timeMs) / 1000.0;
	}

private:
	static void CALLBACK OnTimerStatic(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
	{
		Time* thisPtr = (Time*)dwUser;
		LfnTech::Atomic<>::ExchangeAdd(&thisPtr->m_timeMs, INTERVAL_MS);
	}

	UINT m_resolution;
	MMRESULT m_timerId;
	LONG volatile m_timeMs;
};
#endif // USE_WINDOWS_MULTIMEDIA_TIMER

#if USE_ANSI_TIMER
#pragma message("Time.cpp using ANSI clock() Timer")
// Implements a Time class based on ANSI C's clock().
class Time
{
	static Time instance;

public:
	static const Time& GetInstance()
	{
		return instance;
	}

	double CurrentTime() const
	{
		const double t = static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC);
		return t;
	}
};
#endif // USE_ANSI_TIMER

Time Time::instance;

namespace LfnTech
{
	double CurrentTime()
	{
		return Time::GetInstance().CurrentTime();
	}
}
