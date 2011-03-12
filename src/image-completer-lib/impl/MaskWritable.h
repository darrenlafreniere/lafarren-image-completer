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

#ifndef MASK_WRITABLE_H
#define MASK_WRITABLE_H

#include "Mask.h"

namespace PriorityBp
{
	//
	// Simple writable implementation of the Mask interface.
	//
	class MaskWritable : public Mask
	{
	public:
		MaskWritable(int width, int height, Value value);

		virtual Value GetValue(int x, int y) const;
		void SetValue(int x, int y, Value value);

	private:
		std::vector<Value> m_values;
		int m_width;
	};
}

#endif // MASK_WRITABLE_H
