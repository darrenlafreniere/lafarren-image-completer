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

#ifndef COMPOSITOR_ROOT_H
#define COMPOSITOR_ROOT_H

#include "Compositor.h"

namespace PriorityBp
{
	class ImageFloat;

	//
	// Implements the Compositor interface by delegating to components created
	// from the passed in polymorphic factories.
	//
	class CompositorRoot : public Compositor
	{
	public:
		// See CompositorPatchType enum.
		struct PatchType
		{
			struct Factory
			{
				virtual PatchType* Create(const Input& input, ImageFloat& imageFloat) const = 0;
			};

			virtual ~PatchType() {}
			virtual const ImageFloat& Get(const Patch& patch) const = 0;
		};

		// See CompositorPatchBlender.
		struct PatchBlender
		{
			struct Factory
			{
				virtual PatchBlender* Create(const Input& input, const ImageFloat& imageFloat, ImageFloat& outPatchesBlended) const = 0;
			};

			virtual ~PatchBlender() {}
			virtual void Blend(const Patch& patch, const ImageFloat& patchImage) const = 0;
		};

		// Blends the unified patches image into the output image.
		struct OutputBlender
		{
			virtual void Blend(const Input& input, const ImageFloat& patchesBlended, ImageFloat& outputImageFloat) const = 0;
		};

		// Passed in objects must be valid and allocated on the heap. This
		// compositor will assume ownership of all parameters, and destroy
		// them when the compositor itself is destroyed.
		CompositorRoot(
			const PatchType::Factory* patchTypeFactory,
			const PatchBlender::Factory* patchBlenderFactory,
			const OutputBlender* outputBlender);

		// Compositor interface:
		virtual bool Compose(const Input& input, HostImage& outputImage) const;

	private:
		std::auto_ptr<const PatchType::Factory> m_patchTypeFactory;
		std::auto_ptr<const PatchBlender::Factory> m_patchBlenderFactory;
		std::auto_ptr<const OutputBlender> m_outputBlender;
	};
}

#endif // COMPOSITOR_ROOT_H
