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

#include "tech/Time.h"

#include "tech/DbgMem.h"

namespace LfnIc
{
#if ENABLE_ENERGY_CALCULATOR_FFT
	class ScopedTimeAccumulator
	{
	public:
		inline ScopedTimeAccumulator(double& accumulationTarget)
			: m_accumulationTarget(accumulationTarget)
			, m_startTime(LfnTech::CurrentTime())
		{
		}

		inline ~ScopedTimeAccumulator()
		{
			const double deltaTime = LfnTech::CurrentTime() - m_startTime;
			m_accumulationTarget += deltaTime;
		}

	private:
		double& m_accumulationTarget;
		const double m_startTime;
	};

	struct EnergyBatchSize
	{
		// numBatchPixels is BatchParams.width * BatchParams.height, and is one
		// of the batch input parameters used to rank the "size" of a batch.
		int numBatchPixels;

		// numBatchCalculations is the other input used to rank the "size" of a
		// batch.
		int numBatchCalculations;

		inline EnergyBatchSize(const EnergyCalculator::BatchParams& batchParams, int numBatchCalculations)
			: numBatchPixels(batchParams.width * batchParams.height)
			, numBatchCalculations(numBatchCalculations)
		{
		}

		inline EnergyBatchSize(const EnergyBatchSize& other)
			: numBatchPixels(other.numBatchPixels)
			, numBatchCalculations(other.numBatchCalculations)
		{
		}

		inline EnergyBatchSize& operator=(const EnergyBatchSize& other)
		{
			numBatchPixels = other.numBatchPixels;
			numBatchCalculations = other.numBatchCalculations;
			return *this;
		}

		inline bool operator==(const EnergyBatchSize& other) const
		{
			return
				numBatchPixels == other.numBatchPixels &&
				numBatchCalculations == other.numBatchCalculations;
		}

		inline bool operator!=(const EnergyBatchSize& other) const
		{
			return !operator==(other);
		}

		inline bool operator<=(const EnergyBatchSize& other) const
		{
			return
				numBatchPixels <= other.numBatchPixels &&
				numBatchCalculations <= other.numBatchCalculations;
		}
	};

	// Used to measure the performance of another energy calculator (i.e.,
	// the EnergyCalculatorPerPixel or EnergyCalculatorFft) for a given
	// batch size, and return the faster of the two energy calculators once
	// both are measured.
	class EnergyCalculatorMeasurer : private EnergyCalculator
	{
	public:
		EnergyCalculatorMeasurer(
			const EnergyBatchSize& batchSize,
			EnergyCalculatorContainer& energyCalculatorContainer,
			EnergyCalculatorPerPixel& energyCalculatorPerPixel,
			EnergyCalculatorFft& energyCalculatorFft)
			: m_batchSize(batchSize)
			, m_energyCalculatorContainer(energyCalculatorContainer)
			, m_measurePerPixel(energyCalculatorPerPixel)
			, m_measureFft(energyCalculatorFft)
			, m_measureCurrent(&m_measureFft) // Measure fft first; if batch size is only calculated once, the fft's worst case with large images is better than per-pixel's worst case
			, m_fasterEnergyCalculator(NULL)
		{
		}

		inline const EnergyBatchSize& GetBatchSize() const { return m_batchSize; }

		inline bool FoundFasterEnergyCalculator() const
		{
			return m_fasterEnergyCalculator != NULL;
		}

		inline EnergyCalculator* GetFasterEnergyCalculator() const
		{
			return m_fasterEnergyCalculator;
		}

		inline EnergyCalculator& GetEnergyCalculator()
		{
			if (!m_measureCurrent)
			{
				wxASSERT(m_fasterEnergyCalculator);
				return *m_fasterEnergyCalculator;
			}
			else
			{
				// This privately implements EnergyCalculator, and delegates
				// to the calculator currently being measured.
				wxASSERT(!m_fasterEnergyCalculator);
				return *this;
			}
		}

	private:
		// References an energy calculator to measure, and the current total
		// time spent in the energy calculator.
		struct Measure
		{
			EnergyCalculator& energyCalculator;
			mutable double totalTime;

			Measure(EnergyCalculator& energyCalculator)
				: energyCalculator(energyCalculator)
				, totalTime(0.0)
			{
			}
		};

