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

#ifndef APP_WX_MASK_H
#define APP_WX_MASK_H

#include <vector>
#include "LfnIcMask.h"

//
// Implements LfnIc::Mask, using a wxImage to load and convert a mask image.
//
class AppWxMask : public LfnIc::Mask
{
public:
	AppWxMask();

	// The mask image can be smaller than the input image, and translated to a
	// specific offset. This isn't yet supported via the command line arguments.
    virtual bool LoadAndValidate(const std::string& imagePath, int offsetX = 0, int offsetY = 0);

	virtual Value GetValue(int x, int y) const;

private:
	Value ByteToMaskValue(unsigned char byte) const;

	std::vector<Value> m_values;
	int m_width;
	int m_height;
	int m_offsetX;
	int m_offsetY;
};

#endif
