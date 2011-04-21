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
#include "EnergyCalculatorFft.h"

#if ENABLE_ENERGY_CALCULATOR_FFT

#include "tech/MathUtils.h"

#if FFT_VALIDATION_ENABLED
#include "EnergyCalculatorPerPixel.h"
#endif
#include "EnergyCalculatorUtils.h"
#include "ImageConst.h"
#include "LfnIcSettings.h"
#include "MaskLod.h"

#include "tech/DbgMem.h"

// Debugging flag. Verifies that fft buffer pointers are valid.
#define FFT_ASSERT_BOUNDS_ENABLED 0

#if FFT_ASSERT_BOUNDS_ENABLED
// Verify that __p__ is within __base__'s bounds.
// Assumes m_fftInPlaceBufferNumBytes exists.
#define FFT_ASSERT_BOUNDS(__base__, __p__) \
	wxASSERT_MSG(int(__p__) >= int(__base__), "FFT_ASSERT_BOUNDS failed, pointer underruns buffer."); \
	wxASSERT_MSG((int(__p__) - int(__base__)) < m_fftInPlaceBufferNumBytes, "FFT_ASSERT_BOUNDS failed, pointer overruns buffer.")

// Verify that __rangeBytes__ from __p__ is within __base__'s bounds.
// Assumes m_fftInPlaceBufferNumBytes exists.
#define FFT_ASSERT_BOUNDS_RANGE(__base__, __p__, __rangeBytes__) \
	wxASSERT_MSG(int(__p__) >= int(__base__), "FFT_ASSERT_BOUNDS_RANGE failed, pointer range underruns buffer."); \
	wxASSERT_MSG((int(__p__) + __rangeBytes__ - int(__base__)) <= m_fftInPlaceBufferNumBytes, "FFT_ASSERT_BOUNDS_RANGE failed, pointer range overruns buffer")
#else
// no-ops
#define FFT_ASSERT_BOUNDS(real, out)
#define FFT_ASSERT_BOUNDS_RANGE(__base__, __p__, __rangeBytes__)
#endif

typedef LfnIc::EnergyCalculatorFft::FftReal FftReal;
typedef LfnIc::EnergyCalculatorFft::FftComplex FftComplex;

#if ENERGY_FFT_SINGLE_PRECISION
#define FFTW_PREFIX(name) FFTW_MANGLE_FLOAT(name)
#else
#define FFTW_PREFIX(name) FFTW_MANGLE_DOUBLE(name)
#endif

//
// Policy classes for use with the FillFfti and ReverseFillFfti template functions.
//
namespace LfnIc
{
	static inline FftReal MaskValueToFftReal(Mask::Value maskValue)
	{
		return FftReal((maskValue == Mask::KNOWN) ? 1 : 0);
	}

	class FillPolicyChannel
	{
	public:
		FillPolicyChannel(const ImageConst& inputImage, int channel) :
		  m_imagePixel(inputImage.GetData()),
			  m_channel(channel)
		  {
		  }

		  // rowMajorIndex length is based on EnergyCalculatorFft::m_inputWidth
		  inline FftReal GetReal(int rowMajorIndex) const
		  {
			  return m_imagePixel[rowMajorIndex].channel[m_channel];
		  }

	protected:
		const Image::Pixel* m_imagePixel;
		const int m_channel;
	};

	class FillPolicyChannelScaled : public FillPolicyChannel
	{
	public:
		typedef FillPolicyChannel Super;

		FillPolicyChannelScaled(const ImageConst& inputImage, int channel, FftReal scalar) :
		Super(inputImage, channel),
			m_scalar(scalar)
		{
		}

		// rowMajorIndex length is based on EnergyCalculatorFft::m_inputWidth
		inline FftReal GetReal(int rowMajorIndex) const
		{
			return m_scalar * Super::GetReal(rowMajorIndex);
		}

	protected:
		const FftReal m_scalar;
	};

	class FillPolicyChannelMaskedScaled : public FillPolicyChannelScaled
	{
	public:
		typedef FillPolicyChannelScaled Super;

