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

#ifndef IMAGE_SCALABLE_H
#define IMAGE_SCALABLE_H

#include "ImageConst.h"
#include "Scalable.h"

namespace LfnIc
{
	// Forward declaration. Defined in Image.cpp.
	class ImageConstInternal;

	//
	// Implements both the Image and Scalable interfaces and provides an in
	// place scalable image. Initializes the image from the input Image.
	//
	class ImageScalable : public ImageConst, public Scalable
	{
	public:
		ImageScalable(const Image& image);
		virtual ~ImageScalable();

		virtual const Rgb* GetRgb() const;
		virtual int GetWidth() const;
		virtual int GetHeight() const;

		virtual void ScaleUp();
		virtual void ScaleDown();
		virtual int GetScaleDepth() const;

	private:
		inline ImageConstInternal& GetCurrentResolution() const { return *m_resolutions[m_depth]; }

		std::vector<ImageConstInternal*> m_resolutions;
		int m_depth;
	};
}

#endif
