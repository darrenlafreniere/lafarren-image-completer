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
#include "CompositorUtils.h"

#include "tech/MathUtils.h"
#include "tech/ImageUtils.h"

#include "Image.h"
#include "Mask.h"
#include "PriorityBpSettings.h"

#include "tech/DbgMem.h"

const int BLEND_SIZE = 2;
const int SOFT_MASK_NUM_SAMPLES = (BLEND_SIZE * 2) + 1;

inline float MaskValueToAlpha(PriorityBp::Mask::Value maskValue)
{
	return (maskValue == PriorityBp::Mask::UNKNOWN) ? 0.0f : 1.0f;
}

void PriorityBp::CreateSoftMask(const Compositor::Input& input, std::vector<float>& out)
{
	const Image& inputImage = input.inputImage;
	const Mask& mask = input.mask;
	const int imageWidth = inputImage.GetWidth();
	const int imageHeight = inputImage.GetHeight();
	wxASSERT(imageWidth > 0);
	wxASSERT(imageHeight > 0);

	wxASSERT(SOFT_MASK_NUM_SAMPLES > 1);

	out.resize(imageWidth * imageHeight, 1.0f);

	// Helper class to track alpha samples
	class Samples
	{
	public:
		Samples(float firstSample) :
		m_samplesSum(firstSample * float(SOFT_MASK_NUM_SAMPLES)),
		m_nextSampleIdx(0)
		{
			for (int i = 0; i < SOFT_MASK_NUM_SAMPLES; ++i)
			{
				m_samples[i] = firstSample;
			}
		}

		inline float operator[](int index)
		{
			return m_samples[index];
		}

		void AddSample(float sample)
		{
			m_samplesSum -= m_samples[m_nextSampleIdx];
			m_samples[m_nextSampleIdx] = sample;
			m_samplesSum += sample;
			m_nextSampleIdx = (m_nextSampleIdx + 1) % SOFT_MASK_NUM_SAMPLES;
		}

		inline float Blend() const
		{
			return m_samplesSum / float(SOFT_MASK_NUM_SAMPLES);
		}

	private:
		float m_samplesSum;
		int m_nextSampleIdx;
		float m_samples[SOFT_MASK_NUM_SAMPLES];
	};

	// Horizontal blur
	for (int y = 0, maskIdx = 0; y < imageHeight; ++y)
	{
		Samples samples(MaskValueToAlpha(mask.GetValue(0, y)));
		for (int sampleLeadEdge = 0, sampleEnd = imageWidth + BLEND_SIZE; sampleLeadEdge < sampleEnd; ++sampleLeadEdge)
		{
			samples.AddSample(MaskValueToAlpha(mask.GetValue(std::min(sampleLeadEdge, imageWidth - 1), y)));

			const int x = sampleLeadEdge - BLEND_SIZE;
			if (x >= 0)
			{
				wxASSERT(maskIdx == Lafarren::GetRowMajorIndex(imageWidth, x, y));
				const float hardAlpha = MaskValueToAlpha(mask.GetValue(x, y));
				out[maskIdx++] = std::min(hardAlpha, samples.Blend());
			}
		}
	}

	// Vertical blur
	for (int x = 0; x < imageWidth; ++x)
	{
		Samples samples(MaskValueToAlpha(mask.GetValue(x, 0)));
		for (int sampleLeadEdge = 0, sampleEnd = imageHeight + BLEND_SIZE; sampleLeadEdge < sampleEnd; ++sampleLeadEdge)
		{
			{
				const int maskIdx = Lafarren::GetRowMajorIndex(imageWidth, x, std::min(sampleLeadEdge, imageHeight - 1));
				samples.AddSample(out[maskIdx]);
			}

			const int y = sampleLeadEdge - BLEND_SIZE;
			if (y >= 0)
			{
				const int maskIdx = Lafarren::GetRowMajorIndex(imageWidth, x, y);
				out[maskIdx] *= samples.Blend();
			}
		}
	}

#if _DEBUG
	// Verify that every unknown mask pixel has an alpha of 0.0
	for (int y = 0; y < imageHeight; ++y)
	{
		for (int x = 0; x < imageWidth; ++x)
		{
			if (mask.GetValue(x, y) == Mask::UNKNOWN)
			{
				const int maskIdx = GetRowMajorIndex(imageWidth, x, y);
				wxASSERT(out[maskIdx] == 0.0f);
			}
		}
	}
#endif
}