		virtual void BatchOpenImmediate(const BatchParams& params)
		{
			wxASSERT(m_measureCurrent);

			ScopedTimeAccumulator scopedTimeAccumulator(m_measureCurrent->totalTime);
			m_measureCurrent->energyCalculator.BatchOpenImmediate(params);
		}

		virtual void BatchOpenQueued(const BatchParams& params)
		{
			wxASSERT(m_measureCurrent);

			ScopedTimeAccumulator scopedTimeAccumulator(m_measureCurrent->totalTime);
			m_measureCurrent->energyCalculator.BatchOpenQueued(params);
		}

		virtual void BatchClose()
		{
			wxASSERT(m_measureCurrent);

			ScopedTimeAccumulator scopedTimeAccumulator(m_measureCurrent->totalTime);
			m_measureCurrent->energyCalculator.BatchClose();

			// Advance to the next measuring stage. Fft is measured first; see
			// constructor initialization list.
			if (m_measureCurrent == &m_measureFft)
			{
				m_measureCurrent = &m_measurePerPixel;
			}
			else
			{
				wxASSERT(m_measureCurrent == &m_measurePerPixel);
				m_measureCurrent = NULL;

				// Because the per-pixel energy calculator uses less memory,
				// if it's at least 95% as fast as the fft calculator for this
				// batch size, use it instead.
				const double perPixelBiasScalar = 1.0 / 0.95;
				m_fasterEnergyCalculator = m_measureFft.totalTime < (m_measurePerPixel.totalTime * perPixelBiasScalar)
					? &m_measureFft.energyCalculator
					: &m_measurePerPixel.energyCalculator;

				m_energyCalculatorContainer.OnFoundFasterEnergyCalculator(*this);
			}
		}

		virtual Energy Calculate(int bLeft, int bTop) const
		{
			wxASSERT(m_measureCurrent);

			ScopedTimeAccumulator scopedTimeAccumulator(m_measureCurrent->totalTime);
			return m_measureCurrent->energyCalculator.Calculate(bLeft, bTop);
		}

		virtual BatchQueued::Handle QueueCalculation(int bLeft, int bTop)
		{
			wxASSERT(m_measureCurrent);

			ScopedTimeAccumulator scopedTimeAccumulator(m_measureCurrent->totalTime);
			return m_measureCurrent->energyCalculator.QueueCalculation(bLeft, bTop);
		}

		virtual void ProcessCalculations()
		{
			wxASSERT(m_measureCurrent);

			ScopedTimeAccumulator scopedTimeAccumulator(m_measureCurrent->totalTime);
			m_measureCurrent->energyCalculator.ProcessCalculations();
		}

		virtual Energy GetResult(BatchQueued::Handle handle) const
		{
			wxASSERT(m_measureCurrent);

			ScopedTimeAccumulator scopedTimeAccumulator(m_measureCurrent->totalTime);
			return m_measureCurrent->energyCalculator.GetResult(handle);
		}

		EnergyCalculatorContainer& m_energyCalculatorContainer;

		const EnergyBatchSize m_batchSize;

		// The Measure structures for the two energy calculators.
		Measure m_measurePerPixel;
		Measure m_measureFft;

		// Points to what we're currently measuring, or NULL if measuring has finished.
		Measure* m_measureCurrent;

		// When measuring has finished, this will point at the faster of the two energy
		// calculators.
		EnergyCalculator* m_fasterEnergyCalculator;
	};
#endif // ENABLE_ENERGY_CALCULATOR_FFT
}

//
// EnergyCalculatorContainer implementation
//
LfnIc::EnergyCalculatorContainer::EnergyCalculatorContainer(const Settings& settings, const ImageConst& inputImage, const MaskLod& mask)
	: m_energyCalculatorPerPixel(inputImage, mask)
#if ENABLE_ENERGY_CALCULATOR_FFT
#if FFT_VALIDATION_ENABLED
	, m_energyCalculatorFft(new EnergyCalculatorFft(settings, inputImage, mask, m_energyCalculatorPerPixel))
#else
	, m_energyCalculatorFft(new EnergyCalculatorFft(settings, inputImage, mask))
#endif // FFT_VALIDATION_ENABLED
#endif // ENABLE_ENERGY_CALCULATOR_FFT
{
}

