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

#ifndef ENERGY_CALCULATOR_SEPARATE_FFT_H
#define ENERGY_CALCULATOR_SEPARATE_FFT_H

#include "EnergyCalculatorFftConfig.h"
#if ENABLE_ENERGY_CALCULATOR_FFT

#include "EnergyCalculator.h"
#include "EnergyCalculatorFftUtils.h"
#include "EnergyWsst.h"
#include "fftw3.h"

#define ENERGY_FFT_SINGLE_PRECISION 1

namespace PriorityBp
{
	// Forward declarations
#if FFT_VALIDATION_ENABLED
	class EnergyCalculatorPerPixel;
#endif
	class Image;
	class MaskLod;
	struct Settings;

	// Calculates the energy between two regions of the input image by doing
	// straight per-pixel calculations on the CPU.
	//
	// Internally, the RGB color channel data is stored in a separate buffer
	// per channel.
	class EnergyCalculatorFft : public EnergyCalculator
	{
	public:
		//
		// Definitions
		//

		// FftComplex component index constants:
		static const int REAL = 0;
		static const int IMAG = 1;

		// Types
#if ENERGY_FFT_SINGLE_PRECISION
		typedef float FftReal;
		typedef fftwf_complex FftComplex;
		typedef fftwf_plan FftPlan;
#else
		typedef double FftReal;
		typedef fftw_complex FftComplex;
		typedef fftw_plan FftPlan;
#endif

		//
		// Methods
		//
		EnergyCalculatorFft(
			const Settings& settings,
			const Image& inputImage,
			const MaskLod& mask
#if FFT_VALIDATION_ENABLED
			, EnergyCalculatorPerPixel m_energyCalculatorPerPixel
#endif
			);
		~EnergyCalculatorFft();

		// EnergyCalculator interface
		virtual BatchImmediate BatchOpenImmediate(const BatchParams& params);
		virtual BatchQueued BatchOpenQueued(const BatchParams& params);

	protected:
		// Common batch opening method.
		EnergyCalculatorFft& BatchOpen(const BatchParams& params);

		virtual void BatchClose();
		virtual Energy Calculate(int bLeft, int bTop) const;
		virtual BatchQueued::Handle QueueCalculation(int bLeft, int bTop);
		virtual void ProcessCalculations();
		virtual Energy GetResult(BatchQueued::Handle handle) const;

	private:
		//
		// Internal definitions
		//

		static const int RGB_CHANNELS_NUM = 3;

		union FftwInPlaceBuffer
		{
			void* generic;
			FftReal* real;
			FftComplex* complex;
		};
	
		//
		// Internal methods
		//

		// Allocates and returns an appropriately sized fftw in-place buffer
		FftwInPlaceBuffer FftwInPlaceBufferAlloc() const;

		// Given a FftwInPlaceBuffer pointer type and a y coordinate, this
		// returns a pointer to the data for that row.
		template<typename T>
		T* GetRow(T* real, int y) const;

		// Policy-driven templates for generic fills and reverse fills. Policy
		// class must define:
		//
		//		inline FftReal GetReal(int rowMajorIndex) const;
		//
		template<typename POLICY>
		void FillRealBuffer(const POLICY& policy, FftReal* real, int left, int top, int width, int height) const;

		template<typename POLICY>
		void ReverseFillRealBuffer(const POLICY& policy, FftReal* real, int left, int top, int width, int height) const;

		// Zero-fills the real buffer according to the specified padding.
		void PadRealBuffer(FftReal* real, int leftPad, int topPad, int rightPad, int bottomPad) const;

		// a *= b
		void MultiplyEquals(FftComplex* aComplex, const FftComplex* bComplex) const;
		void MultiplyEquals(FftReal* aReal, FftReal bScalar) const;

		// Applies m_fftPlanBuffer.real to m_batchEnergy2ndAnd3rdTerm using
		// the energy operator policy class. The policy class must define:
		//
		//		inline void Execute(Energy& a, const Energy& b) const;
		//
		// Made this a template to avoid putting conditional behavior in its
		// inner loop.
		template<typename ENERGY_OPERATOR>
		void ApplyFftRealTo2ndAnd3rdTerm(const ENERGY_OPERATOR& energyOperator);

		//
		// Data
		//
		// NOTE: the order of these members is critical due to constructor
		// list dependencies.
		//
		const Settings& m_settings;
		const Image& m_inputImage;
		const MaskLod& m_mask;
#if FFT_VALIDATION_ENABLED
		EnergyCalculatorPerPixel& m_energyCalculatorPerPixel;
#endif

		// Cache some frequently used constants
		const int m_inputWidth;
		const int m_inputHeight;
		const int m_fftWidth;
		const int m_fftHeight;
		const int m_fftInPlaceBufferStride;
		const int m_fftInPlaceBufferNumBytes;

		const EnergyWsst m_wsst;
		const EnergyWsst m_wsstMasked;

		FftwInPlaceBuffer m_fftPlanBuffer;
		FftPlan m_fftPlanRealToComplex;
		FftPlan m_fftPlanComplexToReal;

		// These two buffers are initialized at construction to store the
		// complex output of the fft of the image, and the image squared.
		FftwInPlaceBuffer m_fftComplexImage[RGB_CHANNELS_NUM];
		FftwInPlaceBuffer m_fftComplexImageSquared[RGB_CHANNELS_NUM];

		// Terms calculated by EnergyCalculatorFft::BatchOpen()
		// m_batchEnergy2ndAnd3rdTerm is a row-major buffer of the image
		// dimensions.
		Energy m_batchEnergy1stTerm;
		Energy* m_batchEnergy2ndAnd3rdTerm;

		bool m_isBatchOpen;
		bool m_isBatchProcessed;
		BatchParams m_batchParams;
		std::vector<Energy> m_queuedEnergyResults;
	};
}

#endif // ENABLE_ENERGY_CALCULATOR_FFT
#endif // ENERGY_CALCULATOR_SEPARATE_FFT_H
