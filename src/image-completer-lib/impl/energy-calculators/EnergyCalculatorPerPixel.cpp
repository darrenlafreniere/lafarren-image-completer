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
#include "EnergyCalculatorPerPixel.h"

#include "tech/Atomic.h"
#include "tech/Core.h"
#include "tech/MathUtils.h"

#include "EnergyCalculatorUtils.h"
#include "ImageConst.h"
#include "LfnIcSettings.h"
#include "MaskLod.h"

#include "tech/DbgMem.h"

namespace LfnIc
{
	// If the batch has maxCalculations has >= this value and there are worker
	// threads, queued calculations will be processed over all available hardware
	// threads. Attempting asynchronous batches with fewer calculations than this
	// might actually be slower due to synchronization overhead.
	//
	// NOTE: value is arbitrary. Do some tests to find the sweet spot.
	const int MIN_CALCULATIONS_FOR_ASYNC_BATCH = 30;

	//
	// PolicyNoMask - handles straight pixel SSD calculations without any masking.
	// This policy, along with the non-specialized CalculateEnergy function, provide
	// an extremely efficient implementation for pixels with exactly 3 unsigned char channels.
	//
	class PolicyNoMask_24BitRgb
	{
	public:
		static const bool HAS_MASK = false;
		typedef uint32 ResultType;

		inline void OnPreLoop(const Mask* mask) {}
		inline void OnARow(int aSrcIndex) {}
		inline void OnBRow(int bSrcIndex) {}

		inline int GetMaxPixelsPerBunch() const
		{
			// MaxPixelsForUint32Energy is how many pixels a uint32 energy variable
			// can safely capture without overflowing, assuming the worst case of
			// a pure black patch vs pure white patch, where each channel difference is
			// 255. This is a 32-bit application, but the Energy typedef is 64-bit because
			// of how large the patches can be. Performing the energy calculations in
			// 64-bit has a big performance penalty, so calculate in 32-bit bunches,
			// dumping the bunch result into the 64-bit energy result before reaching
			// overflow.
			static const int MAX_CHANNEL_VALUE = std::numeric_limits<Image::Pixel::ChannelType>::max();
			static const int MAX_ENERGY_PER_PIXEL = (MAX_CHANNEL_VALUE * MAX_CHANNEL_VALUE) * Image::Pixel::NUM_CHANNELS;
			static const int MAX_PIXELS_PER_RESULT = std::numeric_limits<ResultType>::max() / MAX_ENERGY_PER_PIXEL;
			return MAX_PIXELS_PER_RESULT;
		}

		FORCE_INLINE ResultType CalculateSquaredDifference(const Image::Pixel* aSrcRow, const Image::Pixel* bSrcRow, int x)
		{
			const Image::Pixel& a = aSrcRow[x];
			const Image::Pixel& b = bSrcRow[x];

			// d[x] = channel delta
			// e = dr^2 + dg^2 + db^2
			const ResultType dr = a.channel[0] - b.channel[0];
			const ResultType dg = a.channel[1] - b.channel[1];
			const ResultType db = a.channel[2] - b.channel[2];
			return (dr * dr) + (dg * dg) + (db * db);
		}
	};

	class PolicyNoMask_General
	{
	public:
		static const bool HAS_MASK = false;
		typedef float ResultType;

		inline void OnPreLoop(const Mask* mask) {}
		inline void OnARow(int aSrcIndex) {}
		inline void OnBRow(int bSrcIndex) {}

		inline int GetMaxPixelsPerBunch() const
		{
			// It's difficult to get a hard limit for how much a floating
			// point result type can hold without overflowing, since the
			// input pixel range is unknown. This 32x32 bunch limit is
			// arbitrary.
			// TODO: base this on std::numeric_limits<ResultType>::digits.
			return 32 * 32;
		}

		FORCE_INLINE ResultType CalculateSquaredDifference(const Image::Pixel* aSrcRow, const Image::Pixel* bSrcRow, int x)
		{
			const Image::Pixel& a = aSrcRow[x];
			const Image::Pixel& b = bSrcRow[x];

			ResultType squaredDifference = ResultType(0);

			for (int i = 0; i < Image::Pixel::NUM_CHANNELS; ++i)
			{
				squaredDifference += (a.channel[i] - b.channel[i]) * (a.channel[i] - b.channel[i]);
			}

			return squaredDifference;
		}
	};

