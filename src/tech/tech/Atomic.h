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
// Hides platform details for thread safe, atomic operations. Only supports
// types that are sizeof(AtomicNativeType).
//
// Atomic<> represents the most efficient type of atomic operation because
// it uses the native atomic type, and therefore generates fewer casts.
//
#ifndef TECH_ATOMIC_H
#define TECH_ATOMIC_H

namespace Tech
{
#ifdef _MSC_VER
	typedef long AtomicNativeType;
#else
#error "Implement tech/Atomic.h for this platform!"
#endif

	template<typename T = AtomicNativeType>
	class Atomic
	{
	public:
		// See Windows Interlocked API for info.
		static inline T Increment(T volatile* addend);
		static inline T Decrement(T volatile* addend);
		static inline T ExchangeAdd(T volatile* addend, T value);
		static inline T CompareExchange(T volatile* destination, T exchange, T comparand);
	};
}

//
// Include the inline implementations
//
#define INCLUDING_TECH_ATOMIC_INL
#include "tech/Atomic.inl"
#undef INCLUDING_TECH_ATOMIC_INL

#endif
