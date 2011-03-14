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
#include "PatchTypeNormal.h"

#include "tech/MathUtils.h"

#include "ImageFloat.h"
#include "Settings.h"

#include "tech/DbgMem.h"

LfnIc::CompositorRoot::PatchType* LfnIc::PatchTypeNormal::Factory::Create(const Compositor::Input& input, ImageFloat& imageFloat) const
{
	return new PatchTypeNormal(input, imageFloat);
}

LfnIc::PatchTypeNormal::PatchTypeNormal(const Compositor::Input& input, ImageFloat& imageFloat)
	: m_imageFloat(imageFloat)
	, m_patchImage(input.settings.patchWidth, input.settings.patchHeight)
{
}

const LfnIc::ImageFloat& LfnIc::PatchTypeNormal::Get(const Patch& patch) const
{
	// Copy patch data out of image.
	{
		const int imageWidth = m_imageFloat.GetWidth();
		const int imageHeight = m_imageFloat.GetHeight();
		const int patchWidth = m_patchImage.GetWidth();
		const int patchHeight = m_patchImage.GetHeight();
		const int patchNumPixels  = patchWidth * patchHeight;

		const PixelFloat* srcRgbData = m_imageFloat.GetRgb();
		PixelFloat* patchImagePtr = m_patchImage.GetRgb();
		for (int patchY = 0, patchSrcY = patch.srcTop; patchY < patchHeight; ++patchY, ++patchSrcY)
		{
			const PixelFloat* srcRgbPtr = srcRgbData + LfnTech::GetRowMajorIndex(imageWidth, patch.srcLeft, patchSrcY);
			for (int patchX = 0; patchX < patchWidth; ++patchImagePtr, ++srcRgbPtr, ++patchX)
			{
				wxASSERT((srcRgbPtr - srcRgbData) < (imageWidth * imageHeight));
				wxASSERT((patchImagePtr - m_patchImage.GetRgb()) < patchNumPixels);
				*patchImagePtr = *srcRgbPtr;
			}
		}
	}

	return m_patchImage;
}
