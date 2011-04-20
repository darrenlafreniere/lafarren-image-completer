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
#include "OutputBlenderNone.h"

#include "tech/ImageUtils.h"

#include "CompositorUtils.h"
#include "ImageFloat.h"

void LfnIc::OutputBlenderNone::Blend(const Compositor::Input& input, const ImageFloat& patchesBlended, ImageFloat& outputImageFloat) const
{
	const PixelFloat* srcPixelPtr = patchesBlended.GetData();
	PixelFloat* destPixelPtr = outputImageFloat.GetData();

	wxASSERT(patchesBlended.GetWidth() == outputImageFloat.GetWidth());
	wxASSERT(patchesBlended.GetHeight() == outputImageFloat.GetHeight());

	const Mask& mask = input.mask;

	for (int y = 0; y < patchesBlended.GetHeight(); ++y)
	{
		for (int x = 0; x < patchesBlended.GetWidth(); ++x)
		{
			LfnTech::BlendInto(destPixelPtr->channel, srcPixelPtr->channel, 1.0 - MaskValueToAlpha(mask.GetValue(x,y)), Image::Pixel::NUM_CHANNELS);
			++destPixelPtr;
			++srcPixelPtr;
		}
	}
}