	//
	// PolicyMask - base class for testing one of the regions against the mask
	//
	template<typename POLICY_NO_MASK>
	class PolicyMask : public POLICY_NO_MASK
	{
	public:
		static const bool HAS_MASK = true;
		typedef POLICY_NO_MASK Super;
		typedef typename Super::ResultType ResultType;

		inline void OnPreLoop(const MaskLod* mask)
		{
			m_lodBuffer = mask ? mask->GetLodBuffer(mask->GetHighestLod()) : NULL;
		}

		inline ResultType CalculateSquaredDifference(const Image::Pixel* aSrcRow, const Image::Pixel* bSrcRow, int x)
		{
			return (!m_lodRow || m_lodRow[x] == Mask::KNOWN)
				? Super::CalculateSquaredDifference(aSrcRow, bSrcRow, x)
				: ResultType(0);
		}

	protected:
		const Mask::Value* m_lodBuffer;
		const Mask::Value* m_lodRow;
	};

	//
	// PolicyMaskA - tests region A against the mask
	//
	template<typename POLICY_NO_MASK>
	class PolicyMaskA : public PolicyMask<POLICY_NO_MASK>
	{
	public:
		typedef PolicyMask<POLICY_NO_MASK> Super;

		inline void OnARow(int aSrcIndex)
		{
			Super::m_lodRow = Super::m_lodBuffer ? (Super::m_lodBuffer + aSrcIndex) : NULL;
		}
	};

	typedef PolicyMaskA<PolicyNoMask_24BitRgb> PolicyMaskA_24BitRgb;
	typedef PolicyMaskA<PolicyNoMask_General> PolicyMaskA_General;

	//
	// General purpose energy calculation template. Performs masking via a policy
	// template parameter. Because the policy is resolved at compile time, the
	// mask testing is compiled out when it's not needed.
	//
	template<typename POLICY>
	static inline Energy CalculateEnergy(
		const ImageConst& inputImage, const MaskLod* mask,
		int width, int height,
		int aLeft, int aTop,
		int bLeft, int bTop)
	{
		Energy energy64Bit = Energy(0);

		const int imageWidth = inputImage.GetWidth();
		const int imageHeight = inputImage.GetHeight();

		EnergyCalculatorUtils::ClampToMinBoundary(aLeft, bLeft, width, 0);
		EnergyCalculatorUtils::ClampToMinBoundary(aTop, bTop, height, 0);
		EnergyCalculatorUtils::ClampToMaxBoundary(aLeft, bLeft, width, imageWidth);
		EnergyCalculatorUtils::ClampToMaxBoundary(aTop, bTop, height, imageHeight);

		if (width > 0 && height > 0)
		{
			POLICY policy;
			policy.OnPreLoop(mask);

			typename POLICY::ResultType energyBunch = 0;
			int numPixelsInBunch = 0;
			const bool canFitInSingleBunch = (width * height) <= policy.GetMaxPixelsPerBunch();

			const Image::Pixel* inputImageRgb = inputImage.GetData();
			int aRowIndex = LfnTech::GetRowMajorIndex(imageWidth, aLeft, aTop);
			int bRowIndex = LfnTech::GetRowMajorIndex(imageWidth, bLeft, bTop);
			for (int y = 0; y < height; ++y, aRowIndex += imageWidth, bRowIndex += imageWidth)
			{
				const Image::Pixel* aRow = inputImageRgb + aRowIndex;
				const Image::Pixel* bRow = inputImageRgb + bRowIndex;
				policy.OnARow(aRowIndex);
				policy.OnBRow(bRowIndex);

				if (canFitInSingleBunch)
				{
					for (int x = 0; x < width; ++x)
					{
						energyBunch += policy.CalculateSquaredDifference(aRow, bRow, x++);
					}
				}
				else
				{
					int x = 0;
					do
					{
						const int remainingPixelsInRow = width - x;
						const bool shouldDumpBunchAfterStrip = (remainingPixelsInRow > policy.GetMaxPixelsPerBunch());
						const int stripWidth = shouldDumpBunchAfterStrip ? policy.GetMaxPixelsPerBunch() : remainingPixelsInRow;

						while (x < stripWidth)
						{
							energyBunch += policy.CalculateSquaredDifference(aRow, bRow, x++);
						}

						if (shouldDumpBunchAfterStrip)
						{
							energy64Bit += energyBunch;
							energyBunch = 0;
							numPixelsInBunch = 0;
						}
						else
						{
							numPixelsInBunch += stripWidth;
						}
					}
					while (x < width);
				}
			}

			// Add what's left in the bunch.
			energy64Bit += energyBunch;
		}

		wxASSERT(energy64Bit >= ENERGY_MIN && energy64Bit <= ENERGY_MAX);
		return energy64Bit;
	}
}

