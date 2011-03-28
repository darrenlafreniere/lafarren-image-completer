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
// tech base, platform/compiler independent definitions
//
#ifndef TECH_CORE_H
#define TECH_CORE_H

///
/// Native typedefs.
/// NOTE: these are in the global namespace.
///
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

/// Can be fully specialized for type specific information.
template<typename T> struct TypeInfo {};

#if defined(_MSC_VER) || defined(UNIX)
/// Only implemented for 32 bit builds at the moment.
/// Use these only where size actually matters.
typedef char int8;
typedef unsigned char uint8;
typedef unsigned char byte;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
#else
#error "Implement tech/Core.h types for this platform!"
#endif

#if defined(_MSC_VER)
typedef __int64 int64;
typedef unsigned __int64 uint64;
#define FORCE_INLINE __forceinline
#define EXPORT __declspec(dllexport)
#elif defined(UNIX)
typedef long long int64;
typedef unsigned long long uint64;
#define FORCE_INLINE inline
#define EXPORT // we define this to be empty on Linux
#define CALLBACK // we define this to be empty on Linux
#else
#error "Implement tech/Core.h's special keyword macros for this platform!"
#endif

#endif
