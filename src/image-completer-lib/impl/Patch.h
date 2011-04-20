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

#ifndef PATCH_H
#define PATCH_H

#include "NeighborEdge.h"
#include "LfnIcTypes.h"

namespace LfnIc
{
	/// Describes a single rectangular patch that is to be copied from
	/// one region of the image to another. The patch stores the
	/// left-top points of the source and destination, and the width
	/// and height is described in the Settings class.
	struct Patch
	{
		/// Upper-left source and destination x and y within the image.
		int srcLeft;
		int srcTop;
		int destLeft;
		int destTop;

		/// A [0, 1] value that expresses the node's confidence level. This can
		/// be used when computing blend weights for overlapping patches.
		Priority priority;
	};
}

#endif
