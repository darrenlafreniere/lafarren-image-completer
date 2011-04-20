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
// Header to be precompiled
//

#ifndef PCH_H
#define PCH_H

// If not explicitly defining USE_ITK, default to USE_WX.
#ifndef USE_ITK
	#define USE_WX
#endif

// Compile time switches:
#define TECH_PROFILE 1
#define TECH_PROFILE_MACROS 1

#if defined(_DEBUG) && defined(_MSC_VER)
	#include <crtdbg.h>
	#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#else
	#define DEBUG_NEW new
#endif

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <memory>
#include <set>
#include <vector>

// wxWidgets headers
#include "wx/wx.h"

#endif
