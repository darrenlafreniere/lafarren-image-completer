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

#include "Pch.h"
#include "tech/Atomic.h"

#include "tech/DbgMem.h"

namespace Tech
{
	class AtomicValidationTests
	{
	public:
		AtomicValidationTests()
		{
			// Atomic<>::Increment test
			{
				AtomicNativeType x = 10;
				const AtomicNativeType y = Atomic<>::Increment(&x);
				wxASSERT(x == 11 && y == 11);
			}

			// Atomic<>::Decrement test
			{
				AtomicNativeType x = 10;
				const AtomicNativeType y = Atomic<>::Decrement(&x);
				wxASSERT(x == 9 && y == 9);
			}

			// Atomic<>::ExchangeAdd tests
			{
				{
					AtomicNativeType x = 10;
					const AtomicNativeType y = Atomic<>::ExchangeAdd(&x, 5);
					wxASSERT(x == 15 && y == 10);
				}

				{
					AtomicNativeType x = 10;
					const AtomicNativeType y = Atomic<>::ExchangeAdd(&x, -5);
					wxASSERT(x == 5 && y == 10);
				}
			}

			// Atomic<>::CompareExchange tests
			{
				{
					AtomicNativeType x = 10;
					const AtomicNativeType y = Atomic<>::CompareExchange(&x, 5, 10);
					wxASSERT(x == 5 && y == 10);
				}

				{
					AtomicNativeType x = 10;
					const AtomicNativeType y = Atomic<>::CompareExchange(&x, 5, 20);
					wxASSERT(x == 10 && y == 10);
				}
			}
		}
	};

	bool AtomicForceLinkValidationTests()
	{
		static AtomicValidationTests atomicValidationTests;
		return true;
	}
}
