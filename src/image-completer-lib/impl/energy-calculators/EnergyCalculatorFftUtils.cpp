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
#include "EnergyCalculatorFftUtils.h"

#if ENABLE_ENERGY_CALCULATOR_FFT

#include "tech/MathUtils.h"

#include "ImageConst.h"
#include "MaskLod.h"

#include "tech/DbgMem.h"

#if FFT_VALIDATION_ENABLED
LfnIc::Energy LfnIc::EnergyCalculatorFftUtils::BruteForceCalculate1stTerm(const ImageConst& image, int width, int height, int aLeft, int aTop, const MaskLod* aMask)
{
	Energy e = Energy(0);

	const int imageWidth = image.GetWidth();
	const int imageHeight = image.GetHeight();
	const Mask::Value* maskBuffer = aMask ? aMask->GetLodBuffer(aMask->GetHighestLod()) : NULL;
	const Image::Pixel* imagePixel = image.GetData();

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if ((aLeft + x) >= 0 && (aTop + y) >= 0)
			{
				const int idx = LfnTech::GetRowMajorIndex(imageWidth, aLeft + x, aTop + y);
				if (!aMask || maskBuffer[idx] == Mask::KNOWN)
				{
					const Image::Pixel& pixel = imagePixel[idx];

					for (int c = 0; c < Image::Pixel::NUM_CHANNELS; ++c)
					{
						e += Energy(pixel.channel[c] * pixel.channel[c]);
					}
				}
			}
		}
	}

	return e;
}

LfnIc::Energy LfnIc::EnergyCalculatorFftUtils::BruteForceCalculate2ndTerm(const ImageConst& image, int width, int height, int aLeft, int aTop, const MaskLod* aMask, int bLeft, int bTop)
{
	Energy e = Energy(0);

	const int imageWidth = image.GetWidth();
	const int imageHeight = image.GetHeight();
	const Mask::Value* maskBuffer = aMask ? aMask->GetLodBuffer(aMask->GetHighestLod()) : NULL;
	const Image::Pixel* imagePixel = image.GetData();

	for (int j = 0; j < height; ++j)
	{
		const int ay = aTop + j;
		const int by = bTop + j;
		if (ay >= 0 && by >= 0 && ay < imageHeight && by < imageHeight)
		{
			for (int i = 0; i < width; ++i)
			{
				const int ax = aLeft + i;
				const int bx = bLeft + i;
				if (ax >= 0 && bx >= 0 && ax < imageWidth && bx < imageWidth)
				{
					const int aIdx = LfnTech::GetRowMajorIndex(imageWidth, ax, ay);
					if (!aMask || maskBuffer[aIdx] == Mask::KNOWN)
					{
						const Image::Pixel& aPixel = imagePixel[aIdx];
						const Image::Pixel& bPixel = imagePixel[LfnTech::GetRowMajorIndex(imageWidth, bx, by)];
						const int scalar = 2;

						for (int c = 0; c < Image::Pixel::NUM_CHANNELS; ++c)
						{
							e += scalar * aPixel.channel[c] * bPixel.channel[c];
						}
					}
				}
			}
		}
	}

	return -e;
}

LfnIc::Energy LfnIc::EnergyCalculatorFftUtils::BruteForceCalculate3rdTerm(const ImageConst& image, int width, int height, int aLeft, int aTop, const MaskLod* aMask, int bLeft, int bTop)
{
	Energy e = Energy(0);
	return e;
}
#endif //FFT_VALIDATION_ENABLED
#endif // ENABLE_ENERGY_CALCULATOR_FFT
