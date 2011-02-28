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

#include "PriorityBpHost.h"

namespace PriorityBp
{
	class Image;

	// An rgb tuple of floats, where each component is [0.0, 1.0].
	class RgbFloat
	{
	public:
		union
		{
			struct
			{
				float r;
				float g;
				float b;
			};

			static const int NUM_CHANNELS = 3;
			float channel[NUM_CHANNELS];
		};

		inline RgbFloat();
		inline RgbFloat(float r, float g, float b);

		inline RgbFloat& operator=(const RgbFloat& other);
		inline RgbFloat& operator+=(const RgbFloat& other);
		inline RgbFloat& operator-=(const RgbFloat& other);
		inline RgbFloat& operator*=(float x);
	};

	//
	// A grid of RgbFloat pixels.
	//
	class ImageFloat
	{
	public:
		ImageFloat();
		ImageFloat(const Image& input);
		ImageFloat(int width, int height);
		ImageFloat(int width, int height, const RgbFloat& initialRgb);

		void Create(int width, int height);

		void CopyTo(ImageFloat& output) const;
		void CopyTo(HostImage& output) const;

		inline int GetWidth() const { return m_width; }
		inline int GetHeight() const { return m_height; }

		RgbFloat& GetPixel(int x, int y);
		const RgbFloat& GetPixel(int x, int y) const;

		void SetPixel(int x, int y, const RgbFloat& pixel);

		inline RgbFloat* GetRgb() { return &m_data[0]; }
		inline const RgbFloat* GetRgb() const { return &m_data[0]; }

	private:
		ImageFloat(const ImageFloat& other) {}
		ImageFloat& operator=(const ImageFloat& other) { return *this; }

		int m_width;
		int m_height;
		std::vector<RgbFloat> m_data;
	};

	// HostImage::Rgb <=> RgbFloat conversions.
	inline void CopyRgbValue(RgbFloat& to, const HostImage::Rgb& from);
	inline void CopyRgbValue(HostImage::Rgb& to, const RgbFloat& from);
}

//
// Include the inline implementations
//
#define INCLUDING_IMAGE_FLOAT_INL
#include "ImageFloat.inl"
#undef INCLUDING_IMAGE_FLOAT_INL

#endif // IMAGE_FLOAT_H