		FillPolicyChannelMaskedScaled(const ImageConst& inputImage, const MaskLod& mask, int channel, FftReal scalar) :
		Super(inputImage, channel, scalar),
			m_maskBuffer(mask.GetLodBuffer(mask.GetHighestLod()))
		{
		}

		// rowMajorIndex length is based on EnergyCalculatorFft::m_inputWidth
		inline FftReal GetReal(int rowMajorIndex) const
		{
			return MaskValueToFftReal(m_maskBuffer[rowMajorIndex]) * Super::GetReal(rowMajorIndex);
		}

	protected:
		const Mask::Value* m_maskBuffer;
	};

	class FillPolicyMask
	{
	public:
		FillPolicyMask(const MaskLod& mask) :
		  m_maskBuffer(mask.GetLodBuffer(mask.GetHighestLod()))
		  {
		  }

		  // rowMajorIndex length is based on EnergyCalculatorFft::m_inputWidth
		  inline FftReal GetReal(int rowMajorIndex) const
		  {
			  return MaskValueToFftReal(m_maskBuffer[rowMajorIndex]);
		  }

	protected:
		const Mask::Value* m_maskBuffer;
	};
}

//
// Energy operator for use with the ApplyFftRealTo2ndAnd3rdTerm template function.
//
namespace LfnIc
{
	class EnergyOperatorAssignNegative
	{
	public:
		void Execute(Energy& a, const Energy& b) const
		{
			a = -b;
		}
	};

	class EnergyOperatorDecrement
	{
	public:
		void Execute(Energy& a, const Energy& b) const
		{
			a -= b;
		}
	};

	class EnergyOperatorIncrement
	{
	public:
		void Execute(Energy& a, const Energy& b) const
		{
			a += b;
		}
	};
}

//
// EnergyCalculatorFft implementation
//
LfnIc::EnergyCalculatorFft::EnergyCalculatorFft(
	const Settings& settings,
	const ImageConst& inputImage,
	const MaskLod& mask
#if FFT_VALIDATION_ENABLED
	, EnergyCalculatorPerPixel energyCalculatorPerPixel
#endif
	) :
m_settings(settings),
	m_inputImage(inputImage),
	m_mask(mask),
#if FFT_VALIDATION_ENABLED
	m_energyCalculatorPerPixel(energyCalculatorPerPixel),
