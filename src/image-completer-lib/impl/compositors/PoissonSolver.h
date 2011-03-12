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

#ifndef POISSON_SOLVER_H
#define POISSON_SOLVER_H

#include "ImageFloat.h"
#include "MaskWritable.h"

namespace LfnIc
{
	class HostImage;
	class Mask;

	namespace Poisson
	{
		// Approximates the input image's missing regions according to the mask,
		// and writes the result to the output image.
		void Complete(
			const ImageFloat& inputImage,
			const Mask& mask,
			ImageFloat& outputImage);

		// CompositorRoot performs a separate clone for each patch.
		// Cloning has been wrapped in this class to reuse some temp buffers.
		class Cloner
		{
		public:
			Cloner(const ImageFloat& targetImage);

			// Clones the masked source image into the target image at the specified
			// offset within the target, and writes the result to the output image at
			// the specified output offset.
			void Clone(
				const ImageFloat& sourceImage,
				const Mask& mask,
				int sourceOffsetX,
				int sourceOffsetY,
				ImageFloat& outputImage,
				int outputOffsetX,
				int outputOffsetY);

		private:
			void UpdateLaplacian(
				const ImageFloat& sourceImage,
				const Mask& mask,
				int sourceOffsetX,
				int sourceOffsetY);

			const ImageFloat& m_targetImage;
			ImageFloat m_sourceImageTranslated;
			wxRect m_sourceImageTranslatedRect;

			ImageFloat m_laplacian;
			MaskWritable m_laplacianMask;
		};
	}
}

#endif // POISSON_SOLVER_H