LfnIc::EnergyCalculatorContainer::~EnergyCalculatorContainer()
{
#if ENABLE_ENERGY_CALCULATOR_FFT
	ClearMeasurers();
	delete m_energyCalculatorFft;
#endif
}

LfnIc::EnergyCalculator& LfnIc::EnergyCalculatorContainer::Get(const EnergyCalculator::BatchParams& batchParams, int numBatchCalculations)
{
	wxASSERT(numBatchCalculations > 0);
#if !ENABLE_ENERGY_CALCULATOR_FFT
	return m_energyCalculatorPerPixel;
#else
	const EnergyBatchSize batchSize(batchParams, numBatchCalculations);
	EnergyCalculatorMeasurer* measurerToUse = NULL;

	// Iterate through measurers, looking for measurers that have determined
	// the faster calculator to use for its batch size. If a measurer is using
	// a per-pixel calculator and this batch size is smaller than the
	// measurer's, then this batch size should use the per-pixel calculator as
	// well. On the other hand, if a measurer is using a fft calculator and
	// this batch size is larger than the measurer's, then this batch size
	// should also use the fft calculator.
	for (int i = 0, n = m_measurers.size(); i < n; ++i)
	{
		wxASSERT(m_measurers[i]);
		EnergyCalculatorMeasurer& measurer = *m_measurers[i];
		if (measurer.GetFasterEnergyCalculator() == &m_energyCalculatorPerPixel)
		{
			// Per-pixel is better with smaller batches.
			if (batchSize <= measurer.GetBatchSize())
			{
				measurerToUse = &measurer;
				break;
			}
		}
		else if (measurer.GetFasterEnergyCalculator() == m_energyCalculatorFft)
		{
			// Fft is better with larger batches.
			if (measurer.GetBatchSize() <= batchSize)
			{
				measurerToUse = &measurer;
				break;
			}
		}
		else if (measurer.GetBatchSize() == batchSize)
		{
			measurerToUse = &measurer;
			break;
		}
	}

	if (!measurerToUse)
	{
		measurerToUse = new EnergyCalculatorMeasurer(batchSize, *this, m_energyCalculatorPerPixel, *m_energyCalculatorFft);
		m_measurers.push_back(measurerToUse);
	}

	wxASSERT(measurerToUse);
	return measurerToUse->GetEnergyCalculator();
#endif
}

#if ENABLE_ENERGY_CALCULATOR_FFT
void LfnIc::EnergyCalculatorContainer::OnFoundFasterEnergyCalculator(const EnergyCalculatorMeasurer& measurer)
{
	wxASSERT(measurer.FoundFasterEnergyCalculator());
	EnergyCalculator& fasterEnergyCalculator = *measurer.GetFasterEnergyCalculator();

	const EnergyBatchSize& batchSize = measurer.GetBatchSize();
	if (&fasterEnergyCalculator == &m_energyCalculatorPerPixel)
	{
		// Just determined that a batch size is faster with the per-pixel
		// energy calculator. Because the per-pixel calculator is faster with
		// smaller batches than the fft calculator, all measured (or in the
		// process of being measured) batches smaller than this can be removed
		// to reduce the search space that EnergyCalculatorContainer::Get must
		// iterate through.
		for (int i = m_measurers.size(); --i >= 0; )
		{
			if (m_measurers[i] != &measurer)
			{
				wxASSERT(m_measurers[i]);
				if (m_measurers[i]->GetBatchSize() <= batchSize)
				{
					m_measurers.erase(m_measurers.begin() + i);
				}
			}
		}
	}
	else
	{
		wxASSERT(&fasterEnergyCalculator == m_energyCalculatorFft);

		// See comment in if (&fasterEnergyCalculator == &m_energyCalculatorPerPixel)
		// block. Same case here, except all larger batch sizes will use fft and can
		// be removed.
		for (int i = m_measurers.size(); --i >= 0; )
		{
			if (m_measurers[i] != &measurer)
			{
				wxASSERT(m_measurers[i]);
				if (batchSize <= m_measurers[i]->GetBatchSize())
				{
					m_measurers.erase(m_measurers.begin() + i);
				}
			}
		}
	}
}

void LfnIc::EnergyCalculatorContainer::ClearMeasurers()
{
	for (int i = 0, n = m_measurers.size(); i < n; ++i)
	{
		delete m_measurers[i];
	}
}
#endif