#endif
	m_inputWidth(m_inputImage.GetWidth()),
	m_inputHeight(m_inputImage.GetHeight()),
	m_fftWidth(m_inputWidth + m_settings.patchWidth - 1),
	m_fftHeight(m_inputHeight + m_settings.patchHeight - 1),
	// http://www.fftw.org/fftw3_doc/Multi_002dDimensional-DFTs-of-Real-Data.html#Multi_002dDimensional-DFTs-of-Real-Data
	m_fftInPlaceBufferStride(sizeof(FftReal) * 2 * (m_fftWidth / 2 + 1)),
	m_fftInPlaceBufferNumBytes(m_fftInPlaceBufferStride * m_fftHeight),
	m_wsst(inputImage, settings.latticeGapX, settings.latticeGapY),
	m_wsstMasked(inputImage, mask, settings.latticeGapX, settings.latticeGapY),
	m_batchEnergy1stTerm(ENERGY_MIN),
	m_batchEnergy2ndAnd3rdTerm(new Energy[m_inputWidth * m_inputHeight]),
	m_isBatchOpen(false),
	m_isBatchProcessed(false)
{
#ifdef USE_THREADS
	const int fftwInitThreadsResult = FFTW_PREFIX(init_threads)();
	wxASSERT(fftwInitThreadsResult != 0);

	const int cpuCount = wxThread::GetCPUCount();
	wxASSERT(cpuCount >= 1);
	FFTW_PREFIX(plan_with_nthreads)(cpuCount);
#endif

	m_fftPlanBuffer = FftwInPlaceBufferAlloc();

	// Dimensions must be in row-major order, so swap width and height
	// http://www.fftw.org/fftw3_doc/Multi_002dDimensional-DFTs-of-Real-Data.html#Multi_002dDimensional-DFTs-of-Real-Data
	m_fftPlanRealToComplex = FFTW_PREFIX(plan_dft_r2c_2d)(m_fftHeight, m_fftWidth, m_fftPlanBuffer.real, m_fftPlanBuffer.complex, FFTW_MEASURE);
	m_fftPlanComplexToReal = FFTW_PREFIX(plan_dft_c2r_2d)(m_fftHeight, m_fftWidth, m_fftPlanBuffer.complex, m_fftPlanBuffer.real, FFTW_MEASURE);

	// For each channel of m_fftImage and m_fftImageSquared, fill the real data,
	// execute the real-to-complex plan, and copy the results into the channel
	// buffer.
	for (int channel = 0; channel < CHANNELS_NUM; ++channel)
	{
		{
			m_fftComplexImage[channel] = FftwInPlaceBufferAlloc();

			FillPolicyChannel fillPolicy(m_inputImage, channel);
			FillRealBuffer(fillPolicy, m_fftPlanBuffer.real, 0, 0, m_inputWidth, m_inputHeight);

			FFTW_PREFIX(execute)(m_fftPlanRealToComplex);
			memcpy(m_fftComplexImage[channel].generic, m_fftPlanBuffer.generic, m_fftInPlaceBufferNumBytes);
		}

		{
			m_fftComplexImageSquared[channel] = FftwInPlaceBufferAlloc();

			FillPolicyChannel fillPolicy(m_inputImage, channel);
			FillRealBuffer(fillPolicy, m_fftPlanBuffer.real, 0, 0, m_inputWidth, m_inputHeight);

			for (int y = 0; y < m_inputHeight; ++y)
			{
				FftReal* real = GetRow(m_fftPlanBuffer.real, y);
				for (int x = 0; x < m_inputWidth; ++x, ++real)
				{
					FFT_ASSERT_BOUNDS(m_fftPlanBuffer.real, real);
					const FftReal value = *real;
					*real = value * value;
				}
			}

			FFTW_PREFIX(execute)(m_fftPlanRealToComplex);
			memcpy(m_fftComplexImageSquared[channel].generic, m_fftPlanBuffer.generic, m_fftInPlaceBufferNumBytes);
		}
	}
}

LfnIc::EnergyCalculatorFft::~EnergyCalculatorFft()
{
	for (int channel = 0; channel < CHANNELS_NUM; ++channel)
	{
		FFTW_PREFIX(free)(m_fftComplexImage[channel].generic);
		FFTW_PREFIX(free)(m_fftComplexImageSquared[channel].generic);
	}

	FFTW_PREFIX(destroy_plan)(m_fftPlanComplexToReal);
	FFTW_PREFIX(destroy_plan)(m_fftPlanRealToComplex);
	FFTW_PREFIX(free)(m_fftPlanBuffer.generic);

	delete [] m_batchEnergy2ndAnd3rdTerm;

#ifdef USE_THREADS
	FFTW_PREFIX(cleanup_threads)();
#endif
}

