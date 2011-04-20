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

#ifndef COMPOSITOR_UTILS_H
#define COMPOSITOR_UTILS_H

#include "Compositor.h"
#include "LfnIcMask.h"

namespace LfnIc
{
	/// Given the input mask, this method creates a soft mask of image
	/// WxH alpha values. The unknown regions are feathered outward to
	/// smooth the transition over the known-unknown boundaries.
	void CreateSoftMask(const Compositor::Input& input, std::vector<float>& out);

	/// Convert a mask value to an alpha value for blending.
	inline float MaskValueToAlpha(Mask::Value maskValue)
	{
		return (maskValue == Mask::UNKNOWN) ? 0.0f : 1.0f;
	}
}

#endif