//
// EnergyCalculatorPerPixel implementation
//
LfnIc::EnergyCalculatorPerPixel::EnergyCalculatorPerPixel(const ImageConst& inputImage, const MaskLod& mask) :
m_inputImage(inputImage),
	m_mask(mask),
	m_batchState(BatchStateClosed),
	m_isAsyncBatch(false),
	m_queuedCalculationAndResultIndexBuffer(*this),
	m_targetThreadIndex(0)
{
#ifdef USE_THREADS
	const int cpuCount = wxThread::GetCPUCount();
	if (cpuCount > 1)
	{
		const int numWorkerThreads = cpuCount - 1;
		for (int i = 0; i < numWorkerThreads; ++i)
		{
			WorkerThread* workerThread = new WorkerThread(*this);
			m_workerThreads.push_back(workerThread);

			wxASSERT(workerThread->IsPaused());
		}
	}
#endif
}

LfnIc::EnergyCalculatorPerPixel::~EnergyCalculatorPerPixel()
{
	for (int i = 0, n = m_workerThreads.size(); i < n; ++i)
	{
		WorkerThread* workerThread = m_workerThreads[i];
		workerThread->ResumeAndQuit();
		delete workerThread;
	}
}

void LfnIc::EnergyCalculatorPerPixel::BatchOpenImmediate(const BatchParams& params)
{
	wxASSERT(m_batchState == BatchStateClosed);
	m_batchState = BatchStateOpenImmediate;
	m_batchParams = params;
}

void LfnIc::EnergyCalculatorPerPixel::BatchOpenQueued(const BatchParams& params)
{
	wxASSERT(m_batchState == BatchStateClosed);
	m_batchState = BatchStateOpenQueued;
	m_batchParams = params;

	// Set the capacity and clear any previous data.
	{
		m_queuedCalculationsAndResults.clear();
		m_queuedCalculationsAndResults.reserve(m_batchParams.maxCalculations);
	}

	{
		m_isAsyncBatch = (m_workerThreads.size() > 0 && m_batchParams.maxCalculations >= MIN_CALCULATIONS_FOR_ASYNC_BATCH);
	}
}

void LfnIc::EnergyCalculatorPerPixel::BatchClose()
{
	wxASSERT(m_batchState != BatchStateClosed);
	m_batchState = BatchStateClosed;
	m_isAsyncBatch = false;
}

LfnIc::Energy LfnIc::EnergyCalculatorPerPixel::Calculate(int bLeft, int bTop) const
{
	wxASSERT(m_batchState != BatchStateClosed);

	if (LfnIc::Image::PixelInfo::IS_24_BIT_RGB)
	{
		if (m_batchParams.aMasked)
		{
			return CalculateMaskA<PolicyMaskA_24BitRgb>(bLeft, bTop);
		}
		else
		{
			return CalculateNoMask<PolicyNoMask_24BitRgb>(bLeft, bTop);
		}
	}
	else
	{
		if (m_batchParams.aMasked)
		{
			return CalculateMaskA<PolicyMaskA_General>(bLeft, bTop);
		}
		else
		{
			return CalculateNoMask<PolicyNoMask_General>(bLeft, bTop);
		}
	}
} // end Calculate