void LfnIc::EnergyCalculatorFft::BatchOpen(const BatchParams& params)
{
	wxASSERT(!m_isBatchOpen);
	wxASSERT(!m_isBatchProcessed);

	m_batchParams = params;
	m_isBatchOpen = true;

	// If aMasked is true:
	//     Ma*(Ia - Ib)^2
	//   = (Ma*Ia)^2 - 2*(Ma*Ia*Ib) + (Ma*Ib)^2
	//
	// If aMasked is false:
	//     (Ia - Ib)^2
	//   = Ia^2 - 2*(Ia*Ib) + Ib^2
	//
	// Terms with only Ia can be precalculated. Terms with Ib can be calculated
	// via a fast fourier transform.

	// Store first term in m_batchEnergy1stTerm:
	{
		const EnergyWsst& wsst = m_batchParams.aMasked ? m_wsstMasked : m_wsst;
		m_batchEnergy1stTerm = wsst.Calculate(m_batchParams.aLeft, m_batchParams.aTop, m_batchParams.width, m_batchParams.height);
	}

	// Calculate second term into m_batchEnergy2ndAnd3rdTerm
	{
		for (int channel = 0; channel < CHANNELS_NUM; ++channel)
		{
			// Calculate fft(-2 * <Ma?> * Ia) into m_fftPlanBuffer.
			{
				if (m_batchParams.aMasked)
				{
					FillPolicyChannelMaskedScaled fillPolicy(m_inputImage, m_mask, channel, FftReal(2));
					ReverseFillRealBuffer(fillPolicy, m_fftPlanBuffer.real, m_batchParams.aLeft, m_batchParams.aTop, m_batchParams.width, m_batchParams.height);
				}
				else
				{
					FillPolicyChannelScaled fillPolicy(m_inputImage, channel, FftReal(2));
					ReverseFillRealBuffer(fillPolicy, m_fftPlanBuffer.real, m_batchParams.aLeft, m_batchParams.aTop, m_batchParams.width, m_batchParams.height);
				}

				FFTW_PREFIX(execute)(m_fftPlanRealToComplex);
			}

			// fft(Ib) is already in m_fftComplexImage. Multiply
			// m_fftComplexImage into m_fftPlanBuffer.
			MultiplyEquals(m_fftPlanBuffer.complex, m_fftComplexImage[channel].complex);

			// Inverse transform m_fftPlanBuffer, then normalize it
			FFTW_PREFIX(execute)(m_fftPlanComplexToReal);
			MultiplyEquals(m_fftPlanBuffer.real, FftReal(1) / FftReal(m_fftWidth * m_fftHeight));

			// Apply the results from m_fftPlanBuffer into
			// m_batchEnergy2ndAnd3rdTerm
			if (channel == 0)
			{
				EnergyOperatorAssignNegative energyOperator;
				ApplyFftRealTo2ndAnd3rdTerm(energyOperator);
			}
			else
			{
				EnergyOperatorDecrement energyOperator;
				ApplyFftRealTo2ndAnd3rdTerm(energyOperator);
			}
		}

#if FFT_VALIDATION_ENABLED
		for (int y = 0; y < m_inputHeight; ++y)
		{
			Energy* batchEnergy2ndAnd3rdTermCurrent = m_batchEnergy2ndAnd3rdTerm + LfnTech::GetRowMajorIndex(m_inputWidth, 0, y);
			for (int x = 0; x < m_inputWidth; ++x, ++batchEnergy2ndAnd3rdTermCurrent)
			{
				const Energy e = *batchEnergy2ndAnd3rdTermCurrent;
				const Energy eBruteForce = EnergyCalculatorFftUtils::BruteForceCalculate2ndTerm(
					m_inputImage,
					m_inputWidth,
					m_inputHeight,
					m_batchParams.aLeft,
					m_batchParams.aTop,
					m_batchParams.aMasked ? &m_mask : NULL,
					x,
					y);
				wxASSERT(abs(e - eBruteForce) < 10);
			}
		}
#endif
	}

	// If a is masked, calculate and add third term into m_batchEnergy2ndAnd3rdTerm
	// here. Otherwise, Calculate will look up the third term for b from m_wsst.
	if (m_batchParams.aMasked)
	{
		for (int channel = 0; channel < CHANNELS_NUM; ++channel)
		{
			// Calculate fft(<Ma>) into m_fftPlanBuffer.
			{
				FillPolicyMask fillPolicy(m_mask);
				ReverseFillRealBuffer(fillPolicy, m_fftPlanBuffer.real, m_batchParams.aLeft, m_batchParams.aTop, m_batchParams.width, m_batchParams.height);
				FFTW_PREFIX(execute)(m_fftPlanRealToComplex);
			}

			// fft(Ib^2) is already in m_fftComplexImageSquared. Multiply
			// m_fftComplexImageSquared into m_fftPlanBuffer.
			MultiplyEquals(m_fftPlanBuffer.complex, m_fftComplexImageSquared[channel].complex);

			// Inverse transform m_fftPlanBuffer, then normalize it
			FFTW_PREFIX(execute)(m_fftPlanComplexToReal);
			MultiplyEquals(m_fftPlanBuffer.real, FftReal(1) / FftReal(m_fftWidth * m_fftHeight));

			// Add the results from m_fftPlanBuffer into m_batchEnergy2ndAnd3rdTerm
			EnergyOperatorIncrement energyOperator;
			ApplyFftRealTo2ndAnd3rdTerm(energyOperator);
		}
	}
}

