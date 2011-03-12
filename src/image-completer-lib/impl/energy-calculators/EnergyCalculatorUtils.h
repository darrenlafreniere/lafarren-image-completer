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

#ifndef ENERGY_CALCULATOR_UTILS_H
#define ENERGY_CALCULATOR_UTILS_H

namespace PriorityBp
{
	//
	// Provides base utilies that any energy calculator implementation could use.
	//
	class EnergyCalculatorUtils
	{
	public:
		// Ensures that coordinate a is >= the boundary. If not, then a and
		// size are adjusted accordingly.
		static inline void ClampToMinBoundary(int& a, int& size, int boundary);

		// Ensures that coordinate a plus the size are <= the boundary. If
		// not, then size is adjusted accordingly.
		static inline void ClampToMaxBoundary(int a, int& size, int boundary);

		// Ensures that BOTH coordinates a and b are >= the boundary. If not,
		// then a, b, and size are adjusted accordingly.
		static inline void ClampToMinBoundary(int& a, int& b, int& size, int boundary);

		// Ensures that BOTH coordinates a and b plus the size are <= the
		// boundary. If not, then size is adjusted accordingly.
		static inline void ClampToMaxBoundary(int a, int b, int& size, int boundary);
	};
}

//
// Include the inline implementations
//
#define INCLUDING_ENERGY_CALCULATOR_UTILS_INL
#include "EnergyCalculatorUtils.inl"
#undef INCLUDING_ENERGY_CALCULATOR_UTILS_INL

#endif
