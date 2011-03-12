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
// Contains inline implementations of MathUtils.h definitions.
//
#ifndef TECH_MATH_UTILS_INL
#define TECH_MATH_UTILS_INL

#ifndef INCLUDING_TECH_MATH_UTILS_INL
#error "MathUtils.inl must only be included by MathUtils.h"
#endif

namespace LfnTech
{
	inline float Clamp(float f, float min, float max)
	{
		return (f < min) ? min : ((f > max) ? max : f);
	}

	inline float Clamp0To1(float f)
	{
		return Clamp(f, 0.0f, 1.0f);
	}

	inline int GetRowMajorIndex(int width, int x, int y)
	{
		return (width * y) + x;
	}

	inline int GetRowMajorIndex(int width, int channels, int x, int y)
	{
		return (width * y * channels) + (x * channels);
	}

	template<typename T>
	inline T Lerp(T from, T to, float alpha)
	{
		return from + ((to - from) * alpha);
	}

	template<typename T>
	inline float InverseLerp(T value, T min, T max)
	{
		const float diff = max - min;
		return (diff != 0.0f)
			? std::max(0.0f, std::min(float(value  - min) / diff, 1.0f))
			: 1.0f;
	}

	inline bool IsPowerOf2(int n)
	{
		return (n > 0) && ((n & (n - 1)) == 0);
	}

	inline unsigned int NextPowerOf2(unsigned int n)
	{
		static const int MANTISSA_MASK = (1 << 23) - 1;

		(*(float*)&n) = (float) n;
		n = n + MANTISSA_MASK & ~MANTISSA_MASK;
		n = (unsigned) (*(float*)&n);

		return n;
	}

	inline unsigned int LogBase2(unsigned int n)
	{
		// TODO: use one of the other techniques found here if more speed is needed:
		// http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogObvious
		int l = 0;
		while (n >>= 1)
		{
			++l;
		}

		return l;
	}

	template<typename T>
	inline void Swap(T& a, T& b)
	{
		T c = a;
		a = b;
		b = c;
	}
}

#endif
