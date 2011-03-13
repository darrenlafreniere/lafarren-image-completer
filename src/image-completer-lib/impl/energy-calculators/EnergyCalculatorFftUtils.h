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

#ifndef ENERGY_CALCULATOR_FFT_UTILS_H
#define ENERGY_CALCULATOR_FFT_UTILS_H

#include "EnergyCalculatorFftConfig.h"
#if ENABLE_ENERGY_CALCULATOR_FFT

#include "LfnIcTypes.h"

namespace LfnIc
{
	// Forward declarations
	class ImageConst;
	class MaskLod;

	//
	// Provides base utilies that any fft energy calculator implementation could use.
	//
	class EnergyCalculatorFftUtils
	{
	public:
#if FFT_VALIDATION_ENABLED
		// Brute force energy evaluations for validating the fft computations.
		// Should not be used for anything other than testing. There are two
		// equations that the fft calculators crunch, and they are as follows:
		//
		// A is masked:
		//
		//     Ma*(Ia - Ib)^2
		//   = ((Ma*Ia)^2) (-2*(Ma*Ia*Ib)) + ((Ma*Ib)^2)
		//		
		// Unmasked:
		//
		//     (Ia - Ib)^2
		//   = (Ia^2) (-2*(Ia*Ib)) + (Ib^2)
		//
		// The brute force methods calculate one of the 3 terms in either
		// the expanded masked equation or the expanded unmasked equation.
		//
		static Energy BruteForceCalculate1stTerm(const ImageConst& image, int width, int height, int aLeft, int aTop, const MaskLod* aMask);
		static Energy BruteForceCalculate2ndTerm(const ImageConst& image, int width, int height, int aLeft, int aTop, const MaskLod* aMask, int bLeft, int bTop);
		static Energy BruteForceCalculate3rdTerm(const ImageConst& image, int width, int height, int aLeft, int aTop, const MaskLod* aMask, int bLeft, int bTop);
#endif // FFT_VALIDATION_ENABLED
	};
}

#endif // ENABLE_ENERGY_CALCULATOR_FFT
#endif // ENERGY_CALCULATOR_FFT_UTILS_H
