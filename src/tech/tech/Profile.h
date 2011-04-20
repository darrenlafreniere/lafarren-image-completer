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

#ifndef TECH_PROFILE_H
#define TECH_PROFILE_H

#if TECH_PROFILE
namespace LfnTech
{
	///
	/// Base class for time or memory profiling.
	///
	class BaseProfiler
	{
	public:
		BaseProfiler(const char* name);
		virtual ~BaseProfiler();

		inline const std::string& GetName() const { return m_name; }

		void Start();
		void Stop();

	protected:
		virtual void OnStart() = 0;
		virtual void OnStop() = 0;

	private:
		//
		// Data
		//
		std::string m_name;
		long volatile m_startCount;
	};

	///
	/// Profiles execution time.
	///
	class TimeProfiler : public BaseProfiler
	{
	public:
		enum ReportMode
		{
			// Report every sample
			ReportEverySample,

			// Report after every Nth sample
			ReportEveryNthSample,

			// Report when this profiler is destroyed
			ReportFinal,
		};

		TimeProfiler(const char* name, ReportMode reportMode = ReportFinal, int nthBlock = 0);
		virtual ~TimeProfiler();

	private:
		virtual void OnStart();
		virtual void OnStop();

		// Can be used internally by subclasses to differentiate runtime
		// reports from final reports.
		enum ReportContext
		{
			ReportContextRuntime,
			ReportContextFinal,
		};

		void Report(ReportContext, int numBlocks, double time);

		//
		// Data
		//
		int m_reportAfterNumSamples;
		int m_numSamples;
		double m_startTime;
		double m_timeSinceLastReport;
		double m_totalTime;
	};

	//
	// Profiles memory usage.
	//
	class MemProfiler : public BaseProfiler
	{
	public:
		MemProfiler(const char* name);
		virtual ~MemProfiler();

	private:
		virtual void OnStart();
		virtual void OnStop();

		//
		// Data
		//

		// Platform-specific data allocated in Profile.cpp
		class MemProfilerData;
		MemProfilerData* m_data;
	};

	//
	// Starts the profiler upon construction, and stops it upon destruction.
	//
	class ScopedProfiler
	{
	public:
		inline ScopedProfiler(BaseProfiler& profiler) :
		m_profiler(profiler)
		{
			m_profiler.Start();
		}

		inline ~ScopedProfiler()
		{
			m_profiler.Stop();
		}

	private:
		BaseProfiler& m_profiler;
	};
}

//
// Macros for scoped time profiling. Use as such:
//
//		void MyClass::MyMethod()
//		{
//			TECH_TIME_PROFILE("MyClass::MyMethod");
//			...
//
// The time profiler macros create static TimeProfiler objects. Therefore,
// the name passed into time profiler macros cannot change; the first name
// is used throughout execution.
//
// The TECH_PROFILE preprocessor flag must be set to 1

#if TECH_PROFILE_MACROS && defined(_MSC_VER)

// Reports the final profile:
#define TECH_TIME_PROFILE(__name__) \
	TECH_TIME_PROFILE_INDIRECT1(__LINE__, __name__, LfnTech::TimeProfiler::ReportFinal, 0)

// Reports every sample:
#define TECH_TIME_PROFILE_EVERY_SAMPLE(__name__) \
	TECH_TIME_PROFILE_INDIRECT1(__LINE__, __name__, LfnTech::TimeProfiler::ReportEverySample, 1)

// Reports every Nth sample:
#define TECH_TIME_PROFILE_EVERY_NTH_SAMPLE(__name__, __nthBlock__) \
	TECH_TIME_PROFILE_INDIRECT1(__LINE__, __name__, LfnTech::TimeProfiler::ReportEveryNthSample, __nthBlock__)

// Helper macros to successfully expand __LINE__
#define TECH_TIME_PROFILE_INDIRECT1(__line__, __name__, __reportMode__, __nthBlock__) \
	TECH_TIME_PROFILE_INDIRECT2(__line__, __name__, __reportMode__, __nthBlock__)

#define TECH_TIME_PROFILE_INDIRECT2(__line__, __name__, __reportMode__, __nthBlock__) \
	static LfnTech::TimeProfiler techTimeProfiler##__line__(__name__, __reportMode__, __nthBlock__); \
	LfnTech::ScopedProfiler techScopedProfiler##__line__(techTimeProfiler##__line__)

#else

#define TECH_TIME_PROFILE(__name__)
#define TECH_TIME_PROFILE_EVERY_SAMPLE(__name__)
#define TECH_TIME_PROFILE_EVERY_NTH_SAMPLE(__name__, __nthBlock__)

#endif // #if TECH_PROFILE_MACROS

//
// Macros for scoped memory profiling. Use as such:
//
//		void MyClass::MyMethod()
//		{
//			TECH_MEM_PROFILE("MyClass::MyMethod");
//			...
//
// The time profiler macros create non-static MemProfiler objects. Therefore,
// the name passed into time profiler macros can change on any given execution.
//
// The TECH_PROFILE preprocessor flag must be set to 1

#if TECH_PROFILE_MACROS && defined(_MSC_VER)

#define TECH_MEM_PROFILE(__name__) \
	TECH_MEM_PROFILE_INDIRECT1(__LINE__, __name__)

// Helper macros to successfully expand __LINE__
#define TECH_MEM_PROFILE_INDIRECT1(__line__, __name__) \
	TECH_MEM_PROFILE_INDIRECT2(__line__, __name__)

#define TECH_MEM_PROFILE_INDIRECT2(__line__, __name__) \
	LfnTech::MemProfiler techMemProfiler##__line__(__name__); \
	LfnTech::ScopedProfiler techScopedProfiler##__line__(techMemProfiler##__line__)

#else

#define TECH_MEM_PROFILE(__name__)

#endif // #if TECH_PROFILE_MACROS

#endif // #if TECH_PROFILE
#endif // #ifndef TECH_PROFILE_H
