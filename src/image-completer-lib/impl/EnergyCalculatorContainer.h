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

#include "energy-calculators/EnergyCalculatorFftConfig.h"
#if ENABLE_ENERGY_CALCULATOR_FFT
#include "energy-calculators/EnergyCalculatorFft.h"
#endif
#include "energy-calculators/EnergyCalculatorPerPixel.h"
#include "Scalable.h"

namespace LfnIc
{
	class EnergyCalculatorMeasurer;
	class ImageConst;
	class MaskLod;
	struct Settings;

	class EnergyCalculatorContainer : public Scalable
	{
	public:
		EnergyCalculatorContainer(const Settings& settings, const ImageConst& inputImage, const MaskLod& mask);
		~EnergyCalculatorContainer();

		virtual void ScaleUp();
		virtual void ScaleDown();
		virtual int GetScaleDepth() const;

		// Returns a reference to an energy calculator that's suitable for
		// the batch parameters and the number of calculations in the
		// batch.
		EnergyCalculator& Get(const EnergyCalculator::BatchParams& batchParams, int numBatchCalculations);

	private:
		const Settings& m_settings;
		const ImageConst& m_inputImage;
		const MaskLod& m_mask;

		EnergyCalculatorPerPixel m_energyCalculatorPerPixel;
#if ENABLE_ENERGY_CALCULATOR_FFT
		friend class EnergyCalculatorMeasurer;
		void OnFoundFasterEnergyCalculator(const EnergyCalculatorMeasurer& measurer);
		void ClearMeasurers();

		// EnergyCalculatorFft instances are non-scalable. Therefore, they're
		// dynamically allocated and initialized for each resolution.
		class Resolution
		{
		public:
			Resolution(const EnergyCalculatorContainer& energyCalculatorContainer);
			~Resolution();

			EnergyCalculatorFft& GetEnergyCalculatorFft();
		private:
			const EnergyCalculatorContainer& m_energyCalculatorContainer;
			EnergyCalculatorFft* m_energyCalculatorFft;
		};

		friend class Resolution;
		inline Resolution& GetCurrentResolution() const { return *m_resolutions[m_depth]; }
		std::vector<Resolution*> m_resolutions;

		std::vector<EnergyCalculatorMeasurer*> m_measurers;
#endif // ENABLE_ENERGY_CALCULATOR_FFT
		int m_depth;
	};
};

#endif
