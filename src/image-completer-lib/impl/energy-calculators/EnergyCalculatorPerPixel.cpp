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
#include "Image.h"
#include "Mask.h"
#include "PriorityBpSettings.h"

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

	// MAX_PIXELS_FOR_UNSIGNED_32_BIT_ENERGY is how many pixels a uint32 energy
	// variable can safely capture without overflowing, assuming the worst case of
	// a pure black patch vs pure white patch, where each component difference is
	// 255. This is a 32-bit application, but the Energy typedef is 64-bit because
	// of how large the patches can be. Performing the energy calculations in
	// 64-bit has a big performance penalty, so calculate in 32-bit batches,
	// dumping the batch result into the 64-bit energy result before reaching
	// overflow.
	const int MAX_ENERGY_PER_PIXEL = (255 * 255) + (255 * 255) + (255 * 255);
	const int MAX_PIXELS_FOR_UNSIGNED_32_BIT_ENERGY = UINT_MAX / MAX_ENERGY_PER_PIXEL;

	//
	// PolicyNoMask - handles straight pixel SSD calculations without any masking.
	//
	class PolicyNoMask
	{
	public:
		inline void OnPreLoop(const Mask* mask) {}
		inline void OnARow(int aSrcIndex) {}
		inline void OnBRow(int bSrcIndex) {}

		// See MAX_PIXELS_FOR_UNSIGNED_32_BIT_ENERGY for why this uses uint32.
		FORCE_INLINE uint32 CalculateSquaredDifference(const Image::Rgb* aSrcRow, const Image::Rgb* bSrcRow, int x)
		{
			const Image::Rgb& a = aSrcRow[x];
			const Image::Rgb& b = bSrcRow[x];

			// d[x] = component delta
			// e = dr^2 + dg^2 + db^2
			const uint32 dr = a.red   - b.red;
			const uint32 dg = a.green - b.green;
			const uint32 db = a.blue  - b.blue;
			return (dr * dr) + (dg * dg) + (db * db);
		}
	};

	//
	// PolicyMask - base class for testing one of the regions against the mask
	//
	class PolicyMask : public PolicyNoMask
	{
	public:
		typedef PolicyNoMask Super;

		inline void OnPreLoop(const MaskLod* mask)
		{
			m_lodBuffer = mask ? mask->GetLodBuffer(mask->GetHighestLod()) : NULL;
		}

		// See MAX_PIXELS_FOR_UNSIGNED_32_BIT_ENERGY for why this uses uint32.
		inline uint32 CalculateSquaredDifference(const Image::Rgb* aSrcRow, const Image::Rgb* bSrcRow, int x)
		{
			return (!m_lodRow || m_lodRow[x] == Mask::KNOWN)
				? Super::CalculateSquaredDifference(aSrcRow, bSrcRow, x)
				: 0;
		}

	protected:
		const Mask::Value* m_lodBuffer;
		const Mask::Value* m_lodRow;
	};

	//
	// PolicyMaskA - tests region A against the mask
	//
	class PolicyMaskA : public PolicyMask
	{
	public:
		typedef PolicyMask Super;

		inline void OnARow(int aSrcIndex)
		{
			m_lodRow = m_lodBuffer ? (m_lodBuffer + aSrcIndex) : NULL;
		}
	};

	//
	// PolicyMaskA - tests region B against the mask
	//
	class PolicyMaskB : public PolicyMask
	{
	public:
		typedef PolicyMask Super;

		inline void OnBRow(int bSrcIndex)
		{
			m_lodRow = m_lodBuffer ? (m_lodBuffer + bSrcIndex) : NULL;
		}
	};

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

			// See MAX_PIXELS_FOR_UNSIGNED_32_BIT_ENERGY for why energy calculation is
			// divided into potentially multiple batches using uint32.
			uint32 energyU32Bit = 0;
			int numPixelsInBatch = 0;
			const bool canFitInU32Bit = (width * height) <= MAX_PIXELS_FOR_UNSIGNED_32_BIT_ENERGY;

			const Image::Rgb* inputImageRgb = inputImage.GetRgb();
			int aRowIndex = LfnTech::GetRowMajorIndex(imageWidth, aLeft, aTop);
			int bRowIndex = LfnTech::GetRowMajorIndex(imageWidth, bLeft, bTop);
			for (int y = 0; y < height; ++y, aRowIndex += imageWidth, bRowIndex += imageWidth)
			{
				const Image::Rgb* aRow = inputImageRgb + aRowIndex;
				const Image::Rgb* bRow = inputImageRgb + bRowIndex;
				policy.OnARow(aRowIndex);
				policy.OnBRow(bRowIndex);

				if (canFitInU32Bit)
				{
					for (int x = 0; x < width; ++x)
					{
						energyU32Bit += policy.CalculateSquaredDifference(aRow, bRow, x++);
					}
				}
				else
				{
					int x = 0;
					do
					{
						const int remainingPixelsInRow = width - x;
						const bool shouldDumpBatchAfterStrip = (remainingPixelsInRow > MAX_PIXELS_FOR_UNSIGNED_32_BIT_ENERGY);
						const int stripWidth = shouldDumpBatchAfterStrip ? MAX_PIXELS_FOR_UNSIGNED_32_BIT_ENERGY : remainingPixelsInRow;

						while (x < stripWidth)
						{
							energyU32Bit += policy.CalculateSquaredDifference(aRow, bRow, x++);
						}

						if (shouldDumpBatchAfterStrip)
						{
							energy64Bit += energyU32Bit;
							energyU32Bit = 0;
							numPixelsInBatch = 0;
						}
						else
						{
							numPixelsInBatch += stripWidth;
						}
					}
					while (x < width);
				}
			}

			// Add what's left in the batch.
			energy64Bit += energyU32Bit;
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

LfnIc::EnergyCalculator::BatchImmediate LfnIc::EnergyCalculatorPerPixel::BatchOpenImmediate(const BatchParams& params)
{
	wxASSERT(m_batchState == BatchStateClosed);
	m_batchState = BatchStateOpenImmediate;
	m_batchParams = params;

	return GetBatchImmediate(*this);
}

LfnIc::EnergyCalculator::BatchQueued LfnIc::EnergyCalculatorPerPixel::BatchOpenQueued(const BatchParams& params)
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

	return GetBatchQueued(*this);
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

	if (m_batchParams.aMasked)
	{
		return CalculateMaskA(bLeft, bTop);
	}
	else
	{
		return CalculateNoMask(bLeft, bTop);
	}
}

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

LfnIc::Energy LfnIc::EnergyCalculatorPerPixel::CalculateNoMask(int bLeft, int bTop) const
{
	return CalculateEnergy<PolicyNoMask>(
		m_inputImage, NULL,
		m_batchParams.width, m_batchParams.height,
		m_batchParams.aLeft, m_batchParams.aTop,
		bLeft, bTop);
}

LfnIc::Energy LfnIc::EnergyCalculatorPerPixel::CalculateMaskA(int bLeft, int bTop) const
{
	return CalculateEnergy<PolicyMaskA>(
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
