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
// Main API header for the Priority-BP based image completer.
// Based on http://www.csd.uoc.gr/~komod/publications/docs/TIP_07.pdf
//
#ifndef LFN_IC_H
#define LFN_IC_H

#include <iostream>
#include "tech/Core.h"

namespace LfnIc
{
	// Forward declarations
	class Image;
	struct Settings;

	//
	// Performs image completion on a given input image and mask, and returns
	// the output image.
	//
	// If a valid patches istream is provided, then instead of solving the
	// patches by running the Priority-BP algorithm, they will read from the
	// stream.
	//
	// If a valid patches ostream is provided, then the patches will be written
	// to that stream.
	//
	extern EXPORT bool Complete(
		const Settings& settings,
		const Image& inputImage,
		const Image& maskImage,
		Image& outputImage,
		std::istream* patchesIstream = NULL,
		std::ostream* patchesOstream = NULL);
}

#endif
