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

#ifndef HOST_IMAGE_LOCAL_H
#define HOST_IMAGE_LOCAL_H

#include "PriorityBpHost.h"

namespace LfnIc
{
	//
	// Implements LfnIc::Image using heap memory. Intended for local
	// temporary image buffers. PriorityBp is considered the "host" of this
	// image implementation.
	//
	class HostImageLocal : public Image
	{
	public:
		HostImageLocal();
		HostImageLocal(int width, int height);
		virtual ~HostImageLocal();

		virtual bool Init(int width, int height);
		virtual bool IsValid() const;
		virtual Rgb* GetRgb();
		virtual const Rgb* GetRgb() const;
		virtual int GetWidth() const;
		virtual int GetHeight() const;

	private:
		// Internal non-virtual methods that are safe to call from constructor.
		bool InitInternal(int width, int height);
		bool IsValidInternal() const;

		int m_width;
		int m_height;
		Rgb* m_rgb;
	};
}

#endif // HOST_IMAGE_LOCAL_H