LfnIc::EnergyCalculator::BatchQueued::Handle LfnIc::EnergyCalculatorPerPixel::QueueCalculation(int bLeft, int bTop)
{
	wxASSERT(m_batchState != BatchStateClosed);

	wxASSERT(m_queuedCalculationsAndResults.size() + 1 <= m_queuedCalculationsAndResults.capacity());
	const uint queuedCalculationAndResultIndex = m_queuedCalculationsAndResults.size();

	// Add QueuedCalculationAndResult.
	{
		QueuedCalculationAndResult queuedCalculationAndResult;
		queuedCalculationAndResult.bLeft = bLeft;
		queuedCalculationAndResult.bTop = bTop;
		m_queuedCalculationsAndResults.push_back(queuedCalculationAndResult);
	}

	// Give index of new QueuedCalculationAndResult to the next target thread.
	{
		// See comments above m_targetThreadIndex member.
		QueuedCalculationAndResultIndexBuffer& queuedCalculationAndResultIndexBuffer = (!m_isAsyncBatch || static_cast<unsigned int>(m_targetThreadIndex) == m_workerThreads.size())
			? m_queuedCalculationAndResultIndexBuffer                                           // main thread
			: m_workerThreads[m_targetThreadIndex]->GetQueuedCalculationAndResultIndexBuffer(); // worker thread

		queuedCalculationAndResultIndexBuffer.push_back(queuedCalculationAndResultIndex);

		// Cycle to the next thread index.
		m_targetThreadIndex = (m_targetThreadIndex + 1) % (m_workerThreads.size() + 1);
	}

	return BatchQueued::Handle(queuedCalculationAndResultIndex);
}

void LfnIc::EnergyCalculatorPerPixel::ProcessCalculations()
{
	// Resume the worker threads and have them process their calculations.
	if (m_isAsyncBatch)
	{
		for (int i = 0, n = m_workerThreads.size(); i < n; ++i)
		{
			wxASSERT(m_workerThreads[i]->IsPaused());
			m_workerThreads[i]->ResumeAndStartProcessingCalculations();
			wxASSERT(!m_workerThreads[i]->IsPaused());
		}
	}

	// Process the main thread's calculations.
	m_queuedCalculationAndResultIndexBuffer.ProcessCalculationsAndClear();

	// Wait for all worker threads to finish.
	if (m_isAsyncBatch)
	{
		for (int i = 0, n = m_workerThreads.size(); i < n; ++i)
		{
			wxASSERT(!m_workerThreads[i]->IsPaused());
			m_workerThreads[i]->FinishProcessingCalculationsAndPause();
			wxASSERT(m_workerThreads[i]->IsPaused());
		}
	}

	m_batchState = BatchStateOpenQueuedAndProcessed;
}

LfnIc::Energy LfnIc::EnergyCalculatorPerPixel::GetResult(BatchQueued::Handle handle) const
{
	wxASSERT(m_batchState == BatchStateOpenQueuedAndProcessed);
	return m_queuedCalculationsAndResults[handle].result;
}

template <typename POLICY>
LfnIc::Energy LfnIc::EnergyCalculatorPerPixel::CalculateNoMask(int bLeft, int bTop) const
{
	wxCOMPILE_TIME_ASSERT(!POLICY::HAS_MASK, CalculateNoMask_IsCalledWithAMaskPolicy);
	return CalculateEnergy<POLICY>(
		m_inputImage, NULL,
		m_batchParams.width, m_batchParams.height,
		m_batchParams.aLeft, m_batchParams.aTop,
		bLeft, bTop);
}

template<typename POLICY>
LfnIc::Energy LfnIc::EnergyCalculatorPerPixel::CalculateMaskA(int bLeft, int bTop) const
{
	wxCOMPILE_TIME_ASSERT(POLICY::HAS_MASK, CalculateMaskA_IsCalledWithANoMaskPolicy);
	return CalculateEnergy<POLICY>(
		m_inputImage, &m_mask,
		m_batchParams.width, m_batchParams.height,
		m_batchParams.aLeft, m_batchParams.aTop,
		bLeft, bTop);
}

LfnIc::EnergyCalculatorPerPixel::QueuedCalculationAndResultIndexBuffer::QueuedCalculationAndResultIndexBuffer(EnergyCalculatorPerPixel& energyCalculatorPerPixel) :
m_energyCalculatorPerPixel(energyCalculatorPerPixel)
{
}

