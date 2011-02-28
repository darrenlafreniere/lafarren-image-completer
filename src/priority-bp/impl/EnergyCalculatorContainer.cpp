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
#include "EnergyCalculatorContainer.h"

#include "tech/DbgMem.h"

//
// EnergyCalculatorContainer implementation
//
EnergyCalculatorContainer::EnergyCalculatorContainer(const Settings& settings, const Image& inputImage, const MaskLod& mask) :
energyCalculatorPerPixel(inputImage, mask)
#if ENABLE_ENERGY_CALCULATOR_FFT
,
energyCalculatorFft(settings, inputImage, mask
#if FFT_VALIDATION_ENABLED
	, energyCalculatorPerPixel
#endif
	)
#endif
{
}
