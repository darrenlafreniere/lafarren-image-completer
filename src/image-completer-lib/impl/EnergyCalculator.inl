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
// Contains inline implementations of EnergyCalculator.h definitions.
//
#ifndef ENERGY_CALCULATOR_INL
#define ENERGY_CALCULATOR_INL

#ifndef INCLUDING_ENERGY_CALCULATOR_INL
#error "EnergyCalculator.inl must only be included by EnergyCalculator.h"
#endif

namespace LfnIc
{
	//
	// EnergyCalculator
	//
	inline EnergyCalculator::BatchImmediate::BatchImmediate(EnergyCalculator& energyCalculator) :
	m_energyCalculator(energyCalculator)
	{
	}

	inline EnergyCalculator::BatchImmediate::~BatchImmediate()
	{
		m_energyCalculator.BatchClose();
	}
	
	inline Energy EnergyCalculator::BatchImmediate::Calculate(int bLeft, int bTop) const
	{
		return m_energyCalculator.Calculate(bLeft, bTop);
	}

	inline EnergyCalculator::BatchQueued::BatchQueued(EnergyCalculator& energyCalculator) :
	m_energyCalculator(energyCalculator)
	{
	}

	inline EnergyCalculator::BatchQueued::~BatchQueued()
	{
		m_energyCalculator.BatchClose();
	}

	inline EnergyCalculator::BatchQueued::Handle EnergyCalculator::BatchQueued::QueueCalculation(int bLeft, int bTop)
	{
		return m_energyCalculator.QueueCalculation(bLeft, bTop);
	}

	inline void EnergyCalculator::BatchQueued::ProcessCalculations()
	{
		return m_energyCalculator.ProcessCalculations();
	}

	inline Energy EnergyCalculator::BatchQueued::GetResult(Handle handle) const
	{
		return m_energyCalculator.GetResult(handle);
	}

	inline EnergyCalculator::BatchImmediate EnergyCalculator::GetBatchImmediate(EnergyCalculator& energyCalculator)
	{
		return BatchImmediate(energyCalculator);
	}

	inline EnergyCalculator::BatchQueued EnergyCalculator::GetBatchQueued(EnergyCalculator& energyCalculator)
	{
		return BatchQueued(energyCalculator);
	}
}

#endif
