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

#ifndef ENERGY_CALCULATOR_PER_PIXEL_H
#define ENERGY_CALCULATOR_PER_PIXEL_H

#include "EnergyCalculator.h"

namespace LfnIc
{
	// Forward declarations
	class ImageConst;
	class MaskLod;

	// Calculates the energy between two regions of the input image by doing
	// straight per-pixel calculations on the CPU.
	class EnergyCalculatorPerPixel : public EnergyCalculator
	{
	public:
		EnergyCalculatorPerPixel(const ImageConst& inputImage, const MaskLod& mask);
		virtual ~EnergyCalculatorPerPixel();

	private:
		//
		// Internal definitions
		//

		enum BatchState
		{
			BatchStateClosed,
			BatchStateOpenImmediate,
			BatchStateOpenQueued,
			BatchStateOpenQueuedAndProcessed,
		};

		// Each queued calculation parameters and its result is held in this
		// structure, in the m_queuedCalculationsAndResults member.
		struct QueuedCalculationAndResult
		{
			int bLeft;
			int bTop;
			Energy result;
		};

		// An index buffer into the m_queuedCalculationsAndResults member.
		// Stores the indices that a given thread is responsible for
		// processing.
		class QueuedCalculationAndResultIndexBuffer : private std::vector<uint>
		{
		public:
			typedef std::vector<uint> Super;

			QueuedCalculationAndResultIndexBuffer(EnergyCalculatorPerPixel& energyCalculatorPerPixel);

			using Super::push_back;
			void ProcessCalculationsAndClear();

		private:
			EnergyCalculatorPerPixel& m_energyCalculatorPerPixel;
		};

		// Worker thread for parallel calculation processing. New instances
		// automatically initialize themselves in a paused state.
		class WorkerThread : private wxThread
		{
		public:
			WorkerThread(EnergyCalculatorPerPixel& energyCalculatorPerPixel);
			virtual ExitCode Entry();

			inline QueuedCalculationAndResultIndexBuffer& GetQueuedCalculationAndResultIndexBuffer() { return m_queuedCalculationAndResultIndexBuffer; }

			bool IsPaused() const;

			// Resumes the thread and starts processing its calculations.
			void ResumeAndStartProcessingCalculations();

			// Finishes processing this's thread's calculations, and pauses
			// the thread.
			void FinishProcessingCalculationsAndPause();

			// Causes the worker thread to gracefully quit altogether.
			void ResumeAndQuit();

		private:
			enum State
			{
				Invalid = -1,
				Quitting,
				Paused,
				Pausing,
				Resuming,
				Active,
			};

			// Returns the current state using an atomic operation.
			inline State AtomicGetState() const;

			EnergyCalculatorPerPixel& m_energyCalculatorPerPixel;

			// This thread's index buffer into m_energyCalculatorPerPixel.m_queuedCalculationsAndResults.
			QueuedCalculationAndResultIndexBuffer m_queuedCalculationAndResultIndexBuffer;

			mutable long volatile m_state;
		};

		friend class WorkerThread;

		//
		// Internal methods
		//

		// EnergyCalculator interface
		virtual void BatchOpenImmediate(const BatchParams& params);
		virtual void BatchOpenQueued(const BatchParams& params);
		virtual void BatchClose();
		virtual Energy Calculate(int bLeft, int bTop) const;
		virtual BatchQueued::Handle QueueCalculation(int bLeft, int bTop);
		virtual void ProcessCalculations();
		virtual Energy GetResult(BatchQueued::Handle handle) const;

		template<typename POLICY>
		Energy CalculateNoMask(int bLeft, int bTop) const;

		template<typename POLICY>
		Energy CalculateMaskA(int bLeft, int bTop) const;

		//
		// Data
		//
		const ImageConst& m_inputImage;
		const MaskLod& m_mask;
		BatchState m_batchState;
		BatchParams m_batchParams;
		bool m_isAsyncBatch;

		std::vector<WorkerThread*> m_workerThreads;

		// All queued calculations and results.
		std::vector<QueuedCalculationAndResult> m_queuedCalculationsAndResults;

		// Main thread's index buffer into m_queuedCalculationsAndResults.
		QueuedCalculationAndResultIndexBuffer m_queuedCalculationAndResultIndexBuffer;

		// Cycles through the thread indices, and is the target for the next
		// queued calculation (index == m_workerThreads.size() indicates the
		// main thread).
		int m_targetThreadIndex;
	};
}

#endif
