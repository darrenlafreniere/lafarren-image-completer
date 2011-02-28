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

#ifndef PRIORITY_BP_TYPES_H
#define PRIORITY_BP_TYPES_H

#include "tech/Core.h"

namespace PriorityBp
{
	// Markov Random Field energy
	typedef int64 Energy;
	const Energy ENERGY_MIN = Energy(0);
	const Energy ENERGY_MAX = Energy(LLONG_MAX);

	// Markov Random Field belief
	typedef Energy Belief;
	const Belief BELIEF_MIN = Belief(INT_MIN);
	const Belief BELIEF_MAX = Belief(0);

	typedef float Priority;
	const Priority PRIORITY_MIN = Priority(FLT_MIN);
	const Priority PRIORITY_MAX = Priority(1);

	// Determines how patches are extracted from the input image, or otherwise
	// initialized.
	enum CompositorPatchType
	{
		CompositorPatchTypeInvalid = -1,

		// Given a patch rectangle, this patch type simply copies the pixels
		// directly from the input image.
		CompositorPatchTypeNormal,

		// Given a patch rectangle, this patch type copies the pixels directly
		// from the input image, then perform a Poisson blend on them according
		// to the patch's destination in the output image.
		CompositorPatchTypePoisson,

		// Given a patch being applied to the output image, this patch type
		// solid-fills the pixels based on the order of the patch. Patches
		// are applied in order of confidence; the less confident the image
		// completer was about a patch, the earlier it's applied to the output,
		// while patches of greater confidence are applied later, overlapping
		// the less confidence patches. This patch type uses a full rainbow
		// spectrum where red patches are the least confident, and purple
		// patches are the most confident.
		CompositorPatchTypeDebugOrder,

		CompositorPatchTypeNum,
		CompositorPatchTypeDefault = CompositorPatchTypeNormal
	};

	// Determines how patches are blended together to fill the unknown region.
	enum CompositorPatchBlender
	{
		CompositorPatchBlenderInvalid = -1,

		// Blends the patches based on their priority (i.e., their confidence).
		// When a higher priority patch is blended with a lower priority patch,
		// the higher priority contributes more of an influence to the final
		// pixel data. In addition, a soft edge mask is applied to each patch
		// to reduce noticable seams.
		CompositorPatchBlenderPriority,

		// Patches are overlayed onto each other with absolutely no blending.
		CompositorPatchBlenderNone,

		CompositorPatchBlenderNum,
		CompositorPatchBlenderDefault = CompositorPatchBlenderPriority
	};
}

#endif
