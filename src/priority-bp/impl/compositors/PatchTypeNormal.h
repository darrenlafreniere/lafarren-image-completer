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

#ifndef PATCH_IMAGE_SOURCE_NORMAL_H
#define PATCH_IMAGE_SOURCE_NORMAL_H

#include "CompositorRoot.h"
#include "ImageFloat.h"

namespace PriorityBp
{
	class Settings;

	class PatchTypeNormal : public CompositorRoot::PatchType
	{
	public:
		struct Factory : public CompositorRoot::PatchType::Factory
		{
			virtual CompositorRoot::PatchType* Create(const Compositor::Input& input, ImageFloat& imageFloat) const;
		};

		PatchTypeNormal(const Compositor::Input& input, ImageFloat& imageFloat);
		virtual const ImageFloat& Get(const Patch& patch) const;

	private:
		const ImageFloat& m_imageFloat;
		mutable ImageFloat m_patchImage;
	};
}

#endif // PATCH_IMAGE_SOURCE_NORMAL_H
