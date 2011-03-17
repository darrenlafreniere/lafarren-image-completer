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
#include "ImageFloat.h"

#include "tech/MathUtils.h"

#include "ImageConst.h"

#include "tech/DbgMem.h"

wxCOMPILE_TIME_ASSERT(LfnIc::Image::Pixel::NUM_CHANNELS == LfnIc::PixelFloat::NUM_CHANNELS, RgbNumChannelsMismatch);

LfnIc::ImageFloat::ImageFloat()
	: m_width(0)
	, m_height(0)
{
}

LfnIc::ImageFloat::ImageFloat(const ImageConst& input)
{
	const int width = input.GetWidth();
	const int height = input.GetHeight();
	Create(width, height);

	const Image::Pixel* rgbData = input.GetData();
	const int numPixels = width * height;
	for (int i = 0; i < numPixels; ++i)
	{
		const Image::Pixel& inRgb = rgbData[i];
		PixelFloat& outRgb = m_data[i];
		for (int component = 0; component < PixelFloat::NUM_CHANNELS; ++component)
		{
			outRgb.channel[component] = float(inRgb.channel[component]) / 255.0f;
		}
	}
}


LfnIc::ImageFloat::ImageFloat(int width, int height)
	: m_width(width)
	, m_height(height)
	, m_data(width * height)
{
}

LfnIc::ImageFloat::ImageFloat(int width, int height, const PixelFloat& initialPixel)
	: m_width(width)
	, m_height(height)
	, m_data(width * height, initialPixel)
{
}

void LfnIc::ImageFloat::Create(int width, int height)
{
	m_width = width;
	m_height = height;
	m_data.resize(width * height);
}

void LfnIc::ImageFloat::CopyTo(ImageFloat& output) const
{
	output.Create(m_width, m_height);
	memcpy(&output.m_data[0], &m_data[0], m_width * m_height * sizeof(PixelFloat));
}

void LfnIc::ImageFloat::CopyTo(Image& output) const
{
	output.Init(m_width, m_height);

	Image::Pixel* outRgbData = output.GetData();
	for (int y = 0, i = 0; y < m_height; ++y)
	{
		for (int x = 0; x < m_width; ++x, ++i)
		{
			const PixelFloat& inRgb = m_data[i];
			Image::Pixel& outRgb = outRgbData[i];
			for (int component = 0; component < PixelFloat::NUM_CHANNELS; ++component)
			{
				outRgb.channel[component] = static_cast<Image::Pixel::PixelType>((LfnTech::Clamp0To1(inRgb.channel[component]) * 255));
			}
		}
	}
}

LfnIc::PixelFloat& LfnIc::ImageFloat::GetPixel(int x, int y)
{
	return m_data[LfnTech::GetRowMajorIndex(m_width, x, y)];
}

const LfnIc::PixelFloat& LfnIc::ImageFloat::GetPixel(int x, int y) const
{
	return m_data[LfnTech::GetRowMajorIndex(m_width, x, y)];
}

void LfnIc::ImageFloat::SetPixel(int x, int y, const PixelFloat& pixel)
{
	m_data[LfnTech::GetRowMajorIndex(m_width, x, y)] = pixel;
}
