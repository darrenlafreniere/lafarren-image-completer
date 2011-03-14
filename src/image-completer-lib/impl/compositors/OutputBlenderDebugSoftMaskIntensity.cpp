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
#include "OutputBlenderDebugSoftMaskIntensity.h"

#include "CompositorUtils.h"
#include "ImageFloat.h"

void LfnIc::OutputBlenderDebugSoftMaskIntensity::Blend(const Compositor::Input& input, const ImageFloat& patchesBlended, ImageFloat& outputImageFloat) const
{
	std::vector<float> softMask;
	CreateSoftMask(input, softMask);

	const float* softMaskDataPtr = &softMask[0];
	PixelFloat* destRgbDataPtr = outputImageFloat.GetRgb();

	const int imageNumPixels = outputImageFloat.GetWidth() * outputImageFloat.GetHeight();
	for (int i = 0; i < imageNumPixels; ++i, ++destRgbDataPtr, ++softMaskDataPtr)
	{
		// Blend s into d based on the inverse alpha
		const float ia = 1.0f - *softMaskDataPtr;
		const float intensity = (1.0f - ia);
		PixelFloat& dest = *destRgbDataPtr;
		dest.channel[0] = dest.channel[1] = dest.channel[2] = intensity;
	}
}