void LfnIc::EnergyCalculatorFft::BatchOpenImmediate(const BatchParams& params)
{
	BatchOpen(params);
}

void LfnIc::EnergyCalculatorFft::BatchOpenQueued(const BatchParams& params)
{
	BatchOpen(params);
}

void LfnIc::EnergyCalculatorFft::BatchClose()
{
	wxASSERT(m_isBatchOpen);
	m_isBatchOpen = false;
	m_isBatchProcessed = false;

#ifdef _DEBUG
	const int queuedEnergyResultsPreClearCapacity = m_queuedEnergyResults.capacity();
#endif
	m_queuedEnergyResults.clear();

	wxASSERT_MSG(
		m_queuedEnergyResults.capacity() == static_cast<unsigned int>(queuedEnergyResultsPreClearCapacity),
		"m_queuedEnergyResults is unexpectedly deallocating! Could have performance impacts.");

}

LfnIc::Energy LfnIc::EnergyCalculatorFft::Calculate(int bLeft, int bTop) const
{
	wxASSERT(m_isBatchOpen);
	wxASSERT(bLeft >= 0);
	wxASSERT(bTop >= 0);
	wxASSERT(bLeft + m_batchParams.width <= m_inputImage.GetWidth());
	wxASSERT(bTop + m_batchParams.height <= m_inputImage.GetHeight());

	// First term, calculated in BatchOpen:
	Energy e = m_batchEnergy1stTerm;

	// Second term, and possibly third term if aMasked was true, calculated
	// in BatchOpen:
	e += m_batchEnergy2ndAnd3rdTerm[LfnTech::GetRowMajorIndex(m_inputImage.GetWidth(), bLeft, bTop)];

	// Third term if aMasked was false:
	if (!m_batchParams.aMasked)
	{
		e += m_wsst.Calculate(bLeft, bTop, m_batchParams.width, m_batchParams.height);
	}

#if FFT_VALIDATION_ENABLED
	EnergyCalculator::BatchImmediate energyBatch(m_energyCalculatorPerPixel.BatchOpenImmediate(m_batchParams));
	const Energy epp = energyBatch.Calculate(bLeft, bTop);
	wxASSERT(abs(epp - e) < 10);
#endif

	// e might be slightly negative due to accumulated floating point error in the fft calculations:
	return std::max(e, ENERGY_MIN);
}

LfnIc::EnergyCalculator::BatchQueued::Handle LfnIc::EnergyCalculatorFft::QueueCalculation(int bLeft, int bTop)
{
	wxASSERT(m_isBatchOpen);
	wxASSERT(!m_isBatchProcessed);

	// EnergyCalculatorFft doesn't support async energy calculation, since
	// BatchOpen does all the heavy lifting. Call Calculate and cache it.
	const BatchQueued::Handle handle = m_queuedEnergyResults.size();
	m_queuedEnergyResults.push_back(Calculate(bLeft, bTop));
	return handle;
}

void LfnIc::EnergyCalculatorFft::ProcessCalculations()
{
	wxASSERT(m_isBatchOpen);
	wxASSERT(!m_isBatchProcessed);
	m_isBatchProcessed = true;
}

LfnIc::Energy LfnIc::EnergyCalculatorFft::GetResult(BatchQueued::Handle handle) const
{
	wxASSERT(m_isBatchOpen);
	return m_queuedEnergyResults[handle];
}

LfnIc::EnergyCalculatorFft::FftwInPlaceBuffer LfnIc::EnergyCalculatorFft::FftwInPlaceBufferAlloc() const
{
	wxASSERT(!m_isBatchProcessed);
	FftwInPlaceBuffer result = { FFTW_PREFIX(malloc)(m_fftInPlaceBufferNumBytes) };
	return result;
}

template<typename T>
T* LfnIc::EnergyCalculatorFft::GetRow(T* real, int y) const
{
	//return (T*)((BYTE *)real + (m_fftInPlaceBufferStride * y));
	return (T*)((unsigned char*)real + (m_fftInPlaceBufferStride * y));
}

