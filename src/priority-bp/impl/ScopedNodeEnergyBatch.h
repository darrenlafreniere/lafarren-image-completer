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

#ifndef SCOPED_NODE_ENERGY_BATCH_H
#define SCOPED_NODE_ENERGY_BATCH_H

#include "EnergyCalculator.h"

namespace PriorityBp
{
	// Forward declarations
	class Node;

	//
	// Based on EnergyCalculator::BatchImmediate, this class assumes that
	// block A is masked. If the node does not overlap the known region, this
	// class will skip performing the energy calculation and will return
	// ENERGY_MIN for all Calculate calls.
	//
	class ScopedNodeEnergyBatchImmediate
	{ 
	public:
		inline ScopedNodeEnergyBatchImmediate(
			const Node& node,
			EnergyCalculator& energyCalculator,
			const EnergyCalculator::BatchParams& params);

		inline ~ScopedNodeEnergyBatchImmediate();

		inline Energy Calculate(int bLeft, int bTop) const;

	private:
		class Delegate
		{
		public:
			virtual ~Delegate() {}
			virtual Energy Calculate(int bLeft, int bTop) const = 0;
		};

		class DelegateOverlap : public Delegate
		{
		public:
			inline DelegateOverlap(EnergyCalculator& energyCalculator, const EnergyCalculator::BatchParams& params);
			virtual Energy Calculate(int bLeft, int bTop) const;

		private:
			EnergyCalculator::BatchImmediate m_batchImmediate;
		};

		class DelegateNoOverlap : public Delegate
		{
		public:
			virtual Energy Calculate(int bLeft, int bTop) const;
		};

		Delegate* m_delegate;
	};

	//
	// Based on EnergyCalculator::BatchQueued, this class assumes that block A
	// is masked. If the node does not overlap the known region, this class
	// will skip performing the energy calculation and will return ENERGY_MIN
	// for all Calculate calls.
	//
	class ScopedNodeEnergyBatchQueued
	{ 
	public:
		typedef EnergyCalculator::BatchQueued::Handle Handle;

		inline ScopedNodeEnergyBatchQueued(
			const Node& node,
			EnergyCalculator& energyCalculator,
			const EnergyCalculator::BatchParams& params);

		inline ~ScopedNodeEnergyBatchQueued();

		inline Handle QueueCalculation(int bLeft, int bTop);
		inline void ProcessCalculations();
		inline Energy GetResult(Handle handle) const;

	private:
		class Delegate
		{
		public:
			virtual ~Delegate() {}
			virtual Handle QueueCalculation(int bLeft, int bTop) = 0;
			virtual void ProcessCalculations() = 0;
			virtual Energy GetResult(Handle handle) const = 0;
		};

		class DelegateOverlap : public Delegate
		{
		public:
			inline DelegateOverlap(EnergyCalculator& energyCalculator, const EnergyCalculator::BatchParams& params);
			virtual Handle QueueCalculation(int bLeft, int bTop);
			virtual void ProcessCalculations();
			virtual Energy GetResult(Handle handle) const;

		private:
			EnergyCalculator::BatchQueued m_batchQueued;
		};

		class DelegateNoOverlap : public Delegate
		{
		public:
			virtual Handle QueueCalculation(int bLeft, int bTop);
			virtual void ProcessCalculations();
			virtual Energy GetResult(Handle handle) const;
		};

		Delegate* m_delegate;
	};
}

//
// Include the inline implementations
//
#define INCLUDING_SCOPED_NODE_ENERGY_BATCH_INL
#include "ScopedNodeEnergyBatch.inl"
#undef INCLUDING_SCOPED_NODE_ENERGY_BATCH_INL

#endif
