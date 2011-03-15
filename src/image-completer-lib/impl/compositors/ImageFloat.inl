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
	// PixelFloat
	//
	PixelFloat::PixelFloat()
	{
	}

	PixelFloat& PixelFloat::operator=(const PixelFloat& other)
	{
		this->channel[0] = other.channel[0];
		this->channel[1] = other.channel[1];
		this->channel[2] = other.channel[2];
		return *this;
	}

	PixelFloat& PixelFloat::operator+=(const PixelFloat& other)
	{
		this->channel[0] += other.channel[0];
		this->channel[1] += other.channel[1];
		this->channel[2] += other.channel[2];
		return *this;
	}

	PixelFloat& PixelFloat::operator-=(const PixelFloat& other)
	{
		this->channel[0] -= other.channel[0];
		this->channel[1] -= other.channel[1];
		this->channel[2] -= other.channel[2];
		return *this;
	}

	PixelFloat& PixelFloat::operator*=(float x)
	{
		this->channel[0] *= x;
		this->channel[1] *= x;
		this->channel[2] *= x;
		return *this;
	}

    //
    // PixelFloatRGB
    //
    PixelFloatRGB::PixelFloatRGB()
    {
    }

    PixelFloatRGB& PixelFloatRGB::operator=(const PixelFloatRGB& other)
    {
        this->channel[0] = other.channel[0];
        this->channel[1] = other.channel[1];
        this->channel[2] = other.channel[2];
        return *this;
    }

    PixelFloatRGB& PixelFloatRGB::operator+=(const PixelFloatRGB& other)
    {
        this->channel[0] += other.channel[0];
        this->channel[1] += other.channel[1];
        this->channel[2] += other.channel[2];
        return *this;
    }

    PixelFloatRGB& PixelFloatRGB::operator-=(const PixelFloatRGB& other)
    {
        this->channel[0] -= other.channel[0];
        this->channel[1] -= other.channel[1];
        this->channel[2] -= other.channel[2];
        return *this;
    }

    PixelFloatRGB& PixelFloatRGB::operator*=(float x)
    {
        this->channel[0] *= x;
        this->channel[1] *= x;
        this->channel[2] *= x;
        return *this;
    }

	//
	// CopyRgbValue functions
	//
	inline void CopyRgbValue(PixelFloat& to, const Image::Pixel& from)
	{
		to.channel[0] = static_cast<float>(from.channel[0]) / 255.0f;
		to.channel[1] = static_cast<float>(from.channel[1]) / 255.0f;
		to.channel[2] = static_cast<float>(from.channel[2]) / 255.0f;
	}

	inline void CopyRgbValue(Image::Pixel& to, const PixelFloat& from)
	{
		to.channel[0] = static_cast<unsigned char>(from.channel[0] * 255.0f);
		to.channel[1] = static_cast<unsigned char>(from.channel[1] * 255.0f);
		to.channel[2] = static_cast<unsigned char>(from.channel[2] * 255.0f);
	}
}

#endif // IMAGE_FLOAT_INL
