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
#include "PatchBlenderNone.h"

#include "tech/MathUtils.h"

#include "ImageFloat.h"
#include "Settings.h"

#include "tech/DbgMem.h"

LfnIc::CompositorRoot::PatchBlender* LfnIc::PatchBlenderNone::Factory::Create(const Compositor::Input& input, const ImageFloat& imageFloat, ImageFloat& outPatchesBlended) const
{
	return new PatchBlenderNone(input, imageFloat, outPatchesBlended);
}

LfnIc::PatchBlenderNone::PatchBlenderNone(const Compositor::Input& input, const ImageFloat& imageFloat, ImageFloat& outPatchesBlended)
	: m_imageFloat(imageFloat)
	, m_outPatchesBlended(outPatchesBlended)
{
}

void LfnIc::PatchBlenderNone::Blend(const Patch& patch, const ImageFloat& patchImage) const
{
	const int imageWidth = m_imageFloat.GetWidth();
	const int imageHeight = m_imageFloat.GetHeight();
	const int patchWidth = patchImage.GetWidth();
	const int patchHeight = patchImage.GetHeight();
	const int patchNumPixels = patchWidth * patchHeight;

	const PixelFloat* const patchImageData = patchImage.GetRgb();
	PixelFloat* const destRgbData = m_outPatchesBlended.GetRgb();

	const int colClipOffset = std::max(-patch.destLeft, 0);
	const int rowClipOffset = std::max(-patch.destTop, 0);

	const int rowsNum = std::min(patchHeight, imageHeight - patch.destTop);
	for (int row = rowClipOffset, patchDestY = patch.destTop + rowClipOffset; row < rowsNum; ++row, ++patchDestY)
	{
		const int patchRowMajorIndex = LfnTech::GetRowMajorIndex(patchWidth, colClipOffset, row);
		const PixelFloat* patchImagePtr = patchImageData + patchRowMajorIndex;

		const int imageRowMajorIndex = LfnTech::GetRowMajorIndex(imageWidth, patch.destLeft + colClipOffset, patchDestY);
		PixelFloat* destRgbPtr = destRgbData + imageRowMajorIndex;

		const int colsNum = std::min(patchWidth, imageWidth - patch.destLeft);
		for (int col = colClipOffset; col < colsNum; ++patchImagePtr, ++destRgbPtr, ++col)
		{
			wxASSERT((destRgbPtr - destRgbData) < (imageWidth * imageHeight));
			wxASSERT((patchImagePtr - &patchImagePtr[0]) < patchNumPixels);
			*destRgbPtr = *patchImagePtr;
		}
	}
}
