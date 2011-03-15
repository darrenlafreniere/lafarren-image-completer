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
        for(unsigned int component = 0; component < static_cast<unsigned int>(PixelFloat::NUM_CHANNELS); component++)
        {
            this->channel[component] = other.channel[component];
        }
		return *this;
	}

	PixelFloat& PixelFloat::operator+=(const PixelFloat& other)
	{
        for(unsigned int component = 0; component < static_cast<unsigned int>(PixelFloat::NUM_CHANNELS); component++)
        {
            this->channel[component] += other.channel[component];
        }
		return *this;
	}

	PixelFloat& PixelFloat::operator-=(const PixelFloat& other)
	{
        for(unsigned int component = 0; component < static_cast<unsigned int>(PixelFloat::NUM_CHANNELS); component++)
        {
            this->channel[component] -= other.channel[component];
        }
		return *this;
	}

	PixelFloat& PixelFloat::operator*=(float x)
	{
        for(unsigned int component = 0; component < static_cast<unsigned int>(PixelFloat::NUM_CHANNELS); component++)
        {
            this->channel[component] *= x;
        }
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
        for(unsigned int component = 0; component < static_cast<unsigned int>(PixelFloat::NUM_CHANNELS); component++)
        {
            this->channel[component] = other.channel[component];
        }
        return *this;
    }

    PixelFloatRGB& PixelFloatRGB::operator+=(const PixelFloatRGB& other)
    {
        for(unsigned int component = 0; component < static_cast<unsigned int>(PixelFloat::NUM_CHANNELS); component++)
        {
            this->channel[component] += other.channel[component];
        }
        return *this;
    }

    PixelFloatRGB& PixelFloatRGB::operator-=(const PixelFloatRGB& other)
    {
        for(unsigned int component = 0; component < static_cast<unsigned int>(PixelFloat::NUM_CHANNELS); component++)
        {
            this->channel[component] -= other.channel[component];
        }
        return *this;
    }

    PixelFloatRGB& PixelFloatRGB::operator*=(float x)
    {
        for(unsigned int component = 0; component < static_cast<unsigned int>(PixelFloat::NUM_CHANNELS); component++)
        {
            this->channel[component] *= x;
        }
        return *this;
    }

	//
	// CopyRgbValue functions
	//
	inline void CopyRgbValue(PixelFloat& to, const Image::Pixel& from)
	{
        for(unsigned int component = 0; component < static_cast<unsigned int>(PixelFloat::NUM_CHANNELS); component++)
        {
            to.channel[component] = static_cast<float>(from.channel[component]) / 255.0f;
        }
	}

	inline void CopyRgbValue(Image::Pixel& to, const PixelFloat& from)
	{
        for(unsigned int component = 0; component < static_cast<unsigned int>(PixelFloat::NUM_CHANNELS); component++)
        {
            to.channel[component] = static_cast<unsigned char>(from.channel[component] * 255.0f);
        }
	}
}

#endif // IMAGE_FLOAT_INL
