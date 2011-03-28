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

#include "Pch.h"
#include "Compositor.h"

#include "compositors/CompositorRoot.h"
#include "compositors/OutputBlenderDebugSoftMaskIntensity.h"
#include "compositors/OutputBlenderSoftMask.h"
#include "compositors/OutputBlenderNone.h"
#include "compositors/PatchBlenderNone.h"
#include "compositors/PatchBlenderPriority.h"
#include "compositors/PatchTypeNormal.h"
#include "compositors/PatchTypeDebugPatchOrder.h"
#include "compositors/PatchTypePoisson.h"

#include "tech/DbgMem.h"

//
// Compositor::Input implementation
//
LfnIc::Compositor::Input::Input(const Settings& settings, const ImageConst& inputImage, const Mask& mask)
	: settings(settings)
	, inputImage(inputImage)
	, mask(mask)
{
}

//
// CompositorFactory implementation
//
LfnIc::Compositor* LfnIc::CompositorFactory::Create(CompositorPatchType patchType, CompositorPatchBlender patchBlender)
{
	Compositor* compositor = NULL;

	CompositorRoot::PatchType::Factory* patchimageSourceFactory = NULL;
	CompositorRoot::PatchBlender::Factory* patchBlenderFactory = NULL;
	// TODO: add a compositor-output-blender command line option
	CompositorRoot::OutputBlender* outputBlender = new OutputBlenderSoftMask;
	//CompositorRoot::OutputBlender* outputBlender = new OutputBlenderDebugSoftMaskIntensity;
	//CompositorRoot::OutputBlender* outputBlender = new OutputBlenderNone;

	switch (patchType)
	{
	case CompositorPatchTypeNormal:
		patchimageSourceFactory = new PatchTypeNormal::Factory;
		break;
#ifdef USE_POISSON
	case CompositorPatchTypePoisson:
		patchimageSourceFactory = new PatchTypePoisson::Factory;
		break;
#else
		#pragma message("Not built with Poisson blending enabled (USE_POISSON is undefined)!")
#endif
	case CompositorPatchTypeDebugOrder:
		patchimageSourceFactory = new PatchTypeDebugPatchOrder::Factory;
		break;
	default:
		wxASSERT("Invalid patchType");
		break;
	}

	switch (patchBlender)
	{
	case CompositorPatchBlenderPriority:
		patchBlenderFactory = new PatchBlenderPriority::Factory;
		break;
	case CompositorPatchBlenderNone:
		patchBlenderFactory = new PatchBlenderNone::Factory;
		break;
	default:
		wxASSERT("Invalid patchBlender");
		break;
	}

	if (patchimageSourceFactory && patchBlenderFactory && outputBlender)
	{
		compositor = new CompositorRoot(patchimageSourceFactory, patchBlenderFactory, outputBlender);
	}
	else
	{
		delete patchimageSourceFactory;
		delete patchBlenderFactory;
		delete outputBlender;
	}

	return compositor;
}
