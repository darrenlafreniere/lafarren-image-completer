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

//
// Contains inline implementations of ImageFloat.h definitions.
//
#ifndef IMAGE_FLOAT_INL
#define IMAGE_FLOAT_INL

#ifndef INCLUDING_IMAGE_FLOAT_INL
#error "ImageFloat.inl must only be included by ImageFloat.h"
#endif

namespace LfnIc
{
	//
	// RgbFloat
	//
	RgbFloat::RgbFloat()
	{
	}

	RgbFloat::RgbFloat(float r, float g, float b)
		: r(r)
		, g(g)
		, b(b)
	{
	}

	RgbFloat& RgbFloat::operator=(const RgbFloat& other)
	{
		r = other.r;
		g = other.g;
		b = other.b;
		return *this;
	}

	RgbFloat& RgbFloat::operator+=(const RgbFloat& other)
	{
		r += other.r;
		g += other.g;
		b += other.b;
		return *this;
	}

	RgbFloat& RgbFloat::operator-=(const RgbFloat& other)
	{
		r -= other.r;
		g -= other.g;
		b -= other.b;
		return *this;
	}

	RgbFloat& RgbFloat::operator*=(float x)
	{
		r *= x;
		g *= x;
		b *= x;
		return *this;
	}

	//
	// CopyRgbValue functions
	//
	inline void CopyRgbValue(RgbFloat& to, const HostImage::Rgb& from)
	{
		to.r = static_cast<float>(from.r) / 255.0f;
		to.g = static_cast<float>(from.g) / 255.0f;
		to.b = static_cast<float>(from.b) / 255.0f;
	}

	inline void CopyRgbValue(HostImage::Rgb& to, const RgbFloat& from)
	{
		to.r = static_cast<unsigned char>(from.r * 255.0f);
		to.g = static_cast<unsigned char>(from.g * 255.0f);
		to.b = static_cast<unsigned char>(from.b * 255.0f);
	}
}

#endif // IMAGE_FLOAT_INL
