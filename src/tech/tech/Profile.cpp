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
#include "tech/Profile.h"

#if TECH_PROFILE
#include "tech/Atomic.h"
#include "tech/Time.h"

#ifdef _MSC_VER
// Favor std::min and std::max over the min max macros defined by Windows
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#endif

#include "tech/DbgMem.h"

using namespace Lafarren;

//
// BaseProfiler
//
BaseProfiler::BaseProfiler(const char* name) :
m_name(name),
m_startCount(0)
{
}

BaseProfiler::~BaseProfiler()
{
}

void BaseProfiler::Start()
{
	if (Atomic<>::Increment(&m_startCount) == 1)
	{
		OnStart();
	}
}

void BaseProfiler::Stop()
{
	if (Atomic<>::Decrement(&m_startCount) == 0)
	{
		OnStop();
	}
}

//
// TimeProfiler
//
TimeProfiler::TimeProfiler(const char* name, ReportMode report, int nthBlock) :
BaseProfiler(name),
m_reportAfterNumSamples(0),
m_numSamples(0),
m_startTime(0.0),
m_timeSinceLastReport(0.0),
m_totalTime(0.0)
{
	switch (report)
	{
		case ReportEverySample:
		{
			m_reportAfterNumSamples = 1;
		}
		break;
		case ReportEveryNthSample:
		{
			m_reportAfterNumSamples = nthBlock;
		}
		break;
		case ReportFinal:
		{
			m_reportAfterNumSamples = 0;
		}
		break;
	}
}

TimeProfiler::~TimeProfiler()
{
	Report(ReportContextFinal, m_numSamples, m_totalTime);
}

void TimeProfiler::OnStart()
{
	m_startTime = CurrentTime();
}

void TimeProfiler::OnStop()
{
	const double delta = CurrentTime() - m_startTime;
	if (delta >= 0.0f)
	{
		++m_numSamples;
		m_timeSinceLastReport += delta;

		m_totalTime += delta;

		if (m_reportAfterNumSamples > 0 && (m_numSamples % m_reportAfterNumSamples) == 0)
		{
			Report(ReportContextRuntime, m_reportAfterNumSamples, m_timeSinceLastReport);
			m_timeSinceLastReport = 0.0;
		}
	}
}

void TimeProfiler::Report(ReportContext reportContext, int numBlocks, double time)
{
	const double timePerBlock = (numBlocks > 0)
		? (time / double(numBlocks))
		: 0.0;

	const char* reportFormat = (reportContext == ReportContextRuntime)
		? "TIME PROFILE \'%s\' - total time: %g sec, blocks: %d, time per block: %g sec\n"
		: "TIME PROFILE FINAL \'%s\' - total time: %g sec, blocks: %d, time per block: %g sec\n";

	char buffer[2048];
	sprintf_s(
		buffer,
		reportFormat,
		GetName().c_str(),
		time,
		numBlocks,
		timePerBlock);

	// TODO: add other logging options
	OutputDebugString(buffer);
}

//
// MemProfiler
//
#ifdef _MSC_VER
struct MemProfilerData
{
	PROCESS_MEMORY_COUNTERS startMem;
};
#endif

MemProfiler::MemProfiler(const char* name) :
BaseProfiler(name),
m_data(NULL)
{
#ifdef _MSC_VER
	m_data = new MemProfilerData;
	memset(m_data, 0, sizeof(MemProfilerData));
#else
#error "MemProfiler::MemProfiler() is not implemented for this platform"
#endif
}

MemProfiler::~MemProfiler()
{
	delete m_data;
}

void MemProfiler::OnStart()
{
#ifdef _MSC_VER
	MemProfilerData* data = (MemProfilerData*)m_data;
	GetProcessMemoryInfo(GetCurrentProcess(), &data->startMem, sizeof(data->startMem));
#else
#error "MemProfiler::OnStart() is not implemented for this platform"
#endif
}

void MemProfiler::OnStop()
{
	unsigned int startUsage     = 0;
	unsigned int startUsagePeak = 0;
	unsigned int endUsage       = 0;
	unsigned int endUsagePeak   = 0;

#ifdef _MSC_VER
	PROCESS_MEMORY_COUNTERS stopMem;
	GetProcessMemoryInfo(GetCurrentProcess(), &stopMem, sizeof(stopMem));

	MemProfilerData* data = (MemProfilerData*)m_data;
	startUsage     = (unsigned int)data->startMem.WorkingSetSize;
	startUsagePeak = (unsigned int)data->startMem.PeakWorkingSetSize;
	endUsage       = (unsigned int)stopMem.WorkingSetSize;
	endUsagePeak   = (unsigned int)stopMem.PeakWorkingSetSize;
#else
#error "MemProfiler::OnStop() is not implemented for this platform"
#endif

	static const float bytesToMegs = 1.0f / float(1 << 20);
	const char* reportFormat = "MEM PROFILE \'%s\' - start: %.2fM, end: %.2fM {peak start: %.2fM, end: %.2fM}\n";

	char buffer[2048];
	sprintf_s(
		buffer,
		reportFormat,
		GetName().c_str(),
		bytesToMegs * startUsage,
		bytesToMegs * endUsage,
		bytesToMegs * startUsagePeak,
		bytesToMegs * endUsagePeak);

	// TODO: add other logging options
	OutputDebugString(buffer);
}

#endif // #if TECH_PROFILE
