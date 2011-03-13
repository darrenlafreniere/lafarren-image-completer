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

#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "Patch.h"
#include "ImageConst.h"
#include "LfnIcSettings.h"

namespace LfnIc
{
	class ImageConst;
	class Mask;

	//
	// Abstract compositor interface. Composes an output image based on the input.
	//
	class Compositor
	{
	public:
		struct Input
		{
			Input(const Settings& settings, const ImageConst& inputImage, const Mask& mask);

			// Public properties:
			const Settings& settings;
			const ImageConst& inputImage;
			const Mask& mask;

			// The patches are sorted by a recommended order in which they
			// should be applied to the output image.
			std::vector<Patch> patches;
		};

		virtual ~Compositor() {}

		// Clones the input image into an output image, and applies the
		// input patches to the output according to the specific
		// compositor's blend strategy.
		virtual bool Compose(const Input& input, Image& outputImage) const = 0;
	};

	//
	// Instantiates a specific type of compositor based on the Type.
	//
	class CompositorFactory
	{
	public:
		static Compositor* Create(CompositorPatchType patchType, CompositorPatchBlender patchBlender);
	};
}

#endif