template<typename POLICY>
void LfnIc::EnergyCalculatorFft::FillRealBuffer(const POLICY& policy, FftReal* real, int left, int top, int width, int height) const
{
	// Clamp dimensions to fit within the fft dimensions
	width = std::min(width, m_fftWidth);
	height = std::min(height, m_fftHeight);

	// Clamp at the max. Don't clamp the min so that any required left or top
	// zero padding is included.
	EnergyCalculatorUtils::ClampToMaxBoundary(left, width, m_inputWidth);
	EnergyCalculatorUtils::ClampToMaxBoundary(top, height, m_inputHeight);

	const int leftPadding = std::max(-left, 0);
	const int topPadding = std::max(-top, 0);
	const int rightPadding = m_fftWidth - width;
	const int bottomPadding = m_fftHeight - height;
	PadRealBuffer(real, leftPadding, topPadding, rightPadding, bottomPadding);

	const int widthToCopy = width - leftPadding;
	const int heightToCopy = height - topPadding;
	left += leftPadding;
	top += topPadding;
	for (int y = 0; y < heightToCopy; ++y)
	{
		int inRowMajorIdx = LfnTech::GetRowMajorIndex(m_inputWidth, left, top + y);
		FftReal* out = GetRow(real, topPadding + y) + leftPadding;
		for (int x = 0; x < widthToCopy; ++x, ++inRowMajorIdx, ++out)
		{
			FFT_ASSERT_BOUNDS(real, out);
			*out = policy.GetReal(inRowMajorIdx);
		}
	}
}

template<typename POLICY>
void LfnIc::EnergyCalculatorFft::ReverseFillRealBuffer(const POLICY& policy, FftReal* real, int left, int top, int width, int height) const
{
	// Clamp dimensions to fit within the fft buffers
	width = std::min(width, m_fftWidth);
	height = std::min(height, m_fftHeight);

	// Clamp at the min. Don't clamp the max so that any required left or top
	// zero padding is included.
	EnergyCalculatorUtils::ClampToMinBoundary(left, width, 0);
	EnergyCalculatorUtils::ClampToMinBoundary(top, height, 0);

	int right = left + width - 1;
	int bottom = top + height - 1;
	const int policyRight = m_inputWidth - 1;
	const int policyBottom = m_inputHeight - 1;
	const int leftPadding = std::max(right - policyRight, 0);
	const int topPadding = std::max(bottom - policyBottom, 0);
	const int rightPadding = m_fftWidth - width;
	const int bottomPadding = m_fftHeight - height;
	PadRealBuffer(real, leftPadding, topPadding, rightPadding, bottomPadding);

	const int widthToCopy = width - leftPadding;
	const int heightToCopy = height - topPadding;
	right -= leftPadding;
	bottom -= topPadding;
	for (int y = 0; y < heightToCopy; ++y)
	{
		int inRowMajorIdx = LfnTech::GetRowMajorIndex(m_inputWidth, right, bottom - y);
		FftReal* out = GetRow(real, topPadding + y) + leftPadding;
		for (int x = 0; x < widthToCopy; ++x, --inRowMajorIdx, ++out)
		{
			FFT_ASSERT_BOUNDS(real, out);
			*out = policy.GetReal(inRowMajorIdx);
		}
	}
}

