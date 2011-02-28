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

#ifndef TECH_DBGMEM_H
#define TECH_DBGMEM_H

//
// Used to track memory allocations and leaks.
//
// In your project's precompiled header, add these lines:
//
//		#ifdef _DEBUG
//			#include <crtdbg.h>
//			#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
//		#else
//			#define DEBUG_NEW new
//		#endif
//
//
// Then, at the top of each cpp module just after the pch, include this
// DbgMem.h file.
//
// This macro must be disabled to use the placement new operator, and may
// be done so using the UnDbgMem.h:
//
//		#include "tech/UnDbgMem.h"
//		void UsesPlacementNew(Foo* ptr)
//		{
//			new(ptr) Foo();
//		}
//		#include "tech/DbgMem.h"
//

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

#endif