void LfnIc::EnergyCalculatorPerPixel::QueuedCalculationAndResultIndexBuffer::ProcessCalculationsAndClear()
{
	for (int i = 0, n = size(); i < n; ++i)
	{
		QueuedCalculationAndResult& queuedCalculationAndResult = m_energyCalculatorPerPixel.m_queuedCalculationsAndResults[at(i)];
		queuedCalculationAndResult.result = m_energyCalculatorPerPixel.Calculate(queuedCalculationAndResult.bLeft, queuedCalculationAndResult.bTop);
	}

	clear();
}

LfnIc::EnergyCalculatorPerPixel::WorkerThread::WorkerThread(EnergyCalculatorPerPixel& energyCalculatorPerPixel) :
wxThread(wxTHREAD_JOINABLE),
	m_energyCalculatorPerPixel(energyCalculatorPerPixel),
	m_queuedCalculationAndResultIndexBuffer(energyCalculatorPerPixel),
	m_state(Active)
{
	Create();
	Run();
	FinishProcessingCalculationsAndPause();
}

wxThread::ExitCode LfnIc::EnergyCalculatorPerPixel::WorkerThread::Entry()
{
	State state = Active;
	while (state != Quitting)
	{
		m_queuedCalculationAndResultIndexBuffer.ProcessCalculationsAndClear();

		// Just query the state; it's never expected to be invalid, the
		// exchange and comparand are dummies.
		state = AtomicGetState();

		// If the main thread wants us to pause, safely loop in here till it
		// wants to resume.
		if (state == Pausing)
		{
			// Set the paused state, and verify that the main thread's sync
			// loop did its job by waiting.
			{
				const State previousState = State(LfnTech::Atomic<>::CompareExchange(&m_state, Paused, Pausing));
				wxASSERT(previousState == Pausing);
			}

			// Loop until the resuming state is set.
			while ((state = AtomicGetState()) == Paused)
			{
				wxThread::Yield();
			}

			// If we're resuming, set the active state, and verify that the
			// main thread's sync loop did its job by waiting.
			if (state == Resuming)
			{
				const State previousState = State(LfnTech::Atomic<>::CompareExchange(&m_state, Active, Resuming));
				wxASSERT(previousState == Resuming);
			}
		}
	}

	return 0;
}

bool LfnIc::EnergyCalculatorPerPixel::WorkerThread::IsPaused() const
{
	return wxThread::IsPaused();
}

void LfnIc::EnergyCalculatorPerPixel::WorkerThread::ResumeAndStartProcessingCalculations()
{
	wxASSERT(IsPaused());

	const State previousState = State(LfnTech::Atomic<>::CompareExchange(&m_state, Resuming, Paused));
	wxASSERT(previousState == Paused);

	wxThread::Resume();

	// No need to wait until thread sets state to active;
	// FinishProcessingCalculationsAndPause() handles all synchronization.
}

void LfnIc::EnergyCalculatorPerPixel::WorkerThread::FinishProcessingCalculationsAndPause()
{
	wxASSERT(!IsPaused());

	// Sync: loop until state is Active, which is required to set the Pausing
	// state. This is done in case ResumeAndStartProcessingCalculations()
	// was just called and the thread hasn't had a chance to wake up.
	while (LfnTech::Atomic<>::CompareExchange(&m_state, Pausing, Active) != Active)
	{
		wxThread::Yield();
	}

	// Sync: loop until state is Paused, which won't happen until the thread
	// has finished processing.
	while (AtomicGetState() != Paused)
	{
		wxThread::Yield();
	}

	wxThread::Pause();
}

void LfnIc::EnergyCalculatorPerPixel::WorkerThread::ResumeAndQuit()
{
	wxASSERT(IsPaused());

	const State previousState = State(LfnTech::Atomic<>::CompareExchange(&m_state, Quitting, Paused));
	wxASSERT(previousState == Paused);

	wxThread::Resume();

	// Joinable thread, wait for it to join.
	Wait();
}

LfnIc::EnergyCalculatorPerPixel::WorkerThread::State LfnIc::EnergyCalculatorPerPixel::WorkerThread::AtomicGetState() const
{
	// The exchange and comparand are dummies.
	return State(LfnTech::Atomic<>::CompareExchange(&m_state, Invalid, Invalid));
}
