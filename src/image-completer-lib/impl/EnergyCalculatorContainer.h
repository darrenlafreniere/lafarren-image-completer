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

#ifndef ENERGY_CALCULATOR_CONTAINER_H
#define ENERGY_CALCULATOR_CONTAINER_H

#include "energy-calculators/EnergyCalculatorPerPixel.h"

#include "energy-calculators/EnergyCalculatorFftConfig.h"
#if ENABLE_ENERGY_CALCULATOR_FFT
#include "energy-calculators/EnergyCalculatorFft.h"
#endif

namespace LfnIc
{
	class ImageConst;
	class MaskLod;
	struct Settings;

    /// This class holds an input image, a mask, and the completion settings.
	class EnergyCalculatorContainer
	{
	public:
		EnergyCalculatorContainer(const Settings& settings, const ImageConst& inputImage, const MaskLod& mask);

		/// TODO: temp method, expand into an EnergyManager class.
		/// Returns a reference to an energy calculator that's suitable for
		/// the anticipated number of batch calculations.
		inline EnergyCalculator& Get(int numBatchCalculations)
		{
#if ENABLE_ENERGY_CALCULATOR_FFT
			if (numBatchCalculations < 50) // currently a guesstimate
			{
				return energyCalculatorPerPixel;
			}
			else
			{
				return energyCalculatorFft;
			}
#else
			return energyCalculatorPerPixel;
#endif
		}

	private:
		EnergyCalculatorPerPixel energyCalculatorPerPixel;
#if ENABLE_ENERGY_CALCULATOR_FFT
		EnergyCalculatorFft energyCalculatorFft;
#endif
	};
};

#endif
