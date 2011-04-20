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

#ifndef PATCH_IMAGE_SOURCE_POISSON_H
#define PATCH_IMAGE_SOURCE_POISSON_H

#ifdef USE_POISSON

#include "PatchTypeNormal.h"

namespace LfnIc
{
	class ImageFloat;
	class MaskWritable;
	namespace Poisson
	{
		class Cloner;
	}

	///
	/// Serves patches whose pixels are pulled straight from the source image,
	/// then color-blended with their destination area using Poisson cloning.
	///
	class PatchTypePoisson : public PatchTypeNormal
	{
	public:
		struct Factory : public CompositorRoot::PatchType::Factory
		{
			virtual CompositorRoot::PatchType* Create(const Compositor::Input& input, ImageFloat& imageFloat) const;
		};

		PatchTypePoisson(const Compositor::Input& input, ImageFloat& imageFloat);
		virtual const ImageFloat& Get(const Patch& patch) const;

	private:
		std::auto_ptr<Poisson::Cloner> m_poissonCloner;
		std::auto_ptr<const MaskWritable> m_patchMask;
		std::auto_ptr<ImageFloat> m_patchImagePoisson;
	};
}

#endif // end USE_POISSON

#endif // PATCH_IMAGE_SOURCE_POISSON_H
