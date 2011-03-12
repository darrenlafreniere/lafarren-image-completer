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
// General purpose math utilities.
//
#ifndef TECH_MATH_UTILS_H
#define TECH_MATH_UTILS_H

namespace LfnTech
{
	const double PI = 3.1415926535897932384626433832795;

	//
	// If f is < min, returns min. If f is > max, returns max. Otherwise, returns f.
	//
	inline float Clamp(float f, float min, float max);

	//
	// Clamp(f, 0.0f, 1.0f)
	//
	inline float Clamp0To1(float f);

	//
	// Returns the row-major index of a 2d location.
	//
	inline int GetRowMajorIndex(int width, int x, int y);

	//
	// Returns the row-major index of a 2d, multi-channel location.
	//
	inline int GetRowMajorIndex(int width, int channels, int x, int y);

	// Linear interpolation
	template<typename T>
	inline T Lerp(T from, T to, float alpha);

	// Inverse linear interpolation to get an alpha value
	template<typename T>
	inline float InverseLerp(T value, T min, T max);

	// Returns true if n is a power of 2.
	inline bool IsPowerOf2(int n);

	// Returns the next closest power of 2 to n.
	inline unsigned int NextPowerOf2(unsigned int n);
	
	// Returns the log base 2 of n.
	inline unsigned int LogBase2(unsigned int n);

	// Not a math function, move this someday.
	template<typename T>
	inline void Swap(T& a, T& b);

	// Clips a span, defined by a start value and a length, to a minimum value.
	// Returns true if the span was clipped.
	template<typename T>
	inline bool ClipSpanToMin(T& start, T& length, T min);

	// Clips a span, defined by a start value and a length, to a maximum value.
	// Returns true if the span was clipped.
	template<typename T>
	inline bool ClipSpanToMax(T& start, T& length, T max);
}

//
// Include the inline implementations
//
#define INCLUDING_TECH_MATH_UTILS_INL
#include "tech/MathUtils.inl"
#undef INCLUDING_TECH_MATH_UTILS_INL

#endif
