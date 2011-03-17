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

#ifndef IMAGE_CONST_H
#define IMAGE_CONST_H

#include "LfnIcImage.h"

namespace LfnIc
{
	//
	// Partially implements Image under the assumption that the image is
	// const. Used internally to access read-only image data, including scaled
	// down data, during completion.
	//
	class ImageConst : public Image
	{
	public:
		// Partial implementation. The non-const methods will assert.
		// See base class for more info.
		virtual bool Init(int width, int height);
		virtual bool IsValid() const;
		virtual const std::string& GetFilePath() const;
		virtual Pixel* GetData();
		virtual bool LoadAndValidate(const std::string& imagePath) {return false;}

		// Unimplemented. These are defined in the base, but GetRgb() const
		// must be redefined here as well, because otherwise, calling GetRgb()
		// on a const Image& confuses the compile, and it tries calling the
		// non-const version of GetRgb() above, and generates an error.
		// Redefined the other two methods here just for completeness.
		virtual const Pixel* GetData() const = 0;
		virtual int GetWidth() const = 0;
		virtual int GetHeight() const = 0;

	protected:
		// Instances cannot be destroyed through a base Image pointer.
		virtual ~ImageConst() {}
	};
}

#endif
