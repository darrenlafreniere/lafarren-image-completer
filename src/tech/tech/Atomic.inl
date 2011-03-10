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
// Contains inline implementations of Atomic.h definitions.
//
#ifndef TECH_ATOMIC_INL
#define TECH_ATOMIC_INL

#ifndef INCLUDING_TECH_ATOMIC_INL
#error "Atomic.inl must only be included by Atomic.h"
#endif

#ifdef _MSC_VER
#include <windows.h>
#endif

#define ASSERT_TECH_ATOMIC_SIZE(T) \
	wxASSERT(Tech::AtomicForceLinkValidationTests()); \
	wxCOMPILE_TIME_ASSERT(sizeof(T) == sizeof(AtomicNativeType), TypeMustBeSizeofAtomicNativeType)

namespace Tech
{
	bool AtomicForceLinkValidationTests();

#if defined(_MSC_VER)
	template<typename T>
	inline T Atomic<T>::Increment(T volatile* addend)
	{
		ASSERT_TECH_ATOMIC_SIZE(T);
		return static_cast<T>(
			InterlockedIncrement(
			reinterpret_cast<LONG volatile*>(addend)));
	}

	template<typename T>
	inline T Atomic<T>::Decrement(T volatile* addend)
	{
		ASSERT_TECH_ATOMIC_SIZE(T);
		return static_cast<T>(
			InterlockedDecrement(
			reinterpret_cast<LONG volatile*>(addend)));
	}

	template<typename T>
	inline T Atomic<T>::ExchangeAdd(T volatile* addend, T value)
	{
		ASSERT_TECH_ATOMIC_SIZE(T);
		return static_cast<T>(
			InterlockedExchangeAdd(
			reinterpret_cast<LONG volatile*>(addend), 
			static_cast<LONG>(value)));
	}

	template<typename T>
	inline T Atomic<T>::CompareExchange(T volatile* destination, T exchange, T comparand)
	{
		ASSERT_TECH_ATOMIC_SIZE(T);
		return static_cast<T>(
			InterlockedCompareExchange(
			reinterpret_cast<LONG volatile*>(destination),
			static_cast<LONG>(exchange),
			static_cast<LONG>(comparand)));
	}
#elif defined(UNIX)
	template<typename T>
	inline T Atomic<T>::Increment(T volatile* addend)
	{
		ASSERT_TECH_ATOMIC_SIZE(T);
		return __sync_add_and_fetch(
			reinterpret_cast<AtomicNativeType volatile*>(addend),
			1);
	}

	template<typename T>
	inline T Atomic<T>::Decrement(T volatile* addend)
	{
		ASSERT_TECH_ATOMIC_SIZE(T);
		return __sync_sub_and_fetch(
			reinterpret_cast<AtomicNativeType volatile*>(addend),
			1);
	}

	template<typename T>
	inline T Atomic<T>::ExchangeAdd(T volatile* addend, T value)
	{
		ASSERT_TECH_ATOMIC_SIZE(T);
		return __sync_fetch_and_add(
			reinterpret_cast<AtomicNativeType volatile*>(addend),
			value);
	}

	template<typename T>
	inline T Atomic<T>::CompareExchange(T volatile* destination, T exchange, T comparand)
	{
		ASSERT_TECH_ATOMIC_SIZE(T);
		return __sync_val_compare_and_swap(
			reinterpret_cast<AtomicNativeType volatile*>(destination),
			comparand,
			exchange);
	}
#else
#error "Implement tech/Atomic.inl for this platform!"
#endif
}

#endif
