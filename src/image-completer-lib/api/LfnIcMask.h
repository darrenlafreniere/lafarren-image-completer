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

#ifndef LFN_IC_MASK_H
#define LFN_IC_MASK_H

namespace LfnIc
{
	// Mask interface. EXPORTed so that the static const members are linked.
	class Mask
	{
	public:
		// Use typedef and consts rather than enum to enforce single byte storage:
		typedef char Value;

		// Indeterminate is used for lower levels of detail (lod > 0),
		// where the region in question contains both unknown and known
		// pixels. Examination at a higher level of detail may be required.
		static const Value INDETERMINATE = -1;

		// Unknown pixels should be filled by the image completer.
		static const Value UNKNOWN = 0;

		// Ignored pixels should not be filled by the image completer,
		// nor may they be used as input for the image completion
		// process, nor should they be used in energy calculations.
		static const Value IGNORED = 1;

		// Known pixels should not be filled by the image completer,
		// but may be used as input for the image completion process.
		static const Value KNOWN = 2;

		// Samples a single value. If x and y are outside of the mask's
		// internal boundaries, this method is expected to return KNOWN.
		virtual Value GetValue(int x, int y) const = 0;

		// Determine if the mask has at least one known pixel. This is not pure virtual because MaskScalable should always be true.
		virtual bool HasKnownPixel() const {return true;}

	protected:
		// Cannot destroy Mask instance through a pointer to this base.
		~Mask() {}
	};
}

#endif