void LfnIc::EnergyCalculatorFft::PadRealBuffer(FftReal* real, int leftPad, int topPad, int rightPad, int bottomPad) const
{
	// Padding is non-overlapping. As seen below, where the real data is
	// represented by the x's, the top padding and bottom padding run the
	// entire width of the buffer, while the left and right padding is
	// sandwiched between the top and bottom padding.
	//
	//		tttttttttttt
	//		tttttttttttt
	//		llllxxxxrrrr
	//		llllxxxxrrrr
	//		llllxxxxrrrr
	//		llllxxxxrrrr
	//		bbbbbbbbbbbb
	//		bbbbbbbbbbbb
	const int rightPadX = m_fftWidth - rightPad;
	const int bottomPadY = m_fftHeight - bottomPad;

	if (topPad > 0)
	{
		for (int y = 0; y < topPad; ++y)
		{
			FftReal* dest = GetRow(real, y);
			FFT_ASSERT_BOUNDS_RANGE(real, dest, m_fftInPlaceBufferStride);
			memset(dest, 0, m_fftInPlaceBufferStride);
		}
	}

	if (leftPad > 0)
	{
		const int leftPadHeight = bottomPadY;
		//const int leftPadNumBytes = sizeof(FftReal) * leftPad;
		for (int y = topPad; y < leftPadHeight; ++y)
		{
			FftReal* dest = GetRow(real, y);
			FFT_ASSERT_BOUNDS_RANGE(real, dest, leftPadNumBytes);
			memset(dest, 0, sizeof(FftReal) * leftPad);
		}
	}

	if (rightPad > 0)
	{
		const int leftPadHeight = bottomPadY;
		const int rightPadNumBytes = m_fftInPlaceBufferStride - (sizeof(FftReal) * rightPadX);
		for (int y = topPad; y < leftPadHeight; ++y)
		{
			FftReal* dest = GetRow(real, y) + rightPadX;
			FFT_ASSERT_BOUNDS_RANGE(real, dest, rightPadNumBytes);
			memset(dest, 0, rightPadNumBytes);
		}
	}

	if (bottomPad > 0)
	{
		for (int y = bottomPadY; y < m_fftHeight; ++y)
		{
			FftReal* dest = GetRow(real, y);
			FFT_ASSERT_BOUNDS_RANGE(real, dest, m_fftInPlaceBufferStride);
			memset(dest, 0, m_fftInPlaceBufferStride);
		}
	}
}

void LfnIc::EnergyCalculatorFft::MultiplyEquals(FftComplex* aComplex, const FftComplex* bComplex) const
{
	const int complexNum = m_fftInPlaceBufferNumBytes / sizeof(FftComplex);
	FftComplex* aCurrent = aComplex;
	const FftComplex* bCurrent = bComplex;
	for (int i = 0; i < complexNum; ++i, ++aCurrent, ++bCurrent)
	{
		FFT_ASSERT_BOUNDS(aComplex, aCurrent);
		FFT_ASSERT_BOUNDS(bComplex, bCurrent);

		// a = a * b
		//
		//		ar[f] = ar[f]br[f] - ai[f]bi[f]
		//		ai[f] = ar[f]bi[f] + ai[f]br[f]
		FftComplex& a = *aCurrent;
		const FftComplex& b = *bCurrent;

		const FftReal ar = a[REAL]; // Save this before it's overwritten
		a[REAL] = (ar * b[REAL]) - (a[IMAG] * b[IMAG]);
		a[IMAG] = (ar * b[IMAG]) + (a[IMAG] * b[REAL]);
	}
}

void LfnIc::EnergyCalculatorFft::MultiplyEquals(FftReal* aReal, FftReal bScalar) const
{
	for (int y = 0; y < m_fftHeight; ++y)
	{
		FftReal* aCurrent = GetRow(aReal, y);
		for (int x = 0; x < m_fftWidth; ++x, ++aCurrent)
		{
			FFT_ASSERT_BOUNDS(aReal, aCurrent);

			*aCurrent *= bScalar;
		}
	}
}

template<typename ENERGY_OPERATOR>
void LfnIc::EnergyCalculatorFft::ApplyFftRealTo2ndAnd3rdTerm(const ENERGY_OPERATOR& energyOperator)
{
	// The results are shifted by the batch dimensions - 1
	const int resultsLeft = m_batchParams.width - 1;
	const int resultsTop = m_batchParams.height - 1;
	for (int y = 0; y < m_inputHeight; ++y)
	{
		Energy* batchEnergy2ndAnd3rdTermCurrent = m_batchEnergy2ndAnd3rdTerm + LfnTech::GetRowMajorIndex(m_inputWidth, 0, y);
		const FftReal* fftCurrent = GetRow(m_fftPlanBuffer.real, resultsTop + y) + resultsLeft;

		for (int x = 0; x < m_inputWidth; ++x, ++batchEnergy2ndAnd3rdTermCurrent, ++fftCurrent)
		{
			FFT_ASSERT_BOUNDS(m_fftPlanBuffer.real, fftCurrent);
			energyOperator.Execute(*batchEnergy2ndAnd3rdTermCurrent, Energy(*fftCurrent));
		}
	}
}

#endif // ENABLE_ENERGY_CALCULATOR_FFT
