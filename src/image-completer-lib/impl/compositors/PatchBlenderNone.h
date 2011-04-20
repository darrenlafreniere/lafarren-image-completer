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

#ifndef PATCH_BLENDER_NONE_H
#define PATCH_BLENDER_NONE_H

#include "CompositorRoot.h"

namespace LfnIc
{
	/// Layers patches without blending.
	class PatchBlenderNone : public CompositorRoot::PatchBlender
	{
	public:
		struct Factory : public CompositorRoot::PatchBlender::Factory
		{
			virtual PatchBlender* Create(const Compositor::Input& input, const ImageFloat& imageFloat, ImageFloat& outPatchesBlended) const;
		};

		PatchBlenderNone(const Compositor::Input& input, const ImageFloat& imageFloat, ImageFloat& outPatchesBlended);
		virtual void Blend(const Patch& patch, const ImageFloat& patchImage) const;

	private:
		const ImageFloat& m_imageFloat;
		ImageFloat& m_outPatchesBlended;
	};
}

#endif // PATCH_BLENDER_NONE_H
