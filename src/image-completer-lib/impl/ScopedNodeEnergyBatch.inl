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

//
// Contains inline implementations of ScopedNodeEnergyBatch.h definitions.
//
#ifndef SCOPED_NODE_ENERGY_BATCH_INL
#define SCOPED_NODE_ENERGY_BATCH_INL

#ifndef INCLUDING_SCOPED_NODE_ENERGY_BATCH_INL
#error "ScopedNodeEnergyBatch.inl must only be included by ScopedNodeEnergyBatch.h"
#endif

#include "Node.h"

namespace LfnIc
{
	//
	// ScopedNodeEnergyBatchImmediate implementation
	//
	inline ScopedNodeEnergyBatchImmediate::ScopedNodeEnergyBatchImmediate(
		const Node& node,
		EnergyCalculator& energyCalculator,
		const EnergyCalculator::BatchParams& params)
	{
		wxASSERT(params.aMasked);
		m_delegate = node.OverlapsKnownRegion()
			? static_cast<Delegate*>(new DelegateOverlap(energyCalculator, params))
			: new DelegateNoOverlap();
	}

	inline ScopedNodeEnergyBatchImmediate::~ScopedNodeEnergyBatchImmediate()
	{
		delete m_delegate;
	}

	inline Energy ScopedNodeEnergyBatchImmediate::Calculate(int bLeft, int bTop) const
	{
		return m_delegate->Calculate(bLeft, bTop);
	}

	inline ScopedNodeEnergyBatchImmediate::DelegateOverlap::DelegateOverlap(EnergyCalculator& energyCalculator, const EnergyCalculator::BatchParams& params) :
	m_batchImmediate(energyCalculator.BatchOpenImmediate(params))
	{
	}

	Energy ScopedNodeEnergyBatchImmediate::DelegateOverlap::Calculate(int bLeft, int bTop) const
	{
		return m_batchImmediate.Calculate(bLeft, bTop);
	}

	Energy ScopedNodeEnergyBatchImmediate::DelegateNoOverlap::Calculate(int bLeft, int bTop) const
	{
		return ENERGY_MIN;
	}

	//
	// ScopedNodeEnergyBatchQueued implementation
	//
	inline ScopedNodeEnergyBatchQueued::ScopedNodeEnergyBatchQueued(
		const Node& node,
		EnergyCalculator& energyCalculator,
		const EnergyCalculator::BatchParams& params)
	{
		wxASSERT(params.aMasked);
		m_delegate = node.OverlapsKnownRegion()
			? static_cast<Delegate*>(new DelegateOverlap(energyCalculator, params))
			: new DelegateNoOverlap();
	}

	inline ScopedNodeEnergyBatchQueued::~ScopedNodeEnergyBatchQueued()
	{
		delete m_delegate;
	}

	inline ScopedNodeEnergyBatchQueued::Handle ScopedNodeEnergyBatchQueued::QueueCalculation(int bLeft, int bTop)
	{
		return m_delegate->QueueCalculation(bLeft, bTop);
	}

	inline void ScopedNodeEnergyBatchQueued::ProcessCalculations()
	{
		return m_delegate->ProcessCalculations();
	}

	inline Energy ScopedNodeEnergyBatchQueued::GetResult(Handle handle) const
	{
		return m_delegate->GetResult(handle);
	}

	inline ScopedNodeEnergyBatchQueued::DelegateOverlap::DelegateOverlap(EnergyCalculator& energyCalculator, const EnergyCalculator::BatchParams& params) :
	m_batchQueued(energyCalculator.BatchOpenQueued(params))
	{
	}

	EnergyCalculator::BatchQueued::Handle ScopedNodeEnergyBatchQueued::DelegateOverlap::QueueCalculation(int bLeft, int bTop)
	{
		return m_batchQueued.QueueCalculation(bLeft, bTop);
	}

	void ScopedNodeEnergyBatchQueued::DelegateOverlap::ProcessCalculations()
	{
		return m_batchQueued.ProcessCalculations();
	}

	Energy ScopedNodeEnergyBatchQueued::DelegateOverlap::GetResult(Handle handle) const
	{
		return m_batchQueued.GetResult(handle);
	}

	EnergyCalculator::BatchQueued::Handle ScopedNodeEnergyBatchQueued::DelegateNoOverlap::QueueCalculation(int bLeft, int bTop)
	{
		return EnergyCalculator::BatchQueued::INVALID_HANDLE;
	}

	void ScopedNodeEnergyBatchQueued::DelegateNoOverlap::ProcessCalculations()
	{
	}

	Energy ScopedNodeEnergyBatchQueued::DelegateNoOverlap::GetResult(EnergyCalculator::BatchQueued::Handle handle) const
	{
		return ENERGY_MIN;
	}
}

#endif
