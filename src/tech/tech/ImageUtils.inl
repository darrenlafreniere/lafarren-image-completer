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
// Contains inline implementations of ImageUtils.h definitions.
//
#ifndef TECH_IMAGE_UTILS_INL
#define TECH_IMAGE_UTILS_INL

#ifndef INCLUDING_TECH_IMAGE_UTILS_INL
#error "ImageUtils.inl must only be included by ImageUtils.h"
#endif

#include "tech/Core.h"
#include "tech/MathUtils.h"

namespace LfnTech
{
	template<typename T>
	inline T* BlendInto(T* destChannels, const T* srcChannels, float alpha, int numChannels)
	{
		if (alpha > 0.0f)
		{
			if (alpha >= 1.0f)
			{
				memcpy(destChannels, srcChannels, sizeof(T) * numChannels);
			}
			else
			{
				for (int i = 0; i < numChannels; ++i)
				{
					destChannels[i] = (T)(Lerp<float>(destChannels[i], srcChannels[i], alpha));
				}
			}
		}

		return destChannels;
	}

	template<typename T>
	inline T* GetRowMajorPointer(T* base, int stride, int x, int y)
	{
		return base + GetRowMajorIndex(stride / sizeof(T), x, y);
	}

	inline void* GetRowMajorPointer(void* base, int stride, int bytesPerPixel, int x, int y)
	{
		return ((byte*)base) + GetRowMajorIndex(stride / bytesPerPixel, bytesPerPixel, x, y);
	}

	inline const void* GetRowMajorPointer(const void* base, int stride, int bytesPerPixel, int x, int y)
	{
		return ((byte*)base) + GetRowMajorIndex(stride / bytesPerPixel, bytesPerPixel, x, y);
	}
}

#endif
