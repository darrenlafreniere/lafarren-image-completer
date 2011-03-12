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
// Contains inline implementations of EnergyCalculatorUtils.h definitions.
//
#ifndef ENERGY_CALCULATOR_UTILS_INL
#define ENERGY_CALCULATOR_UTILS_INL

#ifndef INCLUDING_ENERGY_CALCULATOR_UTILS_INL
#error "EnergyCalculatorUtils.inl must only be included by EnergyCalculatorUtils.h"
#endif

namespace LfnIc
{
	inline void EnergyCalculatorUtils::ClampToMinBoundary(int& a, int& size, int boundary)
	{
		if (a < boundary)
		{
			const int adjustment = boundary - a;
			a += adjustment;
			size -= adjustment;
		}
	}

	inline void EnergyCalculatorUtils::ClampToMaxBoundary(int a, int& size, int boundary)
	{
		if ((a + size) > boundary)
		{
			size = boundary - a;
		}
	}

	inline void EnergyCalculatorUtils::ClampToMinBoundary(int& a, int& b, int& size, int boundary)
	{
		const int m = std::min(a, b);
		if (m < boundary)
		{
			const int adjustment = boundary - m;
			a += adjustment;
			b += adjustment;
			size -= adjustment;
		}
	}

	inline void EnergyCalculatorUtils::ClampToMaxBoundary(int a, int b, int& size, int boundary)
	{
		const int m = std::max(a, b);
		if ((m + size) > boundary)
		{
			size = boundary - m;
		}
	}
}

#endif
