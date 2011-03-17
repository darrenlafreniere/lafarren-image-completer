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

#ifndef IMAGE_FLOAT_H
#define IMAGE_FLOAT_H

#include "LfnIcImage.h"

namespace LfnIc
{
	class ImageConst;

	// An tuple of floats, where each component is [0.0, 1.0].
	class PixelFloat
	{
	public:
		static const int NUM_CHANNELS = Image::Pixel::NUM_CHANNELS;

		float channel[NUM_CHANNELS];

		inline PixelFloat();

		inline PixelFloat& operator=(const PixelFloat& other);
		inline PixelFloat& operator+=(const PixelFloat& other);
		inline PixelFloat& operator-=(const PixelFloat& other);
		inline PixelFloat& operator*=(float x);
	};

	// An RGB tuple of floats, where each component is [0.0, 1.0].
	// We need this class for some of the debug functions which write out colored (RGB) patches.
	class PixelFloatRGB
	{
	public:
		static const int NUM_CHANNELS = 3;

		float channel[3];

		inline PixelFloatRGB();

		inline PixelFloatRGB& operator=(const PixelFloatRGB& other);
		inline PixelFloatRGB& operator+=(const PixelFloatRGB& other);
		inline PixelFloatRGB& operator-=(const PixelFloatRGB& other);
		inline PixelFloatRGB& operator*=(float x);
	};

	//
	// A grid of RgbFloat pixels.
	//
	class ImageFloat
	{
	public:
		ImageFloat();
		ImageFloat(const ImageConst& input);
		ImageFloat(int width, int height);
		ImageFloat(int width, int height, const PixelFloat& initialPixel);

		void Create(int width, int height);

		void CopyTo(ImageFloat& output) const;
		void CopyTo(Image& output) const;

		inline int GetWidth() const { return m_width; }
		inline int GetHeight() const { return m_height; }

		PixelFloat& GetPixel(int x, int y);
		const PixelFloat& GetPixel(int x, int y) const;

		void SetPixel(int x, int y, const PixelFloat& pixel);

		inline PixelFloat* GetData() { return &m_data[0]; }
		inline const PixelFloat* GetData() const { return &m_data[0]; }

	private:
		ImageFloat(const ImageFloat& other) {}
		ImageFloat& operator=(const ImageFloat& other) { return *this; }

		int m_width;
		int m_height;
		std::vector<PixelFloat> m_data;
	};

	// Image::Rgb <=> RgbFloat conversions.
	inline void CopyRgbValue(PixelFloat& to, const Image::Pixel& from);
	inline void CopyRgbValue(Image::Pixel& to, const PixelFloat& from);
}

//
// Include the inline implementations
//
#define INCLUDING_IMAGE_FLOAT_INL
#include "ImageFloat.inl"
#undef INCLUDING_IMAGE_FLOAT_INL

#endif // IMAGE_FLOAT_H
