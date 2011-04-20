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
#include "PatchBlenderPriority.h"

#include "tech/MathUtils.h"

#include "ImageFloat.h"
#include "LfnIcSettings.h"

#include "tech/DbgMem.h"

// How much the left, right, top, or bottom sides should be feathered as a
// percentage of the patch's width or height.
const float FEATHER_SIDE_PERCENTAGE = 0.10f;
const float ALPHA_OF_LOWEST_PRIORITY_PATCH  = 0.66f;
const float ALPHA_OF_HIGHEST_PRIORITY_PATCH = 1.0f;

LfnIc::CompositorRoot::PatchBlender* LfnIc::PatchBlenderPriority::Factory::Create(const Compositor::Input& input, const ImageFloat& imageFloat, ImageFloat& outPatchesBlended) const
{
	return new PatchBlenderPriority(input, imageFloat, outPatchesBlended);
}

LfnIc::PatchBlenderPriority::PatchBlenderPriority(const Compositor::Input& input, const ImageFloat& imageFloat, ImageFloat& outPatchesBlended)
	: m_priorityLowest(input.patches[0].priority)
	, m_priorityHighest(input.patches[input.patches.size() - 1].priority)
	, m_imageFloat(imageFloat)
	, m_outPatchesBlended(outPatchesBlended)
	, m_patchesWeightSum(imageFloat.GetWidth() * imageFloat.GetHeight(), 0.0f)
{
#if _DEBUG
	// Assert that all patches are sorted in ascending order of priority.
	m_priorityPrevious = PRIORITY_MIN;
	wxASSERT(m_priorityLowest <= m_priorityHighest);
#endif

	// Initialize m_patchFeatherAlpha.
	{
		const Settings& settings = input.settings;
		const int patchWidth = settings.patchWidth;
		const int patchHeight = settings.patchHeight;
		m_patchFeatherAlpha.resize(patchWidth * patchHeight, 1.0f);

		const float featherWidth = patchWidth * FEATHER_SIDE_PERCENTAGE;
		const float featherHeight = patchHeight * FEATHER_SIDE_PERCENTAGE;

		const int patchRight = patchWidth - 1;
		const int patchBottom = patchHeight - 1;

		float* patchFeatherAlphaPtr = &m_patchFeatherAlpha[0];
		for (int y = 0; y < patchHeight; ++y)
		{
			// Use -1.0 and patchWidth/patchHeight as the 0 alpha boundary.
			const float topFeather = LfnTech::InverseLerp<float>(y, -1.0f, featherHeight);
			const float bottomFeather = LfnTech::InverseLerp<float>(y, patchHeight, patchBottom - featherHeight);
			for (int x = 0; x < patchWidth; ++x, ++patchFeatherAlphaPtr)
			{
				const float leftFeather = LfnTech::InverseLerp<float>(x, -1.0f, featherWidth);
				const float rightFeather = LfnTech::InverseLerp<float>(x, patchWidth, patchRight - featherWidth);
				*patchFeatherAlphaPtr = topFeather * bottomFeather * leftFeather * rightFeather;
				wxASSERT(*patchFeatherAlphaPtr > 0.0f && *patchFeatherAlphaPtr <= 1.0f);
			}
		}
	}
}

LfnIc::PatchBlenderPriority::~PatchBlenderPriority()
{
	PixelFloat* rgbData = m_outPatchesBlended.GetData();
	const int imageNumPixels = m_imageFloat.GetWidth() * m_imageFloat.GetHeight();
	for (int i = 0; i < imageNumPixels; ++i)
	{
		const float weightSum = m_patchesWeightSum[i];
		if (weightSum > 0.0f)
		{
			PixelFloat& rgb = rgbData[i];
			for (int c = 0; c < PixelFloat::NUM_CHANNELS; ++c)
			{
				rgb.channel[c] /= weightSum;
			}
		}
	}
}

void LfnIc::PatchBlenderPriority::Blend(const Patch& patch, const ImageFloat& patchImage) const
{
#if _DEBUG
	wxASSERT(m_priorityPrevious <= patch.priority);
	m_priorityPrevious = patch.priority;
#endif
	const int imageWidth = m_imageFloat.GetWidth();
	const int imageHeight = m_imageFloat.GetHeight();
	const int patchWidth = patchImage.GetWidth();
	const int patchHeight = patchImage.GetHeight();
	const int patchNumPixels = patchWidth * patchHeight;

	const float patchWeight = LfnTech::InverseLerp(patch.priority, m_priorityLowest, m_priorityHighest);
	const float patchAlpha = LfnTech::Lerp(ALPHA_OF_LOWEST_PRIORITY_PATCH, ALPHA_OF_HIGHEST_PRIORITY_PATCH, patchWeight);

	const PixelFloat* const patchImageData = patchImage.GetData();
	const float* const patchFeatherAlphaData = &m_patchFeatherAlpha[0];
	PixelFloat* const destRgbData = m_outPatchesBlended.GetData();
	float* const destWeightSumData = &m_patchesWeightSum[0];

	const int colClipOffset = std::max(-patch.destLeft, 0);
	const int rowClipOffset = std::max(-patch.destTop, 0);

	const int rowsNum = std::min(patchHeight, imageHeight - patch.destTop);
	for (int row = rowClipOffset, patchDestY = patch.destTop + rowClipOffset; row < rowsNum; ++row, ++patchDestY)
	{
		const int patchRowMajorIndex = LfnTech::GetRowMajorIndex(patchWidth, colClipOffset, row);
		const PixelFloat* patchImagePtr = patchImageData + patchRowMajorIndex;
		const float* patchFeatherAlphaPtr = patchFeatherAlphaData + patchRowMajorIndex;

		const int imageRowMajorIndex = LfnTech::GetRowMajorIndex(imageWidth, patch.destLeft + colClipOffset, patchDestY);
		PixelFloat* destRgbPtr = destRgbData + imageRowMajorIndex;
		float* destWeightSumPtr = destWeightSumData + imageRowMajorIndex;

		const int colsNum = std::min(patchWidth, imageWidth - patch.destLeft);
		for (int col = colClipOffset; col < colsNum; ++patchImagePtr, ++patchFeatherAlphaPtr, ++destRgbPtr, ++destWeightSumPtr, ++col)
		{
			const float pixelWeight = patchAlpha * *patchFeatherAlphaPtr;

			wxASSERT((destRgbPtr - destRgbData) < (imageWidth * imageHeight));
			wxASSERT((patchImagePtr - &patchImagePtr[0]) < patchNumPixels);
			for (int c = 0; c < PixelFloat::NUM_CHANNELS; ++c)
			{
				destRgbPtr->channel[c] += patchImagePtr->channel[c] * pixelWeight;
			}

			*destWeightSumPtr += pixelWeight;
		}
	}
}
