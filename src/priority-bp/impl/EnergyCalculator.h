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

#ifndef ENERGY_CALCULATOR_H
#define ENERGY_CALCULATOR_H

#include "tech/Core.h"
#include "PriorityBpTypes.h"

namespace PriorityBp
{
	//
	// Abstract interface for energy calculation. For batches of only one to a
	// couple calculations, use the Immediate batch type. For batches with a
	// large number of calculation, the Queued batch type may provide better
	// performance if the EnergyCalculator supports parallelism across
	// hardware threads.
	//
	class EnergyCalculator
	{
	public:
		// See comment above class for info on when to use this batch type.
		// Batch automatically closes when the BatchImmediate instance goes
		// out of scope.
		class BatchImmediate
		{
		public:
			// Closes the batch.
			inline ~BatchImmediate();

			// Given the left-top coordinate of block B, this method returns the
			// energy of block A against B. Calculates and returns the energy
			// immediately.
			//
			// Asserts that a batch is open.
			inline Energy Calculate(int bLeft, int bTop) const;

		private:
			friend EnergyCalculator;
			inline BatchImmediate(EnergyCalculator& energyCalculator);
			EnergyCalculator& m_energyCalculator;
		};

		// See comment above class for info on when to use this batch type.
		// Batch automatically closes when the BatchQueued instance goes
		// out of scope.
		class BatchQueued
		{
		public:
			// Closes the batch.
			inline ~BatchQueued();

			// For use with QueueCalculation and GetResult().
			typedef uint Handle;
			static const Handle INVALID_HANDLE = 0xFFFFFFFF;

			// Given the left-top coordinate of block B, this method queues an
			// energy calculation of block A against B, and returns a handle that
			// is used to later retrieve the result from GetResult().
			//
			// Handles are unique per BatchQueued instance, and are guaranteed
			// to increment from 0 to n. This guarantee may be used to simplify
			// queueing and results retrieval, since it means that if two
			// separate loops use the same localized batch, and one loop calls
			// QueueCalculation() and the other gets the results *in the same
			// logical order*, then the code can avoid having to store Handles,
			// and instead assume that the loops' i=0...n indices are identical
			// to the handles.
			//
			// I.e.,
			//
			//	{
			//		ScopedNodeEnergyBatch scopedNodeEnergyBatch(node, energyCalculator, width, height, aLeft, aTop);
			//		for (int i = 0; i < n; ++i)
			//		{
			//			const EnergyCalculator::BatchQueued::Handle handle = scopedNodeEnergyBatch.QueueCalculation(b[i].left, b[i].top);
			//			wxASSERT(i == handle); // i is identical to handle
			//		}
			//
			//		scopedNodeEnergyBatch.Sync();
			//
			//		for (int i = 0; i < n; ++i)
			//		{
			//			outABEnergy[i] = scopedNodeEnergyBatch.GetResult(i); // i is identical to handle
			//		}
			//	}
			//
			// Asserts that a batch is open.
			inline Handle QueueCalculation(int bLeft, int bTop);

			// Completes the processing of the queued calculations using all
			// available threads (workers & main). QueueCalculation() will
			// assert if called after a this method, and GetResult will assert
			// if called before this method.
			inline void ProcessCalculations();

			// Gets the energy result from a previous QueueCalculation() call.
			//
			// Asserts that a batch is open.
			inline Energy GetResult(Handle handle) const;

		private:
			friend EnergyCalculator;
			inline BatchQueued(EnergyCalculator& energyCalculator);
			EnergyCalculator& m_energyCalculator;
		};

		// Batch opening parameters
		struct BatchParams
		{
			inline BatchParams()
			{
			}

			inline BatchParams(int maxCalculations, int width, int height, int aLeft, int aTop, bool aMasked) :
			maxCalculations(maxCalculations),
			width(width),
			height(height),
			aLeft(aLeft),
			aTop(aTop),
			aMasked(aMasked)
			{
			}

			int maxCalculations;
			int width;
			int height;
			int aLeft;
			int aTop;
			bool aMasked;
		};

		//
		// BatchOpenImmediate
		// BatchOpenQueued
		//
		// Opens an energy calculation batch based on the give width and
		// height, and left-top coordinate of the block against which the rest
		// of the blocks are computed against (block A). If aMasked is true,
		// then only the pixels intersecting with the known regions of block A
		// are factored into each result.
		//
		// Only one batch may be open at a time. Asserts that no batch is open.
		virtual BatchImmediate BatchOpenImmediate(const BatchParams& params) = 0;
		virtual BatchQueued BatchOpenQueued(const BatchParams& params) = 0;

	protected:
		// Batch objects can call these internal methods.
		friend BatchImmediate;
		friend BatchQueued;

		// BatchImmediate and BatchQueued friend this base. Subclasses must
		// use these methods to create new batch instances. The temporary will
		// be optimized away by RVO.
		inline BatchImmediate GetBatchImmediate(EnergyCalculator& energyCalculator);
		inline BatchQueued GetBatchQueued(EnergyCalculator& energyCalculator);

		virtual void BatchClose() = 0;
		virtual Energy Calculate(int bLeft, int bTop) const = 0;
		virtual BatchQueued::Handle QueueCalculation(int bLeft, int bTop) = 0;
		virtual void ProcessCalculations() = 0;
		virtual Energy GetResult(BatchQueued::Handle handle) const = 0;
	};
}

//
// Include the inline implementations
//
#define INCLUDING_ENERGY_CALCULATOR_INL
#include "EnergyCalculator.inl"
#undef INCLUDING_ENERGY_CALCULATOR_INL

#endif
