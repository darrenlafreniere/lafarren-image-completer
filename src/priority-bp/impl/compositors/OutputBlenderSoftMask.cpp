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
#include "OutputBlenderSoftMask.h"

#include "tech/ImageUtils.h"

#include "CompositorUtils.h"
#include "ImageFloat.h"

namespace PriorityBp
{
	void OutputBlenderSoftMask::Blend(const Compositor::Input& input, const ImageFloat& patchesBlended, ImageFloat& outputImageFloat) const
	{
		std::vector<float> softMask;
		CreateSoftMask(input, softMask);

		const RgbFloat* srcRgbDataPtr = patchesBlended.GetRgb();
		const float* softMaskDataPtr = &softMask[0];
		RgbFloat* destRgbDataPtr = outputImageFloat.GetRgb();

		wxASSERT(patchesBlended.GetWidth() == outputImageFloat.GetWidth());
		wxASSERT(patchesBlended.GetHeight() == outputImageFloat.GetHeight());

		const int imageNumPixels = outputImageFloat.GetWidth() * outputImageFloat.GetHeight();
		for (int i = 0; i < imageNumPixels; ++i, ++destRgbDataPtr, ++srcRgbDataPtr, ++softMaskDataPtr)
		{
			// Blend s into d based on the inverse alpha
			const float ia = 1.0f - *softMaskDataPtr;
			Tech::BlendInto(destRgbDataPtr->channel, srcRgbDataPtr->channel, ia, HostImage::Rgb::NUM_CHANNELS);
		}
	}
}
