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
// Contains image related utilities.
//
#ifndef TECH_IMAGE_UTILS_H
#define TECH_IMAGE_UTILS_H

namespace Lafarren
{
	// Blend src into dest by alpha. Returns a reference to dest.
	template<typename T>
	inline T* BlendInto(T* destChannels, const T* srcChannels, float alpha, int numChannels);

	// Returns the row-major pointer of a 2d location. Assumes T represents
	// one pixel's worth of data.
	template<typename T>
	inline T* GetRowMajorPointer(T* base, int stride, int x, int y);

	// Operates on a void* buffer, requires bytes per pixel.
	inline void* GetRowMajorPointer(void* base, int bytesPerPixel, int stride, int x, int y);
	inline const void* GetRowMajorPointer(const void* base, int bytesPerPixel, int stride, int x, int y);

	//
	// Image data copying.
	//
	// These functions return false if destBase or srcBase are NULL, or if
	// destBounds, srcBounds, destStride, srcStride, or bytesPerPixel are zero.
	//
	// Otherwise they return true. If srcRect is empty, it's interpretted as
	// a successful no-op copy, not an error.
	//

	// Copies data from one buffer's rectangle to another buffer. Able to clip
	// the copy to the destination and source size bounds.
	bool Copy(
		void* destBase,
		const void* srcBase,
		const wxSize& destBounds,
		const wxSize& srcBounds,
		const wxPoint& destPoint,
		const wxRect& srcRect,
		int destStride,
		int srcStride,
		int bytesPerPixel);

	// Copies data from one buffer's rectangle to another buffer. Able to clip
	// the copy to the destination and source rect bounds.
	bool Copy(
		void* destBase,
		const void* srcBase,
		const wxRect& destBounds,
		const wxRect& srcBounds,
		const wxPoint& destPoint,
		const wxRect& srcRect,
		int destStride,
		int srcStride,
		int bytesPerPixel);

	// Copies data from one buffer's rectangle to another buffer. The source
	// rectangle and destination point must not be outside of either buffer's
	// bounds.
	bool Copy(
		void* destBase,
		const void* srcBase,
		const wxPoint& destPoint,
		const wxRect& srcRect,
		int destStride,
		int srcStride,
		int bytesPerPixel);
}

//
// Include the inline implementations
//
#define INCLUDING_TECH_IMAGE_UTILS_INL
#include "tech/ImageUtils.inl"
#undef INCLUDING_TECH_IMAGE_UTILS_INL

#endif
