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

#include "Image.h"

#include "tech/DbgMem.h"

wxCOMPILE_TIME_ASSERT(Image::Rgb::NUM_CHANNELS == RgbFloat::NUM_CHANNELS, RgbNumChannelsMismatch);

ImageFloat::ImageFloat()
	: m_width(0)
	, m_height(0)
{
}

ImageFloat::ImageFloat(const Image& input)
{
	const int width = input.GetWidth();
	const int height = input.GetHeight();
	Create(width, height);

	const HostImage::Rgb* rgbData = input.GetRgb();
	const int numPixels = width * height;
	for (int i = 0; i < numPixels; ++i)
	{
		const HostImage::Rgb& inRgb = rgbData[i];
		RgbFloat& outRgb = m_data[i];
		outRgb.r = float(inRgb.r) / 255.0f;
		outRgb.g = float(inRgb.g) / 255.0f;
		outRgb.b = float(inRgb.b) / 255.0f;
	}
}

ImageFloat::ImageFloat(int width, int height)
: m_width(width)
, m_height(height)
, m_data(width * height)
{
}

ImageFloat::ImageFloat(int width, int height, const RgbFloat& initialRgb)
: m_width(width)
, m_height(height)
, m_data(width * height, initialRgb)
{
}

void ImageFloat::Create(int width, int height)
{
	m_width = width;
	m_height = height;
	m_data.resize(width * height);
}

void ImageFloat::CopyTo(ImageFloat& output) const
{
	output.Create(m_width, m_height);
	memcpy(&output.m_data[0], &m_data[0], m_width * m_height * sizeof(RgbFloat));
}

void ImageFloat::CopyTo(HostImage& output) const
{
	output.Init(m_width, m_height);

	HostImage::Rgb* outRgbData = output.GetRgb();
	for (int y = 0, i = 0; y < m_height; ++y)
	{
		for (int x = 0; x < m_width; ++x, ++i)
		{
			const RgbFloat& inRgb = m_data[i];
			HostImage::Rgb& outRgb = outRgbData[i];
			outRgb.r = (unsigned char)(Clamp0To1(inRgb.r) * 255);
			outRgb.g = (unsigned char)(Clamp0To1(inRgb.g) * 255);
			outRgb.b = (unsigned char)(Clamp0To1(inRgb.b) * 255);
		}
	}
}

RgbFloat& ImageFloat::GetPixel(int x, int y)
{
	return m_data[GetRowMajorIndex(m_width, x, y)];
}

const RgbFloat& ImageFloat::GetPixel(int x, int y) const
{
	return m_data[GetRowMajorIndex(m_width, x, y)];
}

void ImageFloat::SetPixel(int x, int y, const RgbFloat& pixel)
{
	m_data[GetRowMajorIndex(m_width, x, y)] = pixel;
}
